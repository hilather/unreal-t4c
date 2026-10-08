#include "Misc/AutomationTest.h"
#include "EnhancedPlayerInput.h"
#include "EnhancedInputSubsystemInterface.h"
#include "InputKeyEventArgs.h"
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
        // No LocalPlayer startup runs in this transient world. PlayerTick still
        // calls TickPlayerInput, which requires a real PlayerInput instance.
        Controller->InitInputSystem();
        check(Controller->PlayerInput);
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
        Defender->ConfigureAttack(Config, {});
        Defender->SetCombatRandomState(FRandomStream(123));
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDeadPlayerMovementTest, "Lighthaven.Integration.DeadPlayerMovement", Flags)
bool FLHDeadPlayerMovementTest::RunTest(const FString&)
{
    FFixture F;
    // This transient world has no floor; flying isolates locomotion from gravity.
    F.Source->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 10);
    F.Controller->SubmitMovement(FVector2D::ZeroVector);
    F.Controller->SubmitMovement(FVector2D(1,0));
    F.Source->AddMovementInput(FVector::ForwardVector, 1);
    F.Source->GetCharacterMovement()->Velocity=FVector(100,0,0);
    TestTrue(TEXT("Enemy lethal attack accepted"), F.Defender->RequestBasicAttack(F.Attacker)==ELHCommandReason::None);
    TestTrue(TEXT("Lethal hit through combat"), F.Defender->ResolveImpact(F.Defender->GetPendingIdentity()));
    TestFalse(TEXT("Player dead"), F.Attacker->IsAlive());
    TestTrue(TEXT("Dead pawn still possessed"), F.Controller->GetPawn()==F.Source);
    // Keyboard digital and gamepad analog semantic axes share SubmitMovement.
    for (const FVector2D Axis : {FVector2D(1,0), FVector2D(0.4,0.7)})
    {
        F.Controller->SubmitMovement(FVector2D::ZeroVector);
        F.Controller->SubmitMovement(Axis);
        TestTrue(TEXT("Fresh dead input blocked"), F.Controller->GetHeldMovement().IsNearlyZero());
        // Also inject residual motion to exercise the per-tick gate independently of ingress.
        F.Source->AddMovementInput(FVector::ForwardVector, 1);
        F.Source->GetCharacterMovement()->Velocity=FVector(100,0,0);
        for (int32 I=0; I<3; ++I)
        {
            F.Controller->PlayerTick(0.016f);
            F.Source->GetCharacterMovement()->TickComponent(0.016f, LEVELTICK_All, nullptr);
            TestTrue(TEXT("Dead held movement stays zero"), F.Controller->GetHeldMovement().IsNearlyZero());
            TestTrue(TEXT("Dead pending input stays zero"), F.Source->GetPendingMovementInputVector().IsNearlyZero());
            TestTrue(TEXT("Dead velocity stays zero"), F.Source->GetVelocity().IsNearlyZero());
        }
    }
    F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 100);
    F.State->InitializeAvatar(F.Source);
    F.Controller->SubmitMovement(FVector2D(1,0));
    TestTrue(TEXT("Restored live avatar requires release"), F.Controller->GetHeldMovement().IsNearlyZero());
    F.Controller->SubmitMovement(FVector2D::ZeroVector);
    F.Controller->SubmitMovement(FVector2D(1,0));
    TestFalse(TEXT("Restored live avatar accepts fresh movement"), F.Controller->GetHeldMovement().IsNearlyZero());
    F.State->InitializeAvatar(F.Target);
    TestTrue(TEXT("Mismatched avatar is alive"), F.Attacker->IsAlive());
    F.Controller->SubmitMovement(FVector2D::ZeroVector);
    F.Controller->SubmitMovement(FVector2D(1,0));
    TestTrue(TEXT("Mismatched avatar blocks input"), F.Controller->GetHeldMovement().IsNearlyZero());
    F.Source->AddMovementInput(FVector::ForwardVector, 1);
    F.Source->GetCharacterMovement()->Velocity=FVector(100,0,0);
    F.Controller->PlayerTick(0.016f);
    TestTrue(TEXT("Mismatched avatar tick clears pending input"), F.Source->GetPendingMovementInputVector().IsNearlyZero());
    TestTrue(TEXT("Mismatched avatar tick stops velocity"), F.Source->GetVelocity().IsNearlyZero());
    return true;
}
// A native subsystem adapter installs the exact runtime mappings without a window
// or LocalPlayer startup. No movement setter or action injection is used.
namespace
{
struct FInputFixture : IEnhancedInputSubsystemInterface
{
    FFixture Game;
    UEnhancedPlayerInput* Input;
    TMap<TObjectPtr<const UInputAction>, FInjectedInput> Injections;
    FInputFixture()
    {
        Input=CastChecked<UEnhancedPlayerInput>(Game.Controller->PlayerInput);
        FModifyContextOptions Options;
        Options.bForceImmediately=true;
        Options.bIgnoreAllPressedKeysUntilRelease=false;
        AddMappingContext(Game.Controller->InputConfig->Context(ELHInputContext::Gameplay),0,Options);
        Game.Source->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
        Tick(); // Observe actual neutral input before the initial press.
    }
    virtual UEnhancedPlayerInput* GetPlayerInput() const override { return Input; }
    virtual TMap<TObjectPtr<const UInputAction>, FInjectedInput>& GetContinuouslyInjectedInputs() override { return Injections; }
    void Key(EInputEvent Event)
    {
        Game.Controller->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::D,Event,Event==IE_Released ? 0.f : 1.f));
    }
    void Tick()
    {
        Game.Controller->PlayerTick(0.016f);
        Game.Source->GetCharacterMovement()->TickComponent(0.016f,LEVELTICK_All,nullptr);
    }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHHeldMovementAttack, "Lighthaven.Integration.HeldMovementThroughAttack", Flags)
