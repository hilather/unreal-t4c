#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "AI/LHEnemyRuntime.h"
#include "AI/LHEnemyAIController.h"
#include "AI/LHEncounterDirector.h"
#include "Framework/LHEnemyCharacter.h"
#include "World/LHWorldMarkers.h"
#include "Abilities/LHAttributeSet.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerState.h"
#include "Engine/TriggerBox.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavMesh/RecastNavMesh.h"
#include "Components/BrushComponent.h"
#include "UObject/UnrealType.h"
#include "Builders/CubeBuilder.h"
#include "ActorFactories/ActorFactory.h"
#include "AssetCompilingManager.h"
namespace LHAITestsPrivate
{
constexpr auto LHAITestsFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
FLHNumber Number(double V)
{
    FLHNumber N; N.Resolution = ELHValueResolution::Resolved; N.Value = V;
    N.Provenance.Status = ELHProvenanceStatus::Prototype;
    N.Provenance.SourceBaseline = TEXT("W4-01 synthetic automation fixture, not gameplay tuning"); return N;
}
FLHInteger Integer(int64 V) { FLHInteger I; I.Resolution = ELHValueResolution::Resolved; I.Value = V; I.Provenance = Number(0).Provenance; return I; }
FLHEnemyRuntimeSpec Spec()
{
    FLHEnemyRuntimeSpec S; S.ContentId.Value = TEXT("Enemy.BrownRat"); S.PresentationId.Value = TEXT("Presentation.Enemy.BrownRat");
    S.CapsuleRadiusCm = Number(25); S.CapsuleHalfHeightCm = Number(25); S.MaxHealth = Number(10);
    S.Accuracy = S.Avoidance = S.Armor = S.Resistance = S.DamageBonus = S.MaxMana = Number(0);
    S.AggroRadiusCm = Number(500); S.LeashRadiusCm = Number(1000); S.MoveSpeedCmPerSec = Number(100);
    S.PathRetryLimit = Number(3); S.MaxActivePursuers = Number(2); S.DecisionIntervalSeconds = Number(.1);
    S.HomeToleranceCm = Number(10); S.SpawnSafetyDistanceCm = Number(1000);
    auto& A = S.Attack;
    A.Combat.HitBase = A.Combat.MinimumChance = A.Combat.MaximumChance = A.Combat.DamageQuantum = Number(1);
    A.Combat.AccuracyScale = A.Combat.AvoidanceScale = A.Combat.ArmorScale = A.Combat.ResistanceScale = A.Combat.MinimumDamage = Number(0);
    A.RequirementPolicy.Basis = LH::Rules::EAttributeBasis::Base; A.RequirementPolicy.BasisProvenance = Number(0).Provenance;
    A.Eligibility.MinimumLevel = Integer(0);
    auto& B = A.Eligibility.MinimumAttributes; B.Strength = B.Endurance = B.Agility = B.Intelligence = B.Wisdom = Integer(0);
    A.ManaCost = A.CooldownSeconds = Number(0); A.ImpactSeconds = Number(1); A.RangeCm = Number(150);
    A.WeaponMinimum = A.WeaponMaximum = Number(10);
    return S;
}
TArray<FLHMechanicalField> Parameters()
{
    auto S = Spec();
    const FLHNumber N[] = {S.AggroRadiusCm,S.LeashRadiusCm,S.MoveSpeedCmPerSec,S.PathRetryLimit,S.MaxActivePursuers,
        S.DecisionIntervalSeconds,S.HomeToleranceCm,S.SpawnSafetyDistanceCm};
    TArray<FLHMechanicalField> Fields;
    for (int I = 0; I < UE_ARRAY_COUNT(N); ++I) { FLHMechanicalField F; F.Key = LHAI::AllowedParameterKeys()[I].Name; F.Value = N[I]; Fields.Add(F); }
    return Fields;
}
struct FWorldFixture
{
    UWorld* World;
    ULHEncounterDirector* D;
    FLHEnemyRuntimeSpec S = Spec();
    FLHAreaRecord Area;
    ALHEnemyCharacter* Player;
    TArray<ALHSpawnMarker*> Markers;
    FWorldFixture()
    {
        auto Init = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).RequiresHitProxies(false)
            .CreateNavigation(true).CreateAISystem(true).ShouldSimulatePhysics(false).SetTransactional(false);
        auto& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
        World = UWorld::CreateWorld(EWorldType::Game,false,MakeUniqueObjectName(nullptr,UWorld::StaticClass(),TEXT("LHAITestWorld"),EUniqueObjectNameOptions::GloballyUnique),
            GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
        Context.SetCurrentWorld(World); World->InitializeActorsForPlay(FURL());
        D = World->GetSubsystem<ULHEncounterDirector>();
        Area.Area.Content.Value = TEXT("Area.Test");
        D->RunId = FGuid::NewGuid();
        D->ResolveSpec = [this](const FLHContentId& Id) { return Id.Value == S.ContentId.Value ? &S : nullptr; };
        D->SettleKill = [](const FLHSpawnLifeId&,const FLHHitIdentity&,AActor*) { return true; };
        FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Player = World->SpawnActor<ALHEnemyCharacter>(FVector(100,0,25),FRotator::ZeroRotator,P);
        auto* C = Player->GetCombatComponent();
        C->SetNumericAttributeBase(ULHAttributeSet::GetMaxHealthAttribute(),100);
        C->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),100);
        C->ConfigureAttack(S.Attack,{}); C->SetCombatRandomState(FRandomStream(1)); Player->InitializeAfterRestore();
        D->Player = Player;
    }
    ~FWorldFixture() { D->Deinitialize(); World->DestroyWorld(false); GEngine->DestroyWorldContext(World); }
    FLHSpawnLifeId Add(ELHEncounterLifeState State, FVector Ground = FVector::ZeroVector)
    {
        auto* M = World->SpawnActor<ALHSpawnMarker>(Ground,FRotator::ZeroRotator);
        M->Area = Area.Area; M->SpawnId = FGuid::NewGuid(); M->EnemyDefinitionId = S.ContentId; Markers.Add(M);
        FLHEncounterRecord R; R.Life.Area = Area.Area; R.Life.SpawnSlot = M->SpawnId;
        R.Definition = S.ContentId; R.State = State; R.CurrentHealth = Number(7); Area.Encounters.Add(R); return R.Life;
    }
    ALHEnemyAIController* AI(const FLHSpawnLifeId& L) { auto* E = D->FindByLife(L); return E ? Cast<ALHEnemyAIController>(E->GetController()) : nullptr; }
    bool BuildNavigation()
    {
        auto* Floor = World->SpawnActor<AStaticMeshActor>(FVector(0,0,-10),FRotator::ZeroRotator);
        auto* Mesh = Floor->GetStaticMeshComponent(); Mesh->SetMobility(EComponentMobility::Static);
        Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
        Mesh->SetWorldScale3D(FVector(40,40,.2)); Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetCollisionResponseToAllChannels(ECR_Block);
        FAssetCompilingManager::Get().FinishAllCompilation(); Mesh->RecreatePhysicsState();
        auto* Volume = World->SpawnActor<ANavMeshBoundsVolume>(FVector(0,0,100),FRotator::ZeroRotator);
        auto* Builder = NewObject<UCubeBuilder>(); Builder->X=4000; Builder->Y=4000; Builder->Z=1000;
        UActorFactory::CreateBrushForVolumeActor(Volume,Builder);
        FNavigationSystem::AddNavigationSystemToWorld(*World,FNavigationSystemRunMode::EditorMode);
        auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (!Nav) return false;
        auto* Wait = FindFProperty<FBoolProperty>(Nav->GetClass(),TEXT("bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically"));
        if (Wait) Wait->SetPropertyValue_InContainer(Nav,false);
        // Assets are synchronously compiled above; this synthetic world cannot service
        // the editor's delayed async-load unlock while RunTest is on the stack.
        Nav->RemoveNavigationBuildLock(ENavigationBuildLock::AsyncLoadLock,UNavigationSystemV1::ELockRemovalRebuildAction::NoRebuild);
        Nav->OnNavigationBoundsUpdated(Volume);
        FNavigationSystem::Build(*World);
        FNavLocation At; return Nav->ProjectPointToNavigation(FVector::ZeroVector,At);
    }
    ATriggerBox* Safe(FVector At)
    {
        auto* B = World->SpawnActor<ATriggerBox>(At,FRotator::ZeroRotator); B->Tags.Add(TEXT("LH.Safety.NoCombat.Required"));
        auto* C = CastChecked<UBoxComponent>(B->GetCollisionComponent()); C->SetBoxExtent(FVector(40)); C->SetCollisionEnabled(ECollisionEnabled::NoCollision); C->UpdateBounds(); return B;
    }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHAISpecTest,"Lighthaven.AI.SpecValidation",LHAITestsPrivate::LHAITestsFlags)
