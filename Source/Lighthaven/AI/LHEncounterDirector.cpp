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
    auto Refuse=[&](const FString& Reason) {
        UE_LOG(LogTemp,Warning,TEXT("LH spawn refused marker=%s encounter=%s area=%s generation=%lld: %s"),
            *R.Life.SpawnSlot.ToString(),*R.Definition.Value.ToString(),*R.Life.Area.Content.Value.ToString(),R.Life.LifeGeneration,*Reason);
    };
    if (!bLoaded) { Refuse(TEXT("director not loaded")); return; }
    if (R.Life.Area.Content.Value != ActiveArea.Content.Value) { Refuse(TEXT("active area mismatch")); return; }
    if (!ResolveSpec || !SettleKill) { Refuse(TEXT("runtime callbacks missing")); return; }
    if (FindByLife(R.Life)) { Refuse(TEXT("life already registered")); return; }
    if (PublishedDeaths.ContainsByPredicate([&](const FLHSpawnLifeId& L) { return LHAI::SameLife(L,R.Life); }))
    { Refuse(TEXT("life death already published")); return; }
    auto* Marker = LHEncounterDirectorPrivate::Marker(GetWorld(), R.Life);
    if (!Marker) { Refuse(TEXT("marker missing")); return; }
    const auto* Spec = ResolveSpec(R.Definition);
    FString Error;
    if (Marker->EnemyDefinitionId.Value != R.Definition.Value) { Refuse(TEXT("marker definition mismatch actual=")+Marker->EnemyDefinitionId.Value.ToString()); return; }
    if (!Spec || Spec->ContentId.Value != R.Definition.Value) { Refuse(TEXT("catalog spec missing/mismatched")); return; }
    if (!LHAI::ValidateSpec(*Spec,Error)) { Refuse(TEXT("ValidateSpec: ")+Error); return; }
    // Markers are floor contacts. Collision must use the runtime capsule, not
    // ACharacter's larger class-default capsule before ApplyRuntimeSpec.
    const FVector Anchor = Marker->GetActorLocation() + FVector(0,0,Spec->CapsuleHalfHeightCm.Value+2);
    FVector Home = Anchor;
    if (Spec->bBoss)
    {
        ATargetPoint* Arena = nullptr;
        for (TActorIterator<ATargetPoint> It(GetWorld()); It; ++It) if (It->ActorHasTag(TEXT("B4.BalorkArena")) || It->GetFName() == FName(TEXT("B4.BalorkArena"))) { Arena = *It; break; }
        if (!Arena) { Refuse(TEXT("B4.BalorkArena missing")); return; }
        Home = Arena->GetActorLocation() + FVector(0,0,Spec->CapsuleHalfHeightCm.Value+2);
        if (FVector::Dist2D(Anchor,Home) > Spec->LeashRadiusCm.Value) { Refuse(FString::Printf(TEXT("boss leash distance=%g limit=%g"),FVector::Dist2D(Anchor,Home),Spec->LeashRadiusCm.Value)); return; }
    }
    const FString Dimensions=FString::Printf(TEXT("center=%s radius=%g halfHeight=%g"),*Anchor.ToString(),Spec->CapsuleRadiusCm.Value,Spec->CapsuleHalfHeightCm.Value);
    if (!IsSafeSegment(Anchor, Anchor, Spec->CapsuleRadiusCm.Value, Spec->CapsuleHalfHeightCm.Value))
    { Refuse(TEXT("NoCombat safety box ")+Dimensions); return; }
    FCollisionQueryParams Params(SCENE_QUERY_STAT(LHInitialSpawn),false,Marker);
    if (GetWorld()->OverlapBlockingTestByChannel(Anchor,Marker->GetActorQuat(),ECC_Pawn,
        FCollisionShape::MakeCapsule(Spec->CapsuleRadiusCm.Value,Spec->CapsuleHalfHeightCm.Value),Params))
    { Refuse(TEXT("runtime capsule blocked ")+Dimensions); return; }
    const FTransform Transform(Marker->GetActorRotation(), Anchor);
    auto* E = GetWorld()->SpawnActorDeferred<ALHEnemyCharacter>(ALHEnemyCharacter::StaticClass(), Transform, nullptr, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!E) { Refuse(TEXT("SpawnActorDeferred failed ")+Dimensions); return; }
    if (!E->ApplyRuntimeSpec(*Spec, R.Life, R.CurrentHealth, Error)) { Refuse(FString::Printf(TEXT("ApplyRuntimeSpec health=%g max=%g: %s"),R.CurrentHealth.Value,Spec->MaxHealth.Value,*Error)); E->Destroy(); return; }
    E->FinishSpawning(Transform);
    if (!IsValid(E) || E->IsActorBeingDestroyed()) { Refuse(TEXT("FinishSpawning destroyed actor ")+Dimensions); return; }
    Enemies.Add(E);
    E->GetCombatComponent()->OnDeath.AddWeakLambda(this, [this, Weak = TWeakObjectPtr<ALHEnemyCharacter>(E)](const FLHHitIdentity& Hit)
        { if (auto* Actor = Weak.Get()) HandleDeath(Actor,Hit); });
    auto* Controller = GetWorld()->SpawnActor<ALHEnemyAIController>();
    if (Controller) Controller->InitializeRuntime(E,this,Home);
    else UE_LOG(LogTemp,Warning,TEXT("LH spawn marker=%s controller creation failed"),*R.Life.SpawnSlot.ToString());
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
    auto Refuse=[&](const TCHAR* Reason) { UE_LOG(LogTemp,Warning,TEXT("LH respawn refused marker=%s area=%s generation=%lld: %s"),*Life.SpawnSlot.ToString(),*Life.Area.Content.Value.ToString(),Life.LifeGeneration,Reason); };
    if (!IsSimulationEnabled()) { Refuse(TEXT("simulation disabled")); return; }
    if (FindByLife(Life)) { Refuse(TEXT("life already registered")); return; }
    auto* M = LHEncounterDirectorPrivate::Marker(GetWorld(),Life);
    if (!M || !ResolveSpec) { Refuse(TEXT("marker or catalog callback missing")); return; }
    FString SafetyError;
    if (!IsSpawnSafe(Life.SpawnSlot,&SafetyError)) { Refuse(*SafetyError); return; }
    const auto* S = ResolveSpec(M->EnemyDefinitionId); if (!S) { Refuse(TEXT("catalog spec missing")); return; }
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
{ for (ALHEnemyCharacter* E : Enemies) if (IsValid(E) && !E->IsActorBeingDestroyed() && E->IsAlive() && LHEncounterDirectorPrivate::SameEntity(E->GetEntityId(RunId),Entity)) return E; return nullptr; }
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
bool ULHEncounterDirector::IsSpawnSafe(const FGuid& SpawnId,FString* Refusal) const
{
    // Respawn polling is not an attempted spawn. Supply diagnostics for SpawnLife
    // without logging every frame while a legitimate D09 gate remains closed.
    auto Fail=[&](const FString& Why) { if (Refusal) *Refusal=Why; return false; };
    if (!IsSimulationEnabled() || !Player.IsValid() || !ResolveSpec) return Fail(TEXT("simulation/player/catalog unavailable"));
    FLHSpawnLifeId L; L.Area = ActiveArea; L.SpawnSlot = SpawnId;
    auto* M = LHEncounterDirectorPrivate::Marker(GetWorld(), L); if (!M) return Fail(TEXT("marker missing"));
    const auto* S = ResolveSpec(M->EnemyDefinitionId); FString Error;
    if (!S || !LHAI::ValidateSpec(*S,Error)) return Fail(TEXT("invalid catalog spec: ")+Error);
    const FVector At = M->GetActorLocation() + FVector(0,0,S->CapsuleHalfHeightCm.Value+2);
    const FString Values=FString::Printf(TEXT(" center=%s radius=%g halfHeight=%g"),*At.ToString(),S->CapsuleRadiusCm.Value,S->CapsuleHalfHeightCm.Value);
    const double Distance=FVector::Dist(At,Player->GetActorLocation());
    if (Distance <= S->SpawnSafetyDistanceCm.Value) return Fail(FString::Printf(TEXT("player distance=%g minimum=%g"),Distance,S->SpawnSafetyDistanceCm.Value)+Values);
    if (HasSight(Player.Get(),M)) return Fail(TEXT("visible to player")+Values);
    if (!IsSafeSegment(At,At,S->CapsuleRadiusCm.Value,S->CapsuleHalfHeightCm.Value)) return Fail(TEXT("NoCombat safety box")+Values);
    auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()); FNavLocation Projected;
    if (!Nav || !Nav->ProjectPointToNavigation(M->GetActorLocation(),Projected)) return Fail(TEXT("navigation projection missing")+Values);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(LHSpawnSafety), false, M);
    if (GetWorld()->OverlapBlockingTestByChannel(At,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(S->CapsuleRadiusCm.Value,S->CapsuleHalfHeightCm.Value),Params)) return Fail(TEXT("runtime capsule blocked")+Values);
    FHitResult Floor;
    if (!GetWorld()->LineTraceSingleByChannel(Floor,At,At-FVector(0,0,S->CapsuleHalfHeightCm.Value*2),ECC_WorldStatic,Params)) return Fail(TEXT("floor trace missed")+Values);
    return true;
}
void ULHEncounterDirector::Deinitialize()
{
    bLoaded = false;
    auto Copy = Enemies; Enemies.Reset();
    for (ALHEnemyCharacter* E : Copy) if (IsValid(E)) { if (auto* C = E->GetController()) C->Destroy(); E->Destroy(); }
    ResolveSpec = {}; SettleKill = {}; AdvanceRespawns = {}; Player.Reset(); PublishedDeaths.Reset();
    Super::Deinitialize();
}
