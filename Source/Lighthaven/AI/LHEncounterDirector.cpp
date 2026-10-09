#include "AI/LHEncounterDirector.h"
#include "AI/LHEnemyAIController.h"
#include "Framework/LHEnemyCharacter.h"
#include "World/LHWorldMarkers.h"
#include "Abilities/LHAttributeSet.h"
#include "EngineUtils.h"
#include "Engine/TriggerBox.h"
#include "Engine/TargetPoint.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
namespace LHEncounterDirectorPrivate
{
ALHSpawnMarker* Marker(UWorld* World, const FLHSpawnLifeId& Life)
{
    for (TActorIterator<ALHSpawnMarker> It(World); It; ++It)
        if (It->SpawnId == Life.SpawnSlot && It->Area.Content.Value == Life.Area.Content.Value) return *It;
    return nullptr;
}
bool SameEntity(const FLHEntityId& A, const FLHEntityId& B)
{ return A.RunId == B.RunId && A.Area.Content.Value == B.Area.Content.Value && A.InstanceId == B.InstanceId; }
}
bool ULHEncounterDirector::IsSimulationEnabled() const
{ return bLoaded && !bFrozen && GetWorld() && !UGameplayStatics::IsGamePaused(GetWorld()) && !GetWorld()->bIsTearingDown; }
void ULHEncounterDirector::SetSimulationFrozen(bool Frozen)
{
    bFrozen = Frozen;
    if (Frozen) for (ALHEnemyCharacter* E : Enemies) if (IsValid(E)) if (auto* C = Cast<ALHEnemyAIController>(E->GetController())) C->Suspend();
}
bool ULHEncounterDirector::IsSafeSegment(const FVector& Start, const FVector& End, float Radius, float HalfHeight) const
{
    for (const auto& Box : SafetyBounds)
    {
        const FBox Expanded = Box.ExpandBy(FVector(Radius, Radius, HalfHeight));
        if (Expanded.IsInsideOrOn(Start) || Expanded.IsInsideOrOn(End) || FMath::LineBoxIntersection(Expanded, Start, End, End-Start)) return false;
    }
    return true;
}
bool ULHEncounterDirector::HasSight(const AActor* Source, const AActor* Target) const
{
    if (!IsValid(Source) || !IsValid(Target)) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(LHAISight), false, Source); Params.AddIgnoredActor(Target);
    return !GetWorld()->LineTraceTestByChannel(Source->GetActorLocation(), Target->GetActorLocation(), ECC_Visibility, Params);
}
void ULHEncounterDirector::Populate(const FLHAreaRecord& Area)
{
    // Hydration is a complete floor replacement, never leaves an old alive actor behind.
    const bool SameArea = bLoaded && ActiveArea.Content.Value == Area.Area.Content.Value;
    auto Old = Enemies; Enemies.Reset();
    for (ALHEnemyCharacter* E : Old) if (IsValid(E)) { if (auto* C = E->GetController()) C->Destroy(); E->Destroy(); }
    if (!SameArea) PublishedDeaths.Reset();
    ActiveArea = Area.Area; bLoaded = true; SafetyBounds.Reset();
    for (TActorIterator<ATriggerBox> It(GetWorld()); It; ++It)
        for (const FName Tag : It->Tags) if (Tag.ToString().StartsWith(TEXT("LH.Safety.NoCombat.")))
        { SafetyBounds.Add(It->GetCollisionComponent()->Bounds.GetBox()); break; }
    for (const auto& Record : Area.Encounters) if (Record.State == ELHEncounterLifeState::Alive) SpawnRecord(Record);
}
void ULHEncounterDirector::SpawnRecord(const FLHEncounterRecord& R)
{
    if (!bLoaded || R.Life.Area.Content.Value != ActiveArea.Content.Value || !ResolveSpec || !SettleKill || FindByLife(R.Life) ||
        PublishedDeaths.ContainsByPredicate([&](const FLHSpawnLifeId& L) { return LHAI::SameLife(L,R.Life); })) return;
    auto* Marker = LHEncounterDirectorPrivate::Marker(GetWorld(), R.Life);
    const auto* Spec = ResolveSpec(R.Definition);
    FString Error;
    if (!Marker || Marker->EnemyDefinitionId.Value != R.Definition.Value || !Spec || Spec->ContentId.Value != R.Definition.Value || !LHAI::ValidateSpec(*Spec, Error))
    { UE_LOG(LogTemp, Warning, TEXT("LH enemy spawn rejected: %s"), *Error); return; }
    FVector Anchor = Marker->GetActorLocation() + FVector(0,0,Spec->CapsuleHalfHeightCm.Value);
    FVector Home = Anchor;
    if (Spec->bBoss)
    {
        ATargetPoint* Arena = nullptr;
        for (TActorIterator<ATargetPoint> It(GetWorld()); It; ++It) if (It->ActorHasTag(TEXT("B4.BalorkArena")) || It->GetFName() == FName(TEXT("B4.BalorkArena"))) { Arena = *It; break; }
        if (!Arena) { UE_LOG(LogTemp, Warning, TEXT("LH boss requires B4.BalorkArena")); return; }
        Home = Arena->GetActorLocation() + FVector(0,0,Spec->CapsuleHalfHeightCm.Value);
        if (FVector::Dist2D(Anchor,Home) > Spec->LeashRadiusCm.Value) return;
    }
    if (!IsSafeSegment(Anchor, Anchor, Spec->CapsuleRadiusCm.Value, Spec->CapsuleHalfHeightCm.Value)) return;
    const FTransform Transform(Marker->GetActorRotation(), Anchor);
    auto* E = GetWorld()->SpawnActorDeferred<ALHEnemyCharacter>(ALHEnemyCharacter::StaticClass(), Transform, nullptr, nullptr,
        ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding);
    if (!E) return;
    if (!E->ApplyRuntimeSpec(*Spec, R.Life, R.CurrentHealth, Error)) { E->Destroy(); return; }
    E->FinishSpawning(Transform);
    if (!IsValid(E) || E->IsActorBeingDestroyed()) return;
    Enemies.Add(E);
    E->GetCombatComponent()->OnDeath.AddWeakLambda(this, [this, Weak = TWeakObjectPtr<ALHEnemyCharacter>(E)](const FLHHitIdentity& Hit)
        { if (auto* Actor = Weak.Get()) HandleDeath(Actor,Hit); });
    auto* Controller = GetWorld()->SpawnActor<ALHEnemyAIController>();
    if (Controller) Controller->InitializeRuntime(E,this,Home);
}
void ULHEncounterDirector::HandleDeath(ALHEnemyCharacter* E, const FLHHitIdentity& Hit)
{
    const auto Life = E->GetLife();
    if (PublishedDeaths.ContainsByPredicate([&](const FLHSpawnLifeId& L) { return LHAI::SameLife(L,Life); })) return;
    PublishedDeaths.Add(Life); // Before callback: reentrant death/despawn cannot award twice.
    E->MarkCorpse();
    // Combat's death event lacks attacker identity. Single-player player binding is the only supported killer.
    if (SettleKill && !SettleKill(Life,Hit,Player.Get()))
        UE_LOG(LogTemp, Error, TEXT("LH kill settlement rejected; integrator must retain/recover failed transaction for slot %s"), *Life.SpawnSlot.ToString());
}
void ULHEncounterDirector::CaptureLive(FLHAreaRecord& Area) const
{
    if (!bLoaded || Area.Area.Content.Value != ActiveArea.Content.Value) return;
    for (auto& R : Area.Encounters)
        if (R.State == ELHEncounterLifeState::Alive) if (auto* E = FindByLife(R.Life)) if (E->IsAlive() && !E->GetCombatComponent()->IsActionPending())
            R.CurrentHealth.Value = E->GetCombatComponent()->GetCombatAttributes()->GetHealth();
}
void ULHEncounterDirector::SpawnLife(const FLHSpawnLifeId& Life)
{
    if (!IsSimulationEnabled() || FindByLife(Life)) return;
    auto* M = LHEncounterDirectorPrivate::Marker(GetWorld(),Life);
    if (!M || !ResolveSpec || !IsSpawnSafe(Life.SpawnSlot)) return;
    const auto* S = ResolveSpec(M->EnemyDefinitionId); if (!S) return;
    // W4-06 calls only AFTER committed Alive/new-generation respawn; this method never advances generation.
    FLHEncounterRecord R; R.Life = Life; R.Definition = M->EnemyDefinitionId; R.State = ELHEncounterLifeState::Alive; R.CurrentHealth = S->MaxHealth;
    SpawnRecord(R);
}
void ULHEncounterDirector::Despawn(const FLHSpawnLifeId& Life)
{
    if (auto* E = FindByLife(Life)) { Enemies.Remove(E); if (auto* C = E->GetController()) C->Destroy(); E->Destroy(); }
}
ALHEnemyCharacter* ULHEncounterDirector::FindByLife(const FLHSpawnLifeId& Life) const
{ for (ALHEnemyCharacter* E : Enemies) if (IsValid(E) && !E->IsActorBeingDestroyed() && LHAI::SameLife(E->GetLife(),Life)) return E; return nullptr; }
ALHEnemyCharacter* ULHEncounterDirector::FindByEntity(const FLHEntityId& Entity) const
{ for (ALHEnemyCharacter* E : Enemies) if (IsValid(E) && !E->IsActorBeingDestroyed() && LHEncounterDirectorPrivate::SameEntity(E->GetEntityId(RunId),Entity)) return E; return nullptr; }
bool ULHEncounterDirector::CanPursue(const ALHEnemyAIController* Controller, int32 Cap) const
{
    int32 Count = 0;
    for (ALHEnemyCharacter* E : Enemies) if (IsValid(E)) if (auto* C = Cast<ALHEnemyAIController>(E->GetController()))
        if (C != Controller && C->IsPursuing()) { ++Count; Cap = FMath::Min(Cap, static_cast<int32>(E->GetRuntimeSpec()->MaxActivePursuers.Value)); }
    return Count < Cap;
}
void ULHEncounterDirector::TickActiveSimulation(float Seconds)
{
    if (!IsSimulationEnabled() || !FMath::IsFinite(Seconds) || Seconds <= 0) return;
    // Callbacks may spawn/despawn. Iterate a copy and check actor validity after each callback.
    if (AdvanceRespawns) AdvanceRespawns(Seconds);
    auto Copy = Enemies;
    for (ALHEnemyCharacter* E : Copy) if (IsValid(E) && !E->IsActorBeingDestroyed()) if (auto* C = Cast<ALHEnemyAIController>(E->GetController())) C->Advance(Seconds);
}
bool ULHEncounterDirector::IsSpawnSafe(const FGuid& SpawnId) const
{
    if (!IsSimulationEnabled() || !Player.IsValid() || !ResolveSpec) return false;
    FLHSpawnLifeId L; L.Area = ActiveArea; L.SpawnSlot = SpawnId;
    auto* M = LHEncounterDirectorPrivate::Marker(GetWorld(), L); if (!M) return false;
    const auto* S = ResolveSpec(M->EnemyDefinitionId); FString Error;
    if (!S || !LHAI::ValidateSpec(*S,Error)) return false;
    const FVector At = M->GetActorLocation() + FVector(0,0,S->CapsuleHalfHeightCm.Value);
    if (FVector::Dist(At,Player->GetActorLocation()) <= S->SpawnSafetyDistanceCm.Value || HasSight(Player.Get(),M) ||
        !IsSafeSegment(At,At,S->CapsuleRadiusCm.Value,S->CapsuleHalfHeightCm.Value)) return false;
    auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()); FNavLocation Projected;
    if (!Nav || !Nav->ProjectPointToNavigation(M->GetActorLocation(),Projected)) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(LHSpawnSafety), false, M);
    if (GetWorld()->OverlapBlockingTestByChannel(At,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(S->CapsuleRadiusCm.Value,S->CapsuleHalfHeightCm.Value),Params)) return false;
    FHitResult Floor;
    return GetWorld()->LineTraceSingleByChannel(Floor,At,At-FVector(0,0,S->CapsuleHalfHeightCm.Value*2),ECC_WorldStatic,Params);
}
void ULHEncounterDirector::Deinitialize()
{
    bLoaded = false;
    auto Copy = Enemies; Enemies.Reset();
    for (ALHEnemyCharacter* E : Copy) if (IsValid(E)) { if (auto* C = E->GetController()) C->Destroy(); E->Destroy(); }
    ResolveSpec = {}; SettleKill = {}; AdvanceRespawns = {}; Player.Reset(); PublishedDeaths.Reset();
    Super::Deinitialize();
}