bool FLHAISpecTest::RunTest(const FString&)
{
    using namespace LHAITestsPrivate;
    auto S = Spec(); FString Error; auto Fields = Parameters();
    TestTrue(TEXT("Complete synthetic spec validates"), LHAI::ValidateSpec(S,Error));
    TestTrue(TEXT("Allowlisted parameters apply"), LHAI::ApplyParameters(Fields,S,Error));
    const auto Duplicate = Fields[0]; Fields.Add(Duplicate);
    TestFalse(TEXT("Duplicate rejects"), LHAI::ApplyParameters(Fields,S,Error)); Fields.Pop();
    Fields[0].Key = TEXT("AI.Unknown"); TestFalse(TEXT("Unknown rejects"),LHAI::ApplyParameters(Fields,S,Error));
    Fields = Parameters(); Fields[0].Value.Resolution = ELHValueResolution::Unresolved;
    TestFalse(TEXT("Unresolved rejects"),LHAI::ApplyParameters(Fields,S,Error));
    TestEqual(TEXT("Rejected update atomic"),S.AggroRadiusCm.Value,500.0);
    Fields = Parameters(); Fields.Pop(); TestFalse(TEXT("Missing required key rejects"),LHAI::ApplyParameters(Fields,S,Error));
    S.Attack.RangeCm = Number(-1); TestFalse(TEXT("Invalid attack rejects"),LHAI::ValidateSpec(S,Error));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHAIStateTest,"Lighthaven.AI.StateMachine",LHAITestsPrivate::LHAITestsFlags)
