#include "Misc/AutomationTest.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Framework/LHEnemyCharacter.h"
#include "Engine/World.h"

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
    FFixture()
    {
        World = UWorld::CreateWorld(EWorldType::Game, false);
        World->InitializeNewWorld(UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true)
            .RequiresHitProxies(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false));
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
        Config.ManaCost = Number(2); Config.CooldownSeconds = Number(3); Config.ImpactSeconds = Number(1);
        Config.RangeCm = Number(200); Config.WeaponMinimum = Number(10); Config.WeaponMaximum = Number(10);
        Attacker->ConfigureAttack(Config, {});
        Attacker->SetCombatRandomState(FRandomStream(123));
    }
    ~FFixture() { Attacker->ClearCombatAvatar(); Defender->ClearCombatAvatar(); World->DestroyWorld(false); }
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
#endif
