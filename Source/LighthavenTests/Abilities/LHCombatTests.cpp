#include "Misc/AutomationTest.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Framework/LHEnemyCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"
#include "CoreGlobals.h"
#include "Components/BoxComponent.h"
#include "Abilities/LHAbilityCatalog.h"
#include "Abilities/LHResourceRecovery.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace LHCombatTestsPrivate
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
    // Restore the host frame only after all simulated ticks and world cleanup.
    TGuardValue<uint64> FrameCounter{GFrameCounter, GFrameCounter};
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDestroyedTargetTest, "Lighthaven.Abilities.DestroyedTarget", LHCombatTestsPrivate::Flags)
bool FLHDestroyedTargetTest::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F;
    TestTrue(TEXT("Activate GAS attack"), F.Attacker->RequestBasicAttack(F.Defender) == ELHCommandReason::None);
    auto Id = F.Attacker->GetPendingIdentity();
    F.Target->Destroy();
    TestFalse(TEXT("Destroyed avatar rejected at impact"), F.Attacker->ResolveImpact(Id));
    TestEqual(TEXT("No damage"), F.Defender->GetCombatAttributes()->GetHealth(), 100.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDuplicateImpactTest, "Lighthaven.Abilities.DuplicateImpact", LHCombatTestsPrivate::Flags)
bool FLHDuplicateImpactTest::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F;
    TestTrue(TEXT("Activate"), F.Attacker->RequestBasicAttack(F.Defender) == ELHCommandReason::None);
    auto Id = F.Attacker->GetPendingIdentity();
    TestTrue(TEXT("First impact"), F.Attacker->ResolveImpact(Id));
    TestFalse(TEXT("Duplicate rejected"), F.Attacker->ResolveImpact(Id));
    TestEqual(TEXT("Exactly one damage application"), F.Defender->GetCombatAttributes()->GetHealth(), 90.f);
    TestEqual(TEXT("Exactly one cost"), F.Attacker->GetCombatAttributes()->GetMana(), 8.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHFailedResourceTest, "Lighthaven.Abilities.FailedResource", LHCombatTestsPrivate::Flags)
bool FLHFailedResourceTest::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F;
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDeadTargetTest, "Lighthaven.Abilities.DeadTarget", LHCombatTestsPrivate::Flags)
bool FLHDeadTargetTest::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F;
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDeathOnceTest, "Lighthaven.Abilities.DeathOnce", LHCombatTestsPrivate::Flags)
bool FLHDeathOnceTest::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F; int32 Deaths = 0;
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHImpactRangeTest, "Lighthaven.Abilities.ImpactRangeAndCancellation", LHCombatTestsPrivate::Flags)
bool FLHImpactRangeTest::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F;
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
namespace LHCombatTestsPrivate
{
void AdvanceImpactTimer(UWorld* World, float Seconds)
{
    // TimerManager skips a second Tick in the same frame. A per-call guard
    // would restore the counter and reuse that frame on every later advance.
    ++GFrameCounter;
    World->GetTimerManager().Tick(Seconds);
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHReentrantReplacementTest, "Lighthaven.Abilities.ImpactReplacement", LHCombatTestsPrivate::Flags)
bool FLHReentrantReplacementTest::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F(true);
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
    LHCombatTestsPrivate::AdvanceImpactTimer(F.World, 1.1f);
    TestEqual(TEXT("First damage once"), F.Defender->GetCombatAttributes()->GetHealth(), 90.f);
    TestEqual(TEXT("Both committed costs survive"), F.Attacker->GetCombatAttributes()->GetMana(), 6.f);
    TestTrue(TEXT("Replacement pending survives old callback"), F.Attacker->IsActionPending());
    TestTrue(TEXT("Replacement identity survives"), F.Attacker->GetPendingIdentity()==Replacement && !(First==Replacement));
    TestFalse(TEXT("Old duplicate rejected"), F.Attacker->ResolveImpact(First));
    LHCombatTestsPrivate::AdvanceImpactTimer(F.World, 0.5f);
    TestEqual(TEXT("Replacement not early"), Impacts, 1);
    LHCombatTestsPrivate::AdvanceImpactTimer(F.World, 0.6f);
    TestEqual(TEXT("Replacement timer survives"), Impacts, 2);
    TestEqual(TEXT("Exactly one damage per activation"), F.Defender->GetCombatAttributes()->GetHealth(), 80.f);
    TestFalse(TEXT("Replacement finished"), F.Attacker->IsActionPending());
    TestFalse(TEXT("Replacement duplicate rejected"), F.Attacker->ResolveImpact(Replacement));
    LHCombatTestsPrivate::AdvanceImpactTimer(F.World, 2.f);
    TestEqual(TEXT("No extra timer hits"), Impacts, 2);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHReentrantCancelTest, "Lighthaven.Abilities.ImpactCancelWithoutReplacement", LHCombatTestsPrivate::Flags)
