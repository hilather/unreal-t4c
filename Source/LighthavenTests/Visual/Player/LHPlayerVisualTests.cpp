#include "Misc/AutomationTest.h"
#include "Visual/Player/LHPlayerVisual.h"
#include "Framework/LHCharacter.h"
#include "Framework/LHEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "ProceduralMeshComponent.h"
#include "CoreGlobals.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
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
    for(const TCHAR* Id:{Body?TEXT("Presentation.Player.Body.B"):TEXT("Presentation.Player.Body.A"),Body?TEXT("Presentation.Player.Face.B"):TEXT("Presentation.Player.Face.A"),Hair?TEXT("Presentation.Player.Hair.Tied"):TEXT("Presentation.Player.Hair.Cropped"),Skin==2?TEXT("Presentation.Player.Skin.DeepWarm"):Skin==1?TEXT("Presentation.Player.Skin.MediumWarm"):TEXT("Presentation.Player.Skin.LightWarm"),TEXT("Presentation.Player.Outfit.StarterLinen")}) { FLHContentId V; V.Value=Id; C.AppearanceIds.Add(V); }
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
    if(V->UsesImportedBody()) for(const auto& Piece:V->Pieces()) if(Piece)
    {
        const FName Id=Piece->GetRecipe().Id;
        const FName Expected=Id==TEXT("Presentation.Player.Weapon")?FName(TEXT("Socket.Bow.L")):Id==TEXT("Presentation.Player.Quiver")?FName(TEXT("Socket.Quiver.Back")):Id==TEXT("Presentation.Player.BowArrow")?FName(TEXT("Socket.Arrow")):NAME_None;
        if(!Expected.IsNone()) TestTrue(TEXT("Equipped geometry attached to named imported socket"),Piece->GetRootComponent()->GetAttachParent()==V->BodyMesh() && Piece->GetRootComponent()->GetAttachSocketName()==Expected);
    }
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
        auto C=Looks(0,0,0); if(!Enabled) C.AppearanceIds.Reset(); V->Present(&C,F.Attacker,0,0);
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
        if(!V->UsesImportedBody()) TestTrue(TEXT("Nock drawn back"),V->Socket(TEXT("BowArrow"))->GetRelativeLocation().X < -15);
        else for(const auto& Piece:V->Pieces()) if(Piece && Piece->GetRecipe().Id==TEXT("Presentation.Player.BowArrow"))
            TestTrue(TEXT("Imported nock follows named arrow socket"),Piece->GetRootComponent()->GetAttachParent()==V->BodyMesh() && Piece->GetRootComponent()->GetAttachSocketName()==TEXT("Socket.Arrow"));
        F.Attacker->OnAttackFinished.Broadcast(E,ELHAttackOutcome::Cancelled);
        E.Ability.Value=TEXT("Spell.FireDart"); F.Attacker->OnAttackCommitted.Broadcast(E); V->Present(&C,F.Attacker,0,.2f); TestTrue(TEXT("Cast event"),V->Pose()==ELHPlayerPose::Cast);
        F.Attacker->OnAttackFinished.Broadcast(E,ELHAttackOutcome::Cancelled);
        F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),80); V->Present(&C,F.Attacker,0,.01f); TestTrue(TEXT("Hit flinch"),V->Pose()==ELHPlayerPose::Hit);
        F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),0); V->Present(&C,F.Attacker,0,.7f); TestTrue(TEXT("Death collapse"),V->Pose()==ELHPlayerPose::Death);
        TestEqual(TEXT("Collapse is skeletal or procedural only"),V->GetRelativeRotation().Pitch,V->UsesImportedBody()?0.0:-90.0);
        F.Attacker->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),100); V->Present(&C,F.Attacker,0,.3f); TestTrue(TEXT("Restore resets death"),V->Pose()==ELHPlayerPose::Idle);
        V->DestroyComponent();
    }
    TestEqual(TEXT("Same target health settlement"),Health[0],Health[1]); TestEqual(TEXT("Same mana cost"),Mana[0],Mana[1]); TestEqual(TEXT("Same impacts"),Impacts[0],Impacts[1]); TestEqual(TEXT("Exactly one impact"),Impacts[1],1); TestEqual(TEXT("Same RNG"),Seeds[0],Seeds[1]); TestEqual(TEXT("Same cooldown"),Cooldown[0],Cooldown[1]); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHPlayerSpawnUprightTest,"Lighthaven.Visual.Player.AliveSpawnAndDetachedAvatarUpright",LHPlayerTestsPrivate::LHPlayerVisualTestsFlags)