bool FLHHeldMovementAttack::RunTest(const FString&)
{
    FInputFixture F;
    F.Key(IE_Pressed); F.Tick();
    TestTrue(TEXT("Real D mapping produces movement"), !F.Game.Controller->GetHeldMovement().IsNearlyZero());
    TestTrue(TEXT("Select target"), F.Game.Controller->SelectTarget(F.Game.Target));
    F.Game.World->GetTimerManager().Tick(0.f);
    TestTrue(TEXT("Attack accepted"), F.Game.Controller->RequestSelectedAttack()==ELHCommandReason::None);
    TestTrue(TEXT("Clear retains physical D state"), F.Game.Controller->IsInputKeyDown(EKeys::D));
    const FVector Stopped=F.Game.Source->GetActorLocation();
    for (int32 I=0; I<5; ++I)
    {
        F.Key(IE_Repeat); F.Tick();
        TestTrue(TEXT("Held key blocked during pending attack"), F.Game.Controller->GetHeldMovement().IsNearlyZero());
    }
    {
        TGuardValue<uint64> Frame(GFrameCounter,GFrameCounter+1);
        F.Game.World->GetTimerManager().Tick(1.1f);
    }
    TestFalse(TEXT("Attack completed through real timer"), F.Game.Attacker->IsActionPending());
    for (int32 I=0; I<5; ++I)
    {
        F.Key(IE_Repeat); F.Tick();
        TestTrue(TEXT("Held key blocked after completion"), F.Game.Controller->GetHeldMovement().IsNearlyZero());
        TestTrue(TEXT("Pawn stays stopped"), F.Game.Source->GetActorLocation().Equals(Stopped,0.01f));
    }
    F.Key(IE_Released); F.Tick(); F.Key(IE_Pressed); F.Tick();
    TestFalse(TEXT("Release then press resumes mapped movement"), F.Game.Controller->GetHeldMovement().IsNearlyZero());
    TestFalse(TEXT("Fresh press moves pawn"), F.Game.Source->GetActorLocation().Equals(Stopped,0.01f));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHHeldMovementContext, "Lighthaven.Integration.HeldMovementThroughContext", Flags)
bool FLHHeldMovementContext::RunTest(const FString&)
{
    FInputFixture F;
    F.Key(IE_Pressed); F.Tick();
    TestFalse(TEXT("Mapped movement established"), F.Game.Controller->GetHeldMovement().IsNearlyZero());
    F.Game.Controller->SetControlContext(ELHInputContext::UI);
    F.Game.Controller->SetControlContext(ELHInputContext::Gameplay);
    F.Key(IE_Repeat); F.Tick();
    TestTrue(TEXT("Context reentry blocks held key"), F.Game.Controller->GetHeldMovement().IsNearlyZero());
    // Simulate the real engine focus-loss flush, not a semantic neutral setter.
    F.Game.Controller->FlushPressedKeys(); F.Tick();
    F.Key(IE_Repeat); F.Tick();
    TestTrue(TEXT("Engine flush and repeat cannot fake release"), F.Game.Controller->GetHeldMovement().IsNearlyZero());
    F.Key(IE_Released); F.Tick(); F.Key(IE_Pressed); F.Tick();
    TestFalse(TEXT("Actual release and press restores input"), F.Game.Controller->GetHeldMovement().IsNearlyZero());
    return true;
}
#endif