bool FLHReentrantCancelTest::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F(true); int32 Impacts=0;
    F.Attacker->OnImpact.AddLambda([&](const FLHHitIdentity&, const LH::Rules::FCombatResult&)
    {
        ++Impacts; F.Attacker->CancelAllAbilities();
    });
    F.World->GetTimerManager().Tick(0.f);
    TestTrue(TEXT("Accepted"), F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
    const auto Id=F.Attacker->GetPendingIdentity();
    LHCombatTestsPrivate::AdvanceImpactTimer(F.World, 1.1f);
    TestFalse(TEXT("Cancelled action cleared"), F.Attacker->IsActionPending());
    TestEqual(TEXT("Cost retained"), F.Attacker->GetCombatAttributes()->GetMana(), 8.f);
    TestFalse(TEXT("Duplicate rejected"), F.Attacker->ResolveImpact(Id));
    LHCombatTestsPrivate::AdvanceImpactTimer(F.World, 2.f);
    TestEqual(TEXT("One publication"), Impacts, 1);
    TestEqual(TEXT("One damage"), F.Defender->GetCombatAttributes()->GetHealth(), 90.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDeathReplacementTest, "Lighthaven.Abilities.TargetDeathReplacement", LHCombatTestsPrivate::Flags)
bool FLHDeathReplacementTest::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F(true); int32 Impacts=0, Deaths=0;
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
    LHCombatTestsPrivate::AdvanceImpactTimer(F.World, 1.1f);
    TestEqual(TEXT("One death"), Deaths, 1);
    TestEqual(TEXT("Lethal first damage"), F.Defender->GetCombatAttributes()->GetHealth(), 0.f);
    TestTrue(TEXT("Replacement pending"), F.Attacker->IsActionPending() && F.Attacker->GetPendingIdentity()==Replacement);
    TestEqual(TEXT("Both costs retained"), F.Attacker->GetCombatAttributes()->GetMana(), 6.f);
    TestEqual(TEXT("Next target untouched before timer"), NextCombat->GetCombatAttributes()->GetHealth(), 100.f);
    LHCombatTestsPrivate::AdvanceImpactTimer(F.World, 1.1f);
    TestEqual(TEXT("Both publications"), Impacts, 2);
    TestEqual(TEXT("Next damage once"), NextCombat->GetCombatAttributes()->GetHealth(), 90.f);
    TestFalse(TEXT("Replacement finished"), F.Attacker->IsActionPending());
    TestFalse(TEXT("Old duplicate rejected"), F.Attacker->ResolveImpact(First));
    TestFalse(TEXT("Replacement duplicate rejected"), F.Attacker->ResolveImpact(Replacement));
    LHCombatTestsPrivate::AdvanceImpactTimer(F.World, 2.f);
    TestEqual(TEXT("No extra publications"), Impacts, 2);
    NextCombat->ClearCombatAvatar(); Next->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCancellationEvents,"Lighthaven.Abilities.CancellationNoRefund",LHCombatTestsPrivate::Flags)
