#include "Misc/AutomationTest.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Framework/LHEnemyCharacter.h"
#include "Framework/LHCharacter.h"
#include "Framework/LHPlayerController.h"
#include "Framework/LHPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "CoreGlobals.h"
#include "UObject/UObjectGlobals.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace
{
FLHNumber Number(double V)
{
    FLHNumber N; N.Resolution = ELHValueResolution::Resolved; N.Value = V;
    N.Provenance.Status = ELHProvenanceStatus::Prototype;
    N.Provenance.SourceBaseline = TEXT("W1-INT generated dev-map fixture / Prototype / 2026-10-08; no historical source");
    return N;
}
FLHInteger Integer(int64 V)
{
    FLHInteger I; I.Resolution = ELHValueResolution::Resolved; I.Value = V;
    I.Provenance = Number(0).Provenance; return I;
}
struct FFixture
{
    UWorld* World;
    ALHCharacter* Source;
    ALHPlayerController* Controller;
    ALHPlayerState* State;
    ALHEnemyCharacter* Target;
    ULHCombatComponent* Attacker;
    ULHCombatComponent* Defender;
    FFixture()
    {
        const FName WorldName = MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("LHIntegrationTestWorld"),
            EUniqueObjectNameOptions::GloballyUnique);
        const UWorld::InitializationValues Initialization = UWorld::InitializationValues()
            .AllowAudioPlayback(false).CreatePhysicsScene(true).RequiresHitProxies(false)
            .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false);
        FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
        // CreateWorld initializes the level and WorldSettings once; never InitializeNewWorld again.
        World = UWorld::CreateWorld(EWorldType::Game, false, WorldName, GetTransientPackage(), true,
            ERHIFeatureLevel::Num, &Initialization);
        check(World);
        Context.SetCurrentWorld(World);
        World->InitializeActorsForPlay(FURL());
        // No game mode or world ticking is needed: avatar initialization and impacts are explicit below.
        FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Controller = World->SpawnActor<ALHPlayerController>();
        State = World->SpawnActor<ALHPlayerState>();
        Controller->SetPlayerState(State);
        Source = World->SpawnActor<ALHCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
        Target = World->SpawnActor<ALHEnemyCharacter>(FVector(100, 0, 0), FRotator::ZeroRotator, Spawn);
        Attacker = Controller->GetPlayerState<ALHPlayerState>()->GetCombatComponent(); Defender = Target->GetCombatComponent();
        for (auto* C : {Attacker, Defender})
        {
            C->SetNumericAttributeBase(ULHAttributeSet::GetMaxHealthAttribute(), 100);
            C->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 100);
            C->SetNumericAttributeBase(ULHAttributeSet::GetMaxManaAttribute(), 10);
            C->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(), 10);
        }
        Controller->Possess(Source); Target->InitializeAfterRestore();
        FLHBasicAttackConfig Config;
        Config.Combat.HitBase = Number(1); Config.Combat.AccuracyScale = Number(0); Config.Combat.AvoidanceScale = Number(0);
        Config.Combat.MinimumChance = Number(1); Config.Combat.MaximumChance = Number(1);
        Config.Combat.ArmorScale = Number(0); Config.Combat.ResistanceScale = Number(0);
        Config.Combat.MinimumDamage = Number(0); Config.Combat.DamageQuantum = Number(1);
        Config.RequirementPolicy.Basis = LH::Rules::EAttributeBasis::Base;
        Config.RequirementPolicy.BasisProvenance = Number(0).Provenance;
        Config.Eligibility.MinimumLevel = Integer(0);
        auto& A = Config.Eligibility.MinimumAttributes;
        A.Strength = Integer(0); A.Endurance = Integer(0); A.Agility = Integer(0); A.Intelligence = Integer(0); A.Wisdom = Integer(0);
        Config.ManaCost = Number(2); Config.CooldownSeconds = Number(3); Config.ImpactSeconds = Number(1);
        Config.RangeCm = Number(200); Config.WeaponMinimum = Number(10); Config.WeaponMaximum = Number(10);
        Attacker->ConfigureAttack(Config, {});
        Attacker->SetCombatRandomState(FRandomStream(123));
    }
    ~FFixture()
    {
        // Cancel GAS actions/timers while their world and avatars are still available.
        Attacker->ClearCombatAvatar();
        Defender->ClearCombatAvatar();
        if (!Target->IsActorBeingDestroyed()) { Target->Destroy(); }
        if (!Source->IsActorBeingDestroyed()) { Source->Destroy(); }
        Controller->UnPossess();
        Controller->Destroy();
        if (!State->IsActorBeingDestroyed()) State->Destroy();
        World->DestroyWorld(false);
        GEngine->DestroyWorldContext(World);
    }
    FFixture(const FFixture&) = delete;
    FFixture& operator=(const FFixture&) = delete;
};
constexpr auto Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHControlsCombatIntegration, "Lighthaven.Integration.ControlsCombat", Flags)
bool FLHControlsCombatIntegration::RunTest(const FString&)
{
    FFixture F;
    TestTrue(TEXT("Pawn forwards PlayerState ASC"), F.Source->GetAbilitySystemComponent()==F.Attacker);
    F.Target->SetActorLocation(FVector(300,0,0));
    F.Controller->CycleTarget(1);
    TestNull(TEXT("Out of combat range excluded"), F.Controller->GetSelectedTarget());
    F.Target->SetActorLocation(FVector(100,0,0));
    F.Controller->CycleTarget(1);
    TestTrue(TEXT("Cycling selects eligible combat dummy"), F.Controller->GetSelectedTarget()==F.Target);
    TestTrue(TEXT("Select combat dummy"), F.Controller->SelectTarget(F.Target));
    int32 Impacts=0, Deaths=0;
    F.Attacker->OnImpact.AddLambda([&](const FLHHitIdentity&, const LH::Rules::FCombatResult&) { ++Impacts; });
    F.Defender->OnDeath.AddLambda([&](const FLHHitIdentity&) { ++Deaths; });
    F.Defender->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 10);
    F.World->GetTimerManager().Tick(0.f); // Activate subsequently created timers immediately.
    TestTrue(TEXT("Controller requests validated attack"), F.Controller->RequestSelectedAttack()==ELHCommandReason::None);
    const auto Id=F.Attacker->GetPendingIdentity();
    TestEqual(TEXT("No damage before explicit impact"), F.Defender->GetCombatAttributes()->GetHealth(), 10.f);
    // Advance the actual ability timer without viewport/physics movement ticks.
    {
        TGuardValue<uint64> Frame(GFrameCounter, GFrameCounter+1);
        F.World->GetTimerManager().Tick(1.1f);
    }
    TestEqual(TEXT("One impact"), Impacts, 1);
    TestEqual(TEXT("One death"), Deaths, 1);
    TestEqual(TEXT("Lethal damage clamps health"), F.Defender->GetCombatAttributes()->GetHealth(), 0.f);
    TestFalse(TEXT("Duplicate callback rejected"), F.Attacker->ResolveImpact(Id));
    TestFalse(TEXT("Dead dummy excluded"), F.Controller->SelectTarget(F.Target));
    F.Controller->CycleTarget(1);
    TestNull(TEXT("Cycling excludes dead dummy"), F.Controller->GetSelectedTarget());
    F.Controller->UnPossess();
    TestEqual(TEXT("Source health retained through unpossess"), F.Attacker->GetCombatAttributes()->GetHealth(), 100.f);
    TestFalse(TEXT("Unpossess clears live combat avatar"), F.Attacker->IsAlive());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHContextIntegration, "Lighthaven.Integration.ContextClearsMovement", Flags)
