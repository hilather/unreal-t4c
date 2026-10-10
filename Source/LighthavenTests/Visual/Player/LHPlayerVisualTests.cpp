#include "Misc/AutomationTest.h"
#include "Visual/Player/LHPlayerVisual.h"
#include "Framework/LHCharacter.h"
#include "Framework/LHEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "ProceduralMeshComponent.h"
#include "CoreGlobals.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHPlayerTestsPrivate
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
        const FName WorldName = MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("LHPlayerVisualCombatTestWorld"),
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

constexpr auto LHPlayerVisualTestsFlags=EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter;
FLHCharacterRecord Looks(int32 Body,int32 Hair,int32 Skin)
{
    FLHCharacterRecord C;
    for(const TCHAR* Id:{Body?TEXT("Presentation.Player.Body.B"):TEXT("Presentation.Player.Body.A"),Hair?TEXT("Presentation.Player.Hair.Tied"):TEXT("Presentation.Player.Hair.Cropped"),Skin==2?TEXT("Presentation.Player.Skin.DeepWarm"):Skin==1?TEXT("Presentation.Player.Skin.MediumWarm"):TEXT("Presentation.Player.Skin.LightWarm"),TEXT("Presentation.Player.Outfit.StarterLinen")}) { FLHContentId V; V.Value=Id; C.AppearanceIds.Add(V); }
    return C;
}
void Equip(FLHCharacterRecord& C,const TCHAR* Id,ELHEquipmentSlot Slot)
{
    FLHItemInstance I; I.Id.InstanceId=FGuid::NewGuid(); I.Definition.Value=Id; C.Inventory.Add(I);
    FLHEquipmentBinding B; B.Item=I.Id; B.Slot=Slot; C.Equipment.Add(B);
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHPlayerLooksTest,"Lighthaven.Visual.Player.DeterministicAppearance",LHPlayerTestsPrivate::LHPlayerVisualTestsFlags)
bool FLHPlayerLooksTest::RunTest(const FString&)
{
    TSet<uint32> Hashes;
    for(int32 B=0;B<2;++B) for(int32 H=0;H<2;++H) for(int32 S=0;S<3;++S)
    {
        auto C=LHPlayerTestsPrivate::Looks(B,H,S); auto Parts=LHPlayerVisual::Build(C);
        auto Hash=LHPlayerVisual::Fingerprint(Parts); Hashes.Add(Hash);
        TestEqual(TEXT("Deterministic"),Hash,LHPlayerVisual::Fingerprint(LHPlayerVisual::Build(C)));
        for(const auto& P:Parts) TestTrue(TEXT("Recipe collision empty"),P.Recipe.Collision.IsEmpty());
    }
    TestEqual(TEXT("All 12 creation combinations distinct"),Hashes.Num(),12); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHPlayerSocketsTest,"Lighthaven.Visual.Player.SocketsCollisionAndMovement",LHPlayerTestsPrivate::LHPlayerVisualTestsFlags)
bool FLHPlayerSocketsTest::RunTest(const FString&)
{
    LHPlayerTestsPrivate::FFixture F;
    FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* P=F.World->SpawnActor<ALHCharacter>(FVector(1000,0,90),FRotator::ZeroRotator,Spawn);
    auto* V=P->PlayerVisual.Get(); auto C=LHPlayerTestsPrivate::Looks(0,0,0);
    LHPlayerTestsPrivate::Equip(C,TEXT("Item.AshwoodFlatbow"),ELHEquipmentSlot::MainHand);
    LHPlayerTestsPrivate::Equip(C,TEXT("Item.WoodenArrows"),ELHEquipmentSlot::Quiver);
    V->Present(&C,nullptr,0,.1f);
    TestEqual(TEXT("Capsule radius preserved"),P->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),35.f);
    TestEqual(TEXT("Capsule half height preserved"),P->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(),90.f);
    TestTrue(TEXT("Bow in left hand"),V->Socket(TEXT("Weapon"))->GetAttachParent()==V->Socket(TEXT("Socket.Bow.L")));
    TestTrue(TEXT("Quiver on back"),V->Socket(TEXT("Quiver"))->GetAttachParent()==V->Socket(TEXT("Socket.Quiver.Back")));
    for(const auto& Piece:V->Pieces()) { TestTrue(TEXT("No blockers"),Piece->GetBlockers().IsEmpty()); TestEqual(TEXT("Mesh collision off"),Piece->GetMesh()->GetCollisionEnabled(),ECollisionEnabled::NoCollision); }
    TestTrue(TEXT("Idle"),V->Pose()==ELHPlayerPose::Idle);
    V->Present(&C,nullptr,220,.1f); TestTrue(TEXT("Walk"),V->Pose()==ELHPlayerPose::Walk);
    TestTrue(TEXT("Legs move in walk"),!V->Socket(TEXT("Leg.L"))->GetRelativeRotation().IsNearlyZero());
    V->Present(&C,nullptr,450,.1f); TestTrue(TEXT("Run"),V->Pose()==ELHPlayerPose::Run);
    C.Equipment.Reset(); C.Inventory.Reset(); LHPlayerTestsPrivate::Equip(C,TEXT("Item.RustedDirk"),ELHEquipmentSlot::MainHand);
    V->Present(&C,nullptr,0,.1f); TestTrue(TEXT("Dirk right hand after rebuild"),V->Socket(TEXT("Weapon"))->GetAttachParent()==V->Socket(TEXT("Socket.Weapon.R")));
    P->Destroy(); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHPlayerEventsTest,"Lighthaven.Visual.Player.CombatEventsAndSettlementGuard",LHPlayerTestsPrivate::LHPlayerVisualTestsFlags)
