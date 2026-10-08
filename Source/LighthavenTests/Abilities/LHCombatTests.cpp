#include "Misc/AutomationTest.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Framework/LHEnemyCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"
#include "CoreGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace
{
FLHNumber Number(double V)
{
    FLHNumber N; N.Resolution = ELHValueResolution::Resolved; N.Value = V;
    N.Provenance.Status = ELHProvenanceStatus::Prototype;
    N.Provenance.SourceBaseline = TEXT("W1-03 synthetic Automation fixture, not play tuning");
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
    ALHEnemyCharacter* Source;
    ALHEnemyCharacter* Target;
    ULHCombatComponent* Attacker;
    ULHCombatComponent* Defender;
    FFixture(bool bZeroCooldown = false)
    {
        const FName WorldName = MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("LHCombatTestWorld"),
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
        Source = World->SpawnActor<ALHEnemyCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
        Target = World->SpawnActor<ALHEnemyCharacter>(FVector(100, 0, 0), FRotator::ZeroRotator, Spawn);
        Attacker = Source->GetCombatComponent(); Defender = Target->GetCombatComponent();
        for (auto* C : {Attacker, Defender})
        {
            C->SetNumericAttributeBase(ULHAttributeSet::GetMaxHealthAttribute(), 100);
            C->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 100);
            C->SetNumericAttributeBase(ULHAttributeSet::GetMaxManaAttribute(), 10);
            C->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(), 10);
        }
        Source->InitializeAfterRestore(); Target->InitializeAfterRestore();
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
        Config.ManaCost = Number(2); Config.CooldownSeconds = Number(bZeroCooldown ? 0 : 3); Config.ImpactSeconds = Number(1);
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
        World->DestroyWorld(false);
        GEngine->DestroyWorldContext(World);
    }
    FFixture(const FFixture&) = delete;
    FFixture& operator=(const FFixture&) = delete;
};
constexpr auto Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDestroyedTargetTest, "Lighthaven.Abilities.DestroyedTarget", Flags)
bool FLHDestroyedTargetTest::RunTest(const FString&)
{
    FFixture F;
    TestTrue(TEXT("Activate GAS attack"), F.Attacker->RequestBasicAttack(F.Defender) == ELHCommandReason::None);
    auto Id = F.Attacker->GetPendingIdentity();
    F.Target->Destroy();
    TestFalse(TEXT("Destroyed avatar rejected at impact"), F.Attacker->ResolveImpact(Id));
    TestEqual(TEXT("No damage"), F.Defender->GetCombatAttributes()->GetHealth(), 100.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDuplicateImpactTest, "Lighthaven.Abilities.DuplicateImpact", Flags)
bool FLHDuplicateImpactTest::RunTest(const FString&)
{
    FFixture F;
    TestTrue(TEXT("Activate"), F.Attacker->RequestBasicAttack(F.Defender) == ELHCommandReason::None);
    auto Id = F.Attacker->GetPendingIdentity();
    TestTrue(TEXT("First impact"), F.Attacker->ResolveImpact(Id));
    TestFalse(TEXT("Duplicate rejected"), F.Attacker->ResolveImpact(Id));
    TestEqual(TEXT("Exactly one damage application"), F.Defender->GetCombatAttributes()->GetHealth(), 90.f);
    TestEqual(TEXT("Exactly one cost"), F.Attacker->GetCombatAttributes()->GetMana(), 8.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHFailedResourceTest, "Lighthaven.Abilities.FailedResource", Flags)
bool FLHFailedResourceTest::RunTest(const FString&)
{
    FFixture F;
    F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(), 1);
    const int32 Seed = F.Attacker->GetCombatRandomState().GetCurrentSeed();
    TestTrue(TEXT("Insufficient resource rejects"), F.Attacker->RequestBasicAttack(F.Defender) == ELHCommandReason::InsufficientMana);
    TestFalse(TEXT("No pending impact"), F.Attacker->ResolveImpact(F.Attacker->GetPendingIdentity()));
    TestEqual(TEXT("Mana unchanged"), F.Attacker->GetCombatAttributes()->GetMana(), 1.f);
    TestEqual(TEXT("Health unchanged"), F.Defender->GetCombatAttributes()->GetHealth(), 100.f);
    TestEqual(TEXT("No cooldown charged"), F.Attacker->GetRemainingCooldown(), 0.0);
    TestEqual(TEXT("No random roll consumed"), F.Attacker->GetCombatRandomState().GetCurrentSeed(), Seed);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDeadTargetTest, "Lighthaven.Abilities.DeadTarget", Flags)
bool FLHDeadTargetTest::RunTest(const FString&)
{
    FFixture F;
    F.Defender->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 0);
    TestTrue(TEXT("Dead target rejects activation"), F.Attacker->RequestBasicAttack(F.Defender) == ELHCommandReason::InvalidLifeState);
    F.Defender->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 100);
    TestTrue(TEXT("Live target activates"), F.Attacker->RequestBasicAttack(F.Defender) == ELHCommandReason::None);
    auto Id = F.Attacker->GetPendingIdentity();
    F.Defender->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 0);
    TestFalse(TEXT("Target dies during windup"), F.Attacker->ResolveImpact(Id));
    TestEqual(TEXT("Dead health unchanged"), F.Defender->GetCombatAttributes()->GetHealth(), 0.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDeathOnceTest, "Lighthaven.Abilities.DeathOnce", Flags)