bool FLHContextIntegration::RunTest(const FString&)
{
    FFixture F;
    F.Controller->SubmitMovement(FVector2D::ZeroVector);
    F.Controller->SubmitMovement(FVector2D(1,1));
    F.Source->AddMovementInput(FVector::ForwardVector, 1);
    F.Source->GetCharacterMovement()->Velocity=FVector(100,0,0);
    TestFalse(TEXT("Held input established"), F.Controller->GetHeldMovement().IsNearlyZero());
    F.Controller->SetControlContext(ELHInputContext::UI);
    TestTrue(TEXT("Held axes cleared"), F.Controller->GetHeldMovement().IsNearlyZero());
    TestTrue(TEXT("Pending pawn input consumed"), F.Source->GetPendingMovementInputVector().IsNearlyZero());
    TestTrue(TEXT("Velocity stopped"), F.Source->GetVelocity().IsNearlyZero());
    F.Controller->SetControlContext(ELHInputContext::Gameplay);
    F.Controller->SubmitMovement(FVector2D(1,0));
    TestTrue(TEXT("Held stick blocked until neutral"), F.Controller->GetHeldMovement().IsNearlyZero());
    F.Controller->SubmitMovement(FVector2D::ZeroVector);
    F.Controller->SubmitMovement(FVector2D(1,0));
    TestFalse(TEXT("Neutral then new input accepted"), F.Controller->GetHeldMovement().IsNearlyZero());
    return true;
}
#endif