bool FLHAIStateTest::RunTest(const FString&)
{
    using namespace LHAITestsPrivate;
    FWorldFixture F; auto Life = F.Add(ELHEncounterLifeState::Alive); F.D->Populate(F.Area);
    auto* C = F.AI(Life); if (!TestNotNull(TEXT("Controller spawned"),C)) return false;
    C->Advance(.2); TestTrue(TEXT("Acquires with sight"),C->GetState()==ELHAIState::Chase);
    C->Advance(.2); TestTrue(TEXT("Attacks through component"),F.D->FindByLife(Life)->GetCombatComponent()->IsActionPending());
    F.Player->SetActorLocation(FVector(1500,0,25)); C->Advance(.2);
    TestTrue(TEXT("Target leashed"),C->GetState()==ELHAIState::ReturnHome);
    C->Advance(.2); TestTrue(TEXT("Home returns idle"),C->GetState()==ELHAIState::Idle);
    F.Player->SetActorLocation(FVector(400,0,25));
    TestTrue(TEXT("Synthetic Recast mesh built"),F.BuildNavigation());
    C->Advance(.2); C->Advance(.2);
    TestTrue(TEXT("Navigable chase remains active"),C->GetState()==ELHAIState::Chase);
    TestEqual(TEXT("Navigable request succeeds"),C->GetFailedPaths(),0);
    C->EnterDead(); C->Advance(10); TestTrue(TEXT("Dead terminal"),C->GetState()==ELHAIState::Dead);
    // Real visibility collision, including a positive blocking control.
    FWorldFixture W; auto L = W.Add(ELHEncounterLifeState::Alive);
    auto* Wall = W.World->SpawnActor<ATriggerBox>(FVector(50,0,25),FRotator::ZeroRotator);
    auto* Box = CastChecked<UBoxComponent>(Wall->GetCollisionComponent()); Box->SetBoxExtent(FVector(10,100,100));
    Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly); Box->SetCollisionResponseToAllChannels(ECR_Ignore); Box->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block); Box->RecreatePhysicsState();
    W.D->Populate(W.Area); auto* WC = W.AI(L); if (!TestNotNull(TEXT("Wall controller"),WC)) return false;
    TestFalse(TEXT("Wall blocks sight control"),W.D->HasSight(W.D->FindByLife(L),W.Player));
    WC->Advance(.2); TestTrue(TEXT("No acquire through wall"),WC->GetState()==ELHAIState::Idle);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHAIPathTest,"Lighthaven.AI.BoundedPathRetry",LHAITestsPrivate::LHAITestsFlags)