bool FLHCancellationEvents::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F; int Commits=0,Cancels=0,Finishes=0;
    F.Attacker->OnAttackCommitted.AddLambda([&](const FLHAttackEvent&){++Commits;});
    F.Attacker->OnAttackCancelled.AddLambda([&](const FLHAttackEvent&,ELHAttackCancelReason){++Cancels;});
    F.Attacker->OnAttackFinished.AddLambda([&](const FLHAttackEvent&,ELHAttackOutcome){++Finishes;});
    F.Target->SetActorLocation(FVector(1000,0,0));
    TestTrue(TEXT("Precommit free rejection"),F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::OutOfRange);
    TestEqual(TEXT("No commit"),Commits,0); F.Target->SetActorLocation(FVector(100,0,0));
    TestTrue(TEXT("Commit"),F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
    F.Attacker->CancelAttack(ELHAttackCancelReason::Travel); F.Attacker->CancelAllAbilities();
    TestEqual(TEXT("Cost retained"),F.Attacker->GetCombatAttributes()->GetMana(),8.f);
    TestTrue(TEXT("Cooldown retained"),F.Attacker->GetRemainingCooldown()>0);
    TestEqual(TEXT("Commit once"),Commits,1); TestEqual(TEXT("Cancel once"),Cancels,1); TestEqual(TEXT("Finish once"),Finishes,1); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCooldownPersist,"Lighthaven.Abilities.CooldownPersistence",LHCombatTestsPrivate::Flags)
bool FLHCooldownPersist::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F;
    TMap<FName,double> R; R.Add(TEXT("Attack.Melee.Basic"),3); R.Add(TEXT("Spell.FireDart"),1.5);
    TestTrue(TEXT("Restore independent"),F.Attacker->RestoreCooldownMap(R));
    FLHEntityId Owner; Owner.RunId=FGuid::NewGuid(); Owner.InstanceId=FGuid::NewGuid(); Owner.Area.Content.Value=TEXT("Area.Test");
    TArray<FLHCooldownRecord> Saved; LHAbilities::CaptureCooldowns(*F.Attacker,Owner,Saved);
    TestEqual(TEXT("Two records"),Saved.Num(),2); F.Attacker->ClearCombatAvatar(); F.Source->InitializeAfterRestore();
    TestTrue(TEXT("Restore after travel"),LHAbilities::RestoreCooldowns(*F.Attacker,Owner,Saved));
    FLHContentId Id; Id.Value=TEXT("Spell.FireDart"); TestEqual(TEXT("No catchup"),F.Attacker->GetRemainingCooldown(Id),1.5);
    const auto Duplicate=Saved[0]; Saved.Add(Duplicate); TestFalse(TEXT("Duplicate atomic rejection"),LHAbilities::RestoreCooldowns(*F.Attacker,Owner,Saved)); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHSpellAuthority,"Lighthaven.Abilities.SpellKnowledgeAndMana",LHCombatTestsPrivate::Flags)
bool FLHSpellAuthority::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F; LHAbilities::FLHUseAbilityContext C;
    C.Source=F.Attacker; C.Target=F.Defender; C.Combat=LH::Rules::MakeStage1PrototypeCombat();
    C.TargetId.RunId=FGuid::NewGuid(); C.TargetId.InstanceId=FGuid::NewGuid(); C.TargetId.Area.Content.Value=TEXT("Area.Test");
    auto& B=C.Snapshot.Character.BaseAttributes; B.Strength=LHCombatTestsPrivate::Integer(30); B.Endurance=B.Strength; B.Agility=B.Strength; B.Intelligence=B.Strength; B.Wisdom=B.Strength;
    C.Snapshot.Character.EarnedLevel=LHCombatTestsPrivate::Integer(6);
    FLHUseAbilityRequest R; R.Ability.Value=TEXT("Spell.FireDart"); R.Target=C.TargetId;
    TestTrue(TEXT("Unlearned"),LHAbilities::ExecuteUseAbility(C,R)==ELHCommandReason::Ineligible);
    C.Snapshot.Character.LearnedSpells.Add(R.Ability); F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(),0);
    TestTrue(TEXT("No mana"),LHAbilities::ExecuteUseAbility(C,R)==ELHCommandReason::InsufficientMana);
    TestEqual(TEXT("Rejected mana unchanged"),F.Attacker->GetCombatAttributes()->GetMana(),0.f);
    F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(),10);
    TestTrue(TEXT("Cast"),LHAbilities::ExecuteUseAbility(C,R)==ELHCommandReason::None);
    TestTrue(TEXT("Measured range prototype output"),F.Defender->GetCombatAttributes()->GetHealth()<=92 && F.Defender->GetCombatAttributes()->GetHealth()>=77);
    R.Ability.Value=TEXT("Spell.HealLight"); C.Snapshot.Character.LearnedSpells.Add(R.Ability); C.bTargetFriendly=true;
    F.Defender->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),99);
    TestTrue(TEXT("Heal independent cooldown"),LHAbilities::ExecuteUseAbility(C,R)==ELHCommandReason::None);
    TestEqual(TEXT("Heal clamps"),F.Defender->GetCombatAttributes()->GetHealth(),100.f); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHBowQuiver,"Lighthaven.Abilities.BowRequiresQuiver",LHCombatTestsPrivate::Flags)