bool FLHPlayerSpawnUprightTest::RunTest(const FString&)
{
    using namespace LHPlayerTestsPrivate;
    FFixture F;
    FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* Player=F.World->SpawnActor<ALHCharacter>(FVector(1000,0,90),FRotator::ZeroRotator,Spawn);
    auto* V=Player->PlayerVisual.Get();
    auto C=Looks(0,0,0); C.CurrentHealth=Number(100);
    F.Attacker->ClearCombatAvatar();
    TestFalse(TEXT("Detached GAS avatar is not IsAlive"),F.Attacker->IsAlive());
    V->Present(&C,F.Attacker,0,1);
    TestTrue(TEXT("Alive saved character spawns idle during avatar gap"),V->Pose()==ELHPlayerPose::Idle);
    TestTrue(TEXT("Floor root upright"),V->GetComponentTransform().GetRotation().RotateVector(FVector::UpVector).Equals(FVector::UpVector,1.e-4));
    TestTrue(TEXT("Head above feet"),V->Socket(TEXT("Head"))->GetComponentLocation().Z>V->Socket(TEXT("Leg.L"))->GetComponentLocation().Z);
    TestTrue(TEXT("Visual floor root at capsule base"),FMath::IsNearlyZero(V->GetComponentLocation().Z));
    if(V->BodyMesh())
    {
        TestTrue(TEXT("Imported root upright"),V->BodyMesh()->GetRelativeRotation().IsNearlyZero());
        TestTrue(TEXT("Imported idle head above pelvis"),V->BodyMesh()->GetSocketLocation(TEXT("head")).Z>V->BodyMesh()->GetSocketLocation(TEXT("pelvis")).Z+50);
    }
    C.AppearanceIds.Reset(); V->Present(&C,F.Attacker,0,1);
    TestFalse(TEXT("Unresolved appearance falls back"),V->UsesImportedBody());
    TestTrue(TEXT("Fallback idle upright at spawn"),V->Pose()==ELHPlayerPose::Idle && V->GetRelativeRotation().IsNearlyZero());
    C.CurrentHealth=Number(0); V->Present(&C,F.Attacker,0,1);
    TestTrue(TEXT("Saved dead character still collapses"),V->Pose()==ELHPlayerPose::Death);
    C.CurrentHealth=Number(100); V->Present(&C,F.Attacker,0,0);
    TestTrue(TEXT("Respawn clears collapse without gameplay writes"),V->Pose()==ELHPlayerPose::Idle && V->GetRelativeRotation().IsNearlyZero());
    TestEqual(TEXT("Saved health unchanged"),C.CurrentHealth.Value,100.0);
    Player->Destroy(); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHPlayerImportedAppearanceTest,"Lighthaven.Visual.Player.ImportedAppearancesAndActions",LHPlayerTestsPrivate::LHPlayerVisualTestsFlags)
bool FLHPlayerImportedAppearanceTest::RunTest(const FString&)
{
    using namespace LHPlayerTestsPrivate;
    const FName Expected[]={TEXT("idle"),TEXT("move"),TEXT("run"),TEXT("melee"),TEXT("bow"),TEXT("cast"),TEXT("hit"),TEXT("death")};
    for(int32 A=0;A<8;++A) TestEqual(TEXT("Pose action mapping"),LHPlayerVisual::Action(static_cast<ELHPlayerPose>(A)),Expected[A]);
    FFixture F;
    auto* V=NewObject<ULHPlayerVisualComponent>(F.Source); V->RegisterComponent();
    for(int32 B=0;B<2;++B) for(int32 H=0;H<2;++H) for(int32 S=0;S<3;++S)
    {
        auto C=Looks(B,H,S); int32 Body,Hair,Skin;
        TestTrue(TEXT("Valid appearance resolves"),LHPlayerVisual::ResolveAppearance(C,Body,Hair,Skin));
        TestEqual(TEXT("Body index"),Body,B); TestEqual(TEXT("Hair index"),Hair,H); TestEqual(TEXT("Skin index"),Skin,S);
        V->Present(&C,F.Attacker,0,0);
        if(!TestTrue(TEXT("Imported body and hair available for valid appearance"),V->UsesImportedBody())) continue;
        auto* Mesh=V->BodyMesh()->GetSkeletalMeshAsset();
        const auto Bounds=Mesh->GetBounds();
        TestTrue(TEXT("Rest bounds upright in centimetres"),Bounds.BoxExtent.Z>70 && Bounds.BoxExtent.Z<100 && Bounds.BoxExtent.Z>Bounds.BoxExtent.X*2 && Bounds.Origin.Z>70);
        for(const FName Socket:{FName(TEXT("Socket.Weapon.R")),FName(TEXT("Socket.Bow.L")),FName(TEXT("Socket.Arrow")),FName(TEXT("Socket.Quiver.Back"))}) TestTrue(TEXT("Named equipment socket"),Mesh->FindSocket(Socket)!=nullptr);
        TestTrue(TEXT("Rest weapon shaft faces forward"),V->BodyMesh()->GetSocketTransform(TEXT("Socket.Weapon.R")).GetUnitAxis(EAxis::X).X>.98);
        TestEqual(TEXT("Imported collision off"),V->BodyMesh()->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
        for(int32 A=0;A<8;++A)
        {
            auto* Clip=LoadObject<UAnimSequence>(nullptr,*LHPlayerVisual::ActionAssetPath(C,static_cast<ELHPlayerPose>(A)));
            TestNotNull(TEXT("Required action"),Clip);
            if(Clip) TestTrue(TEXT("Body-specific skeleton"),Clip->GetSkeleton()==Mesh->GetSkeleton());
        }
    }
    auto Invalid=Looks(0,0,0); FLHContentId Face; Face.Value=TEXT("Presentation.Player.Face.B"); Invalid.AppearanceIds.Add(Face);
    int32 B,H,S; TestFalse(TEXT("Mismatched face rejected"),LHPlayerVisual::ResolveAppearance(Invalid,B,H,S));
    V->DestroyComponent(); return true;
}
#endif