bool FLHDeathOnceTest::RunTest(const FString&)
{
    FFixture F; int32 Deaths = 0;
    F.Defender->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 10);
    F.Defender->OnDeath.AddLambda([&Deaths](const FLHHitIdentity&) { ++Deaths; });
    TestTrue(TEXT("Activate"), F.Attacker->RequestBasicAttack(F.Defender) == ELHCommandReason::None);
    auto Id = F.Attacker->GetPendingIdentity();
    TestTrue(TEXT("Lethal hit"), F.Attacker->ResolveImpact(Id));
    TestFalse(TEXT("Duplicate lethal callback"), F.Attacker->ResolveImpact(Id));
    TestEqual(TEXT("One lifecycle event"), Deaths, 1);
    TestEqual(TEXT("Health clamps to zero"), F.Defender->GetCombatAttributes()->GetHealth(), 0.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHImpactRangeTest, "Lighthaven.Abilities.ImpactRangeAndCancellation", Flags)
bool FLHImpactRangeTest::RunTest(const FString&)
{
    FFixture F;
    TestTrue(TEXT("Activate"), F.Attacker->RequestBasicAttack(F.Defender) == ELHCommandReason::None);
    auto Id = F.Attacker->GetPendingIdentity();
    F.Target->SetActorLocation(FVector(1000, 0, 0));
    TestFalse(TEXT("Range revalidated"), F.Attacker->ResolveImpact(Id));
    TestEqual(TEXT("No damage outside range"), F.Defender->GetCombatAttributes()->GetHealth(), 100.f);
    F.Attacker->CancelAllAbilities();
    TestFalse(TEXT("Cancelled callback rejected"), F.Attacker->ResolveImpact(Id));
    TestEqual(TEXT("Committed cost retained"), F.Attacker->GetCombatAttributes()->GetMana(), 8.f);
    TestTrue(TEXT("Committed cooldown retained"), F.Attacker->GetRemainingCooldown() > 0);
    return true;
}
// Drive TimerManager, including timers created re-entrantly during publication.
namespace
{
void AdvanceImpactTimer(UWorld* World, float Seconds)
{
    TGuardValue<uint64> Frame(GFrameCounter, GFrameCounter+1);
    World->GetTimerManager().Tick(Seconds);
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHReentrantReplacementTest, "Lighthaven.Abilities.ImpactReplacement", Flags)
bool FLHReentrantReplacementTest::RunTest(const FString&)
{
    FFixture F(true);
    int32 Impacts=0;
    FLHHitIdentity First, Replacement;
    F.Attacker->OnImpact.AddLambda([&](const FLHHitIdentity& Id, const LH::Rules::FCombatResult&)
    {
        ++Impacts;
        if (Impacts != 1) return;
        TestTrue(TEXT("First publication retains original identity"), Id==First);
        F.Attacker->CancelAllAbilities();
        TestTrue(TEXT("Replacement accepted inside listener"), F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
        Replacement=F.Attacker->GetPendingIdentity();
        TestTrue(TEXT("Published identity survives pending overwrite"), Id==First);
    });
    F.World->GetTimerManager().Tick(0.f);
    TestTrue(TEXT("First accepted"), F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
    First=F.Attacker->GetPendingIdentity();
    AdvanceImpactTimer(F.World, 1.1f);
    TestEqual(TEXT("First damage once"), F.Defender->GetCombatAttributes()->GetHealth(), 90.f);
    TestEqual(TEXT("Both committed costs survive"), F.Attacker->GetCombatAttributes()->GetMana(), 6.f);
    TestTrue(TEXT("Replacement pending survives old callback"), F.Attacker->IsActionPending());
    TestTrue(TEXT("Replacement identity survives"), F.Attacker->GetPendingIdentity()==Replacement && !(First==Replacement));
    TestFalse(TEXT("Old duplicate rejected"), F.Attacker->ResolveImpact(First));
    AdvanceImpactTimer(F.World, 0.5f);
    TestEqual(TEXT("Replacement not early"), Impacts, 1);
    AdvanceImpactTimer(F.World, 0.6f);
    TestEqual(TEXT("Replacement timer survives"), Impacts, 2);
    TestEqual(TEXT("Exactly one damage per activation"), F.Defender->GetCombatAttributes()->GetHealth(), 80.f);
    TestFalse(TEXT("Replacement finished"), F.Attacker->IsActionPending());
    TestFalse(TEXT("Replacement duplicate rejected"), F.Attacker->ResolveImpact(Replacement));
    AdvanceImpactTimer(F.World, 2.f);
    TestEqual(TEXT("No extra timer hits"), Impacts, 2);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHReentrantCancelTest, "Lighthaven.Abilities.ImpactCancelWithoutReplacement", Flags)
bool FLHReentrantCancelTest::RunTest(const FString&)
{
    FFixture F(true); int32 Impacts=0;
    F.Attacker->OnImpact.AddLambda([&](const FLHHitIdentity&, const LH::Rules::FCombatResult&)
    {
        ++Impacts; F.Attacker->CancelAllAbilities();
    });
    F.World->GetTimerManager().Tick(0.f);
    TestTrue(TEXT("Accepted"), F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
    const auto Id=F.Attacker->GetPendingIdentity();
    AdvanceImpactTimer(F.World, 1.1f);
    TestFalse(TEXT("Cancelled action cleared"), F.Attacker->IsActionPending());
    TestEqual(TEXT("Cost retained"), F.Attacker->GetCombatAttributes()->GetMana(), 8.f);
    TestFalse(TEXT("Duplicate rejected"), F.Attacker->ResolveImpact(Id));
    AdvanceImpactTimer(F.World, 2.f);
    TestEqual(TEXT("One publication"), Impacts, 1);
    TestEqual(TEXT("One damage"), F.Defender->GetCombatAttributes()->GetHealth(), 90.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDeathReplacementTest, "Lighthaven.Abilities.TargetDeathReplacement", Flags)
bool FLHDeathReplacementTest::RunTest(const FString&)
{
    FFixture F(true); int32 Impacts=0, Deaths=0;
    // A second initialized live target is needed; attacking the dead life must reject.
    FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* Next=F.World->SpawnActor<ALHEnemyCharacter>(FVector(0,100,0), FRotator::ZeroRotator, Spawn);
    auto* NextCombat=Next->GetCombatComponent();
    NextCombat->SetNumericAttributeBase(ULHAttributeSet::GetMaxHealthAttribute(), 100);
    NextCombat->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 100);
    Next->InitializeAfterRestore();
    F.Defender->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 10);
    FLHHitIdentity First, Replacement;
    F.Defender->OnDeath.AddLambda([&](const FLHHitIdentity& Id)
    {
        ++Deaths;
        TestTrue(TEXT("Death identity is first activation"), Id==First);
        F.Attacker->CancelAllAbilities();
        TestTrue(TEXT("Death listener replacement accepted"), F.Attacker->RequestBasicAttack(NextCombat)==ELHCommandReason::None);
        Replacement=F.Attacker->GetPendingIdentity();
        TestTrue(TEXT("Death identity survives replacement"), Id==First);
    });
    F.Attacker->OnImpact.AddLambda([&](const FLHHitIdentity& Id, const LH::Rules::FCombatResult&)
    {
        ++Impacts;
        TestTrue(TEXT("Publication owns correct activation"), Id==(Impacts==1 ? First : Replacement));
    });
    F.World->GetTimerManager().Tick(0.f);
    TestTrue(TEXT("First accepted"), F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
    First=F.Attacker->GetPendingIdentity();
    AdvanceImpactTimer(F.World, 1.1f);
    TestEqual(TEXT("One death"), Deaths, 1);
    TestEqual(TEXT("Lethal first damage"), F.Defender->GetCombatAttributes()->GetHealth(), 0.f);
    TestTrue(TEXT("Replacement pending"), F.Attacker->IsActionPending() && F.Attacker->GetPendingIdentity()==Replacement);
    TestEqual(TEXT("Both costs retained"), F.Attacker->GetCombatAttributes()->GetMana(), 6.f);
    TestEqual(TEXT("Next target untouched before timer"), NextCombat->GetCombatAttributes()->GetHealth(), 100.f);
    AdvanceImpactTimer(F.World, 1.1f);
    TestEqual(TEXT("Both publications"), Impacts, 2);
    TestEqual(TEXT("Next damage once"), NextCombat->GetCombatAttributes()->GetHealth(), 90.f);
    TestFalse(TEXT("Replacement finished"), F.Attacker->IsActionPending());
    TestFalse(TEXT("Old duplicate rejected"), F.Attacker->ResolveImpact(First));
    TestFalse(TEXT("Replacement duplicate rejected"), F.Attacker->ResolveImpact(Replacement));
    AdvanceImpactTimer(F.World, 2.f);
    TestEqual(TEXT("No extra publications"), Impacts, 2);
    NextCombat->ClearCombatAvatar(); Next->Destroy();
    return true;
}
#endif