bool FLHBowQuiver::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F; LHAbilities::FLHUseAbilityContext C; C.Source=F.Attacker; C.Target=F.Defender; C.Combat=LH::Rules::MakeStage1PrototypeCombat();
    C.TargetId.RunId=FGuid::NewGuid(); C.TargetId.InstanceId=FGuid::NewGuid(); C.TargetId.Area.Content.Value=TEXT("Area.Test");
    auto& B=C.Snapshot.Character.BaseAttributes; B.Strength=LHCombatTestsPrivate::Integer(20); B.Endurance=B.Strength; B.Agility=B.Strength; B.Intelligence=B.Strength; B.Wisdom=B.Strength; C.Snapshot.Character.EarnedLevel=LHCombatTestsPrivate::Integer(1);
    FLHItemInstance Bow; Bow.Id=C.TargetId; Bow.Id.InstanceId=FGuid::NewGuid(); Bow.Definition.Value=TEXT("Item.AshwoodFlatbow"); Bow.Quantity=LHCombatTestsPrivate::Integer(1); C.Snapshot.Character.Inventory.Add(Bow);
    FLHEquipmentBinding Binding; Binding.Slot=ELHEquipmentSlot::MainHand; Binding.Item=Bow.Id; C.Snapshot.Character.Equipment.Add(Binding);
    FLHCombatItemData BowData,QuiverData; BowData.bWeapon=true; BowData.Weapon.bBow=true; BowData.Weapon.MinimumDamage=LHCombatTestsPrivate::Number(1); BowData.Weapon.MaximumDamage=LHCombatTestsPrivate::Number(3); BowData.Weapon.RangeCm=LHCombatTestsPrivate::Number(1200); FLHContentId QId; QId.Value=TEXT("Item.WoodenArrows"); BowData.Weapon.CompatibleQuivers.Add(QId);
    QuiverData.bQuiver=true; QuiverData.Weapon.QuiverDamageBonus=LHCombatTestsPrivate::Number(1);
    C.ItemLookup=[&](const FLHContentId& I)->const FLHCombatItemData* {return I.Value==Bow.Definition.Value?&BowData:I.Value==QId.Value?&QuiverData:nullptr;};
    FLHUseAbilityRequest R; R.Ability.Value=TEXT("Attack.Ranged.Bow"); R.Target=C.TargetId;
    TestTrue(TEXT("Missing quiver"),LHAbilities::ExecuteUseAbility(C,R)==ELHCommandReason::Ineligible);
    FLHItemInstance Q=Bow; Q.Id.InstanceId=FGuid::NewGuid(); Q.Definition=QId; C.Snapshot.Character.Inventory.Add(Q); Binding.Slot=ELHEquipmentSlot::Quiver; Binding.Item=Q.Id; C.Snapshot.Character.Equipment.Add(Binding);
    FLHLearnedSkill Skill; Skill.Skill.Value=TEXT("Skill.Archery"); Skill.TrainedValue=LHCombatTestsPrivate::Integer(100); C.Snapshot.Character.LearnedSkills.Add(Skill);
    for(int Shot=0;Shot<3;++Shot) { TestTrue(TEXT("With quiver"),LHAbilities::ExecuteUseAbility(C,R)==ELHCommandReason::None); TestTrue(TEXT("Reset fixture cooldown"),F.Attacker->RestoreRemainingCooldown(0)); }
    TestEqual(TEXT("Unlimited arrows unchanged"),C.Snapshot.Character.Inventory[1].Quantity.Value,int64(1)); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHMeleeSight,"Lighthaven.Abilities.MeleeRangeAndLOS",LHCombatTestsPrivate::Flags)