bool FLHPlayerEventsTest::RunTest(const FString&)
{
    using namespace LHPlayerTestsPrivate;
    float Health[2],Mana[2]; double Cooldown[2]; int32 Impacts[2]={0,0}; int32 Seeds[2];
    for(int32 Enabled=0;Enabled<2;++Enabled)
    {
        FFixture F;
        auto* V=NewObject<ULHPlayerVisualComponent>(F.Source); V->RegisterComponent(); V->SetPresentationEnabled(Enabled!=0);
        auto C=Looks(0,0,0); V->Present(&C,F.Attacker,0,0);
        F.Attacker->OnImpact.AddLambda([&](const FLHHitIdentity&,const LH::Rules::FCombatResult&){++Impacts[Enabled];});
        TestTrue(TEXT("Attack accepted"),F.Attacker->RequestBasicAttack(F.Defender)==ELHCommandReason::None);
        V->Present(&C,F.Attacker,0,.5f); TestTrue(TEXT("Committed melee"),V->Pose()==ELHPlayerPose::Melee);
        FLHAttackEvent Stale; Stale.Identity.ActivationId=FGuid::NewGuid(); F.Attacker->OnAttackFinished.Broadcast(Stale,ELHAttackOutcome::Cancelled);
        V->Present(&C,F.Attacker,0,0); TestTrue(TEXT("Stale finish cannot clear current swing"),V->Pose()==ELHPlayerPose::Melee);
        TestEqual(TEXT("Presentation cannot resolve early"),F.Defender->GetCombatAttributes()->GetHealth(),100.f);
        const auto Identity=F.Attacker->GetPendingIdentity();
        TestTrue(TEXT("Authoritative impact"),F.Attacker->ResolveImpact(Identity)); F.Attacker->FinishAttack();
        Cooldown[Enabled]=F.Attacker->GetRemainingCooldown(); Health[Enabled]=F.Defender->GetCombatAttributes()->GetHealth(); Mana[Enabled]=F.Attacker->GetCombatAttributes()->GetMana(); Seeds[Enabled]=F.Attacker->GetCombatRandomState().GetCurrentSeed();
        V->Present(&C,F.Attacker,0,.3f); TestTrue(TEXT("Finished back to idle"),V->Pose()==ELHPlayerPose::Idle);
        Equip(C,TEXT("Item.AshwoodFlatbow"),ELHEquipmentSlot::MainHand); Equip(C,TEXT("Item.WoodenArrows"),ELHEquipmentSlot::Quiver); V->Present(&C,F.Attacker,0,0);
        FLHAttackEvent E; E.ImpactSeconds=.7; E.Ability.Value=TEXT("Attack.Ranged.Bow"); F.Attacker->OnAttackCommitted.Broadcast(E);
        V->Present(&C,F.Attacker,0,.2f); TestTrue(TEXT("Bow event"),V->Pose()==ELHPlayerPose::Bow);
        TestTrue(TEXT("Nock drawn back"),V->Socket(TEXT("BowArrow"))->GetRelativeLocation().X < -15);
        F.Attacker->OnAttackFinished.Broadcast(E,ELHAttackOutcome::Cancelled);
        E.Ability.Value=TEXT("Spell.FireDart"); F.Attacker->OnAttackCommitted.Broadcast(E); V->Present(&C,F.Attacker,0,.2f); TestTrue(TEXT("Cast event"),V->Pose()==ELHPlayerPose::Cast);
        F.Attacker->OnAttackFinished.Broadcast(E,ELHAttackOutcome::Cancelled);
        F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),80); V->Present(&C,F.Attacker,0,.01f); TestTrue(TEXT("Hit flinch"),V->Pose()==ELHPlayerPose::Hit);
        F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),0); V->Present(&C,F.Attacker,0,.7f); TestTrue(TEXT("Death collapse"),V->Pose()==ELHPlayerPose::Death);
        TestEqual(TEXT("Collapsed presentation only"),V->GetRelativeRotation().Pitch,-90.0);
        F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),100); V->Present(&C,F.Attacker,0,.3f); TestTrue(TEXT("Restore resets death"),V->Pose()==ELHPlayerPose::Idle);
        V->DestroyComponent();
    }
    TestEqual(TEXT("Same target health settlement"),Health[0],Health[1]); TestEqual(TEXT("Same mana cost"),Mana[0],Mana[1]); TestEqual(TEXT("Same impacts"),Impacts[0],Impacts[1]); TestEqual(TEXT("Exactly one impact"),Impacts[1],1); TestEqual(TEXT("Same RNG"),Seeds[0],Seeds[1]); TestEqual(TEXT("Same cooldown"),Cooldown[0],Cooldown[1]); return true;
}
#endif