bool FLHAIPathTest::RunTest(const FString&)
{
    using namespace LHAITestsPrivate;
    FWorldFixture F; F.Player->SetActorLocation(FVector(400,0,25)); auto L = F.Add(ELHEncounterLifeState::Alive); F.D->Populate(F.Area);
    auto* C = F.AI(L); if (!TestNotNull(TEXT("Controller"),C)) return false;
    C->Advance(.2);
    // World has no navigable geometry; real path requests reject, no injected result.
    C->Advance(.2); TestEqual(TEXT("First failed request"),C->GetFailedPaths(),1);
    C->Advance(.2); TestEqual(TEXT("Second failed request"),C->GetFailedPaths(),2);
    C->Advance(.2); TestTrue(TEXT("Third failure returns home"),C->GetState()==ELHAIState::ReturnHome);
    C->Advance(0); TestTrue(TEXT("No zero-time retry spin"),C->GetState()==ELHAIState::ReturnHome);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHAISafeTest,"Lighthaven.AI.SafeZone",LHAITestsPrivate::LHAITestsFlags)
bool FLHAISafeTest::RunTest(const FString&)
{
    using namespace LHAITestsPrivate;
    FWorldFixture F; F.Safe(F.Player->GetActorLocation()); auto L = F.Add(ELHEncounterLifeState::Alive); F.D->Populate(F.Area);
    TestFalse(TEXT("NoCombat segment control"),F.D->IsSafeSegment(FVector::ZeroVector,F.Player->GetActorLocation()));
    auto* C = F.AI(L); if (!TestNotNull(TEXT("Controller"),C)) return false;
    C->Advance(.2); C->Advance(.2);
    TestTrue(TEXT("No safe-zone acquire"),C->GetState()==ELHAIState::Idle);
    TestFalse(TEXT("No pending safe-zone attack"),F.D->FindByLife(L)->GetCombatComponent()->IsActionPending());
    FWorldFixture W; auto WL = W.Add(ELHEncounterLifeState::Alive);
    auto* Zone = W.Safe(FVector(140,0,25)); CastChecked<UBoxComponent>(Zone->GetCollisionComponent())->SetBoxExtent(FVector(10));
    Zone->GetCollisionComponent()->UpdateBounds(); W.D->Populate(W.Area);
    auto* WC = W.AI(WL); if (!TestNotNull(TEXT("Windup controller"),WC)) return false;
    WC->Advance(.2); WC->Advance(.2);
    auto* Combat = W.D->FindByLife(WL)->GetCombatComponent();
    TestTrue(TEXT("Windup started outside safety"),Combat->IsActionPending());
    const auto Hit = Combat->GetPendingIdentity(); W.Player->SetActorLocation(FVector(140,0,25)); WC->Advance(.2);
    TestFalse(TEXT("Entering safety cancels pending hit"),Combat->ResolveImpact(Hit));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHAITwoTest,"Lighthaven.AI.TwoEnemiesOneSettlement",LHAITestsPrivate::LHAITestsFlags)
bool FLHAITwoTest::RunTest(const FString&)
{
    using namespace LHAITestsPrivate;
    FWorldFixture F; auto L1=F.Add(ELHEncounterLifeState::Alive); auto L2=F.Add(ELHEncounterLifeState::Alive,FVector(0,100,0));
    int32 Calls=0; F.D->SettleKill=[&](const FLHSpawnLifeId&,const FLHHitIdentity&,AActor* Killer) { TestTrue(TEXT("Player attribution"),Killer==F.Player); ++Calls; return true; };
    F.D->Populate(F.Area); auto* C1=F.AI(L1); auto* C2=F.AI(L2);
    if (!TestNotNull(TEXT("First controller"),C1) || !TestNotNull(TEXT("Second controller"),C2)) return false;
    C1->Advance(.2); C2->Advance(.2); TestTrue(TEXT("Two pursue"),C1->IsPursuing() && C2->IsPursuing());
    auto* PC=F.Player->GetCombatComponent();
    for (const auto& L : {L1,L2})
    {
        auto* E=F.D->FindByLife(L); auto* EC=E->GetCombatComponent();
        TestTrue(TEXT("Lethal attack commits"),PC->RequestBasicAttack(EC)==ELHCommandReason::None);
        auto Hit=PC->GetPendingIdentity();
        ULHCombatComponent* Contender = nullptr; FLHHitIdentity OtherHit;
        if (LHAI::SameLife(L,L1))
        {
            Contender = F.D->FindByLife(L2)->GetCombatComponent();
            TestTrue(TEXT("Second lethal activation pending on same life"),Contender->RequestBasicAttack(EC)==ELHCommandReason::None);
            OtherHit = Contender->GetPendingIdentity();
        }
        TestTrue(TEXT("Lethal impact"),PC->ResolveImpact(Hit));
        if (Contender) { TestFalse(TEXT("Simultaneous second lethal rejected on dead target"),Contender->ResolveImpact(OtherHit)); Contender->CancelAllAbilities(); }
        TestFalse(TEXT("Duplicate lethal rejected"),PC->ResolveImpact(Hit));
        PC->CancelAllAbilities();
        // Reentrant duplicate publication also hits director latch.
        EC->OnDeath.Broadcast(Hit); TestTrue(TEXT("Actor retained as corpse"),E->IsCorpse());
    }
    TestEqual(TEXT("Each life settles once"),Calls,2);
    F.D->Populate(F.Area); TestEqual(TEXT("Stale Alive hydration does not resurrect published death"),Calls,2);
    TestNull(TEXT("Published life suppressed"),F.D->FindByLife(L1)); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHAIPopulateTest,"Lighthaven.AI.DirectorPopulateAndCapture",LHAITestsPrivate::LHAITestsFlags)