bool FLHMeleeSight::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F; F.Target->SetActorLocation(FVector(1000,0,0));
    TestTrue(TEXT("Out of range"),F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::OutOfRange);
    F.Target->SetActorLocation(FVector(100,0,0));
    auto* Wall=F.World->SpawnActor<AActor>(); auto* Box=NewObject<UBoxComponent>(Wall); Wall->SetRootComponent(Box); Box->SetBoxExtent(FVector(5,100,100)); Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly); Box->SetCollisionResponseToAllChannels(ECR_Ignore); Box->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block); Box->RegisterComponent(); Wall->SetActorLocation(FVector(50,0,0)); Box->RecreatePhysicsState();
    TestTrue(TEXT("Wall rejection"),F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::OutOfRange);
    TestEqual(TEXT("Free rejections"),F.Attacker->GetCombatAttributes()->GetMana(),10.f);
    Box->SetCollisionEnabled(ECollisionEnabled::NoCollision); Wall->Destroy();
    TestTrue(TEXT("Legal melee"),F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
    TestTrue(TEXT("Legal impact"),F.Attacker->ResolveImpact(F.Attacker->GetPendingIdentity())); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHZeroManaRecovery,"Lighthaven.Abilities.ZeroManaNoGoldRecovery",LHCombatTestsPrivate::Flags)
bool FLHZeroManaRecovery::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F; F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(),0);
    auto P=LH::Rules::MakeLedgerPrototypeRuleset().Mana;
    TestTrue(TEXT("Menu time ignored"),F.Attacker->AdvanceManaRegen(100,true,P)); TestEqual(TEXT("Still empty"),F.Attacker->GetCombatAttributes()->GetMana(),0.f);
    TestTrue(TEXT("Natural recovery"),F.Attacker->AdvanceManaRegen(10,false,P)); TestEqual(TEXT("Cast cost recovered without gold or reload"),F.Attacker->GetCombatAttributes()->GetMana(),2.f);
    LHAbilities::FLHUseAbilityContext C; C.Source=F.Attacker; C.Target=F.Defender;
    C.TargetId.RunId=FGuid::NewGuid(); C.TargetId.InstanceId=FGuid::NewGuid(); C.TargetId.Area.Content.Value=TEXT("Area.Test");
    auto& B=C.Snapshot.Character.BaseAttributes; B.Strength=LHCombatTestsPrivate::Integer(21); B.Endurance=B.Strength; B.Agility=B.Strength; B.Intelligence=B.Strength; B.Wisdom=B.Strength;
    C.Snapshot.Character.Gold=LHCombatTestsPrivate::Integer(0); C.Snapshot.Character.EarnedLevel=LHCombatTestsPrivate::Integer(2);
    FLHUseAbilityRequest R; R.Ability.Value=TEXT("Spell.FireDart"); R.Target=C.TargetId; C.Snapshot.Character.LearnedSpells.Add(R.Ability);
    TestTrue(TEXT("Legal Fire Dart after natural recovery"),LHAbilities::ExecuteUseAbility(C,R)==ELHCommandReason::None); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCommitReplacement,"Lighthaven.Abilities.CommitReplacement",LHCombatTestsPrivate::Flags)