bool FLHAIPopulateTest::RunTest(const FString&)
{
    using namespace LHAITestsPrivate;
    FWorldFixture F; auto Alive=F.Add(ELHEncounterLifeState::Alive);
    auto Dead=F.Add(ELHEncounterLifeState::Dead,FVector(0,200,0)); auto Pending=F.Add(ELHEncounterLifeState::RespawnPending,FVector(0,400,0));
    auto Permanent=F.Add(ELHEncounterLifeState::PermanentlyDefeated,FVector(0,600,0)); F.D->Populate(F.Area);
    auto* E=F.D->FindByLife(Alive); if (!TestNotNull(TEXT("Alive spawned"),E)) return false;
    TestEqual(TEXT("Persisted health installed"),E->GetCombatComponent()->GetCombatAttributes()->GetHealth(),7.f);
    TestNull(TEXT("Dead absent"),F.D->FindByLife(Dead)); TestNull(TEXT("Pending absent"),F.D->FindByLife(Pending)); TestNull(TEXT("Permanent absent"),F.D->FindByLife(Permanent));
    E->GetCombatComponent()->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),4); F.D->CaptureLive(F.Area);
    TestEqual(TEXT("Live health captured"),F.Area.Encounters[0].CurrentHealth.Value,4.0);
    F.D->Populate(F.Area); TestEqual(TEXT("Reload health"),F.D->FindByLife(Alive)->GetCombatComponent()->GetCombatAttributes()->GetHealth(),4.f);
    F.Area.Encounters[0].State=ELHEncounterLifeState::Dead; F.D->Populate(F.Area); TestNull(TEXT("Reload never resurrects dead"),F.D->FindByLife(Alive)); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHAIPauseTest,"Lighthaven.AI.PausedAndUnloadedNoTick",LHAITestsPrivate::LHAITestsFlags)
bool FLHAIPauseTest::RunTest(const FString&)
{
    using namespace LHAITestsPrivate;
    FWorldFixture F; int32 Calls=0; float Time=0; F.D->AdvanceRespawns=[&](float S) { ++Calls; Time+=S; };
    F.D->TickActiveSimulation(2); TestEqual(TEXT("Unpopulated floor inactive"),Calls,0);
    F.D->Populate(F.Area); F.D->TickActiveSimulation(0); TestEqual(TEXT("Zero time inactive"),Calls,0);
    F.D->SetSimulationFrozen(true); F.D->TickActiveSimulation(2); TestEqual(TEXT("Travel freeze inactive"),Calls,0);
    F.D->SetSimulationFrozen(false);
    auto* Pauser = F.World->SpawnActor<APlayerState>(); F.World->GetWorldSettings()->SetPauserPlayerState(Pauser);
    F.D->TickActiveSimulation(2); TestEqual(TEXT("Paused stops respawn time"),Calls,0);
    F.World->GetWorldSettings()->SetPauserPlayerState(nullptr);
    F.D->TickActiveSimulation(2); TestEqual(TEXT("Active floor advances"),Calls,1); TestEqual(TEXT("Exact active time"),Time,2.f);
    F.D->Deinitialize(); F.D->TickActiveSimulation(2); TestEqual(TEXT("Unloaded stops"),Calls,1); return true;
}
#endif