bool FLHCommitReplacement::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F(true); int Commits=0,Finishes=0; FLHHitIdentity Replacement;
    F.Attacker->OnAttackFinished.AddLambda([&](const FLHAttackEvent&,ELHAttackOutcome){++Finishes; TestTrue(TEXT("Finish stack blocks snapshots"),F.Attacker->IsPublishingActionEvents());});
    F.Attacker->OnAttackCommitted.AddLambda([&](const FLHAttackEvent& E){
        ++Commits; if(Commits!=1) return; const auto First=E.Identity;
        F.Attacker->CancelAllAbilities();
        TestTrue(TEXT("Replacement inside commit"),F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
        Replacement=F.Attacker->GetPendingIdentity(); TestTrue(TEXT("Captured commit identity stable"),E.Identity==First);
    });
    F.World->GetTimerManager().Tick(0);
    TestTrue(TEXT("Original request accepted"),F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
    TestEqual(TEXT("Both commits"),Commits,2); TestEqual(TEXT("Original finish once"),Finishes,1);
    TestTrue(TEXT("Replacement remains pending"),F.Attacker->IsActionPending() && F.Attacker->GetPendingIdentity()==Replacement);
    LHCombatTestsPrivate::AdvanceImpactTimer(F.World,1.1f);
    TestEqual(TEXT("Only replacement impact"),F.Defender->GetCombatAttributes()->GetHealth(),90.f);
    TestEqual(TEXT("Both finish once"),Finishes,2); TestEqual(TEXT("Both costs retained"),F.Attacker->GetCombatAttributes()->GetMana(),6.f); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCostListenerCancel,"Lighthaven.Abilities.CostListenerCancellation",LHCombatTestsPrivate::Flags)
bool FLHCostListenerCancel::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F(true); int Commits=0,Cancels=0,Finishes=0;
    F.Attacker->GetGameplayAttributeValueChangeDelegate(ULHAttributeSet::GetManaAttribute()).AddLambda([&](const FOnAttributeChangeData&){F.Attacker->CancelAllAbilities();});
    F.Attacker->OnAttackCommitted.AddLambda([&](const FLHAttackEvent&){++Commits; TestEqual(TEXT("No finish before commit event"),Finishes,0);});
    F.Attacker->OnAttackCancelled.AddLambda([&](const FLHAttackEvent&,ELHAttackCancelReason){++Cancels;});
    F.Attacker->OnAttackFinished.AddLambda([&](const FLHAttackEvent&,ELHAttackOutcome O){++Finishes; TestTrue(TEXT("Cancelled outcome"),O==ELHAttackOutcome::Cancelled);});
    TestTrue(TEXT("Committed request"),F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
    TestEqual(TEXT("Cost remains"),F.Attacker->GetCombatAttributes()->GetMana(),8.f); TestEqual(TEXT("Commit once"),Commits,1); TestEqual(TEXT("Cancel once"),Cancels,1); TestEqual(TEXT("Finish once"),Finishes,1);
    TestFalse(TEXT("No pending timer"),F.Attacker->IsActionPending()); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHLightTransient,"Lighthaven.Abilities.LightTransient",LHCombatTestsPrivate::Flags)
bool FLHLightTransient::RunTest(const FString&)
{
    LHCombatTestsPrivate::FFixture F; LHAbilities::FLHUseAbilityContext C; C.Source=F.Attacker; C.Target=F.Attacker;
    C.TargetId.RunId=FGuid::NewGuid(); C.TargetId.InstanceId=FGuid::NewGuid(); C.TargetId.Area.Content.Value=TEXT("Area.Test"); C.SourceId=C.TargetId;
    auto& B=C.Snapshot.Character.BaseAttributes; B.Strength=LHCombatTestsPrivate::Integer(21); B.Endurance=B.Strength; B.Agility=B.Strength; B.Intelligence=B.Strength; B.Wisdom=B.Strength; C.Snapshot.Character.EarnedLevel=LHCombatTestsPrivate::Integer(2);
    FLHUseAbilityRequest R; R.Ability.Value=TEXT("Spell.Light"); R.Target=C.TargetId; C.Snapshot.Character.LearnedSpells.Add(R.Ability);
    TestTrue(TEXT("Caster light"),LHAbilities::ExecuteUseAbility(C,R)==ELHCommandReason::None); TestEqual(TEXT("Sourced duration"),F.Attacker->GetLightRemainingSeconds(),600.0); TestEqual(TEXT("No health effect"),F.Attacker->GetCombatAttributes()->GetHealth(),100.f); TestEqual(TEXT("Sourced cost"),F.Attacker->GetCombatAttributes()->GetMana(),0.f);
    F.Attacker->ClearCombatAvatar(); TestEqual(TEXT("Travel detachment preserves Light"),F.Attacker->GetLightRemainingSeconds(),600.0); return true;
}
#endif
