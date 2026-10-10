#include "Misc/AutomationTest.h"
#include "Visual/Monsters/LHMonsterVisual.h"
#include "Framework/LHEnemyCharacter.h"
#include "AI/LHEnemyAIController.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Abilities/LHAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Visual/LHVisualKit.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "Persistence/LHSaveCodec.h"
#include "Framework/LHWave2Profile.h"
#include "ProceduralMeshComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHMonsterTestsPrivate {
constexpr auto Flags=EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter;
struct FWorld {
    UWorld* W;
    FWorld() {
        auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(true).ShouldSimulatePhysics(false);
        W=UWorld::CreateWorld(EWorldType::Game,false,MakeUniqueObjectName(GetTransientPackage(),UWorld::StaticClass(),TEXT("MonsterTest")),GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
        GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(W); W->InitializeActorsForPlay(FURL());
    }
    ~FWorld() { W->DestroyWorld(false); GEngine->DestroyWorldContext(W); }
    ALHEnemyCharacter* Spawn(const FLHEnemyCatalogRow& R,FVector Pos=FVector::ZeroVector, const FLHSpawnLifeId* SavedLife=nullptr) {
        FActorSpawnParameters P; P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        auto* E=W->SpawnActor<ALHEnemyCharacter>(Pos,FRotator::ZeroRotator,P);
        FLHSpawnLifeId Life; Life.SpawnSlot=FGuid(1,2,3,4); Life.Area.Content.Value=TEXT("Area.MonsterTest"); Life.LifeGeneration=1;
        if(SavedLife) Life=*SavedLife;
        FString Error; if(!E->ApplyRuntimeSpec(R.Runtime,Life,R.Health,Error)) { E->Destroy(); return nullptr; } return E;
    }
};
bool SameScenePose(const FTransform& A,const FTransform& B) {
    // Actor attachment/rotator cache round-trips are not bit-identical across world positions.
    // Explicit recipe fields and generated vertices/indices remain exact below.
    return A.Equals(B,1.e-6);
}
struct FSettlement {
    FLHCharacterProfile Profile=LHWave2::PrototypeProfile(); FLHSaveSnapshot S; FLHKillFacts Facts;
    bool Init(const FLHEnemyCatalogRow& Row) {
        FLHCharacterAuthority A; if(!A.Initialize(Profile,FGuid(1,2,3,4),42)) return false;
        FLHCharacterPreview V; if(A.Preview(LHWave2::PrototypeAnswers(),V)!=ELHCommandReason::None) return false;
        FLHCreateCharacterRequest R; R.Request.Epoch=FGuid(1,2,3,4); R.Request.Value=FGuid(5,6,7,8); R.DisplayName=TEXT("Monster fixture"); R.Creation=V.Creation; R.PreviewToken=V.Token;
        for(const TCHAR* Name:{TEXT("Presentation.Player.Body.A"),TEXT("Presentation.Player.Hair.Cropped"),TEXT("Presentation.Player.Skin.LightWarm"),TEXT("Presentation.Player.Outfit.StarterLinen")}) { FLHContentId Id; Id.Value=Name; R.AppearanceIds.Add(Id); }
        if(A.Execute(R).Disposition!=ELHCommandDisposition::Accepted) return false;
        A.Export(S); S.Header.BuildId=TEXT("W5MonsterFixture"); S.Header.ContentRevision=LHWave2::CatalogHash(); S.Header.ChecksumAlgorithm=TEXT("SHA256"); S.Header.PayloadCodec=TEXT("LHCanonicalBinary1");
        S.Character.ActiveEntrance=LHWorld::Registry()[1].SafeFallback;
        S.Session.SafeRespawn.Entrance=LHWorld::Registry()[0].SafeFallback; S.Session.SafeRespawn.TransformResolution=ELHValueResolution::Resolved; S.Session.SafeRespawn.SafeTransform=LHWorld::FindEntrance(S.Session.SafeRespawn.Entrance)->SafeTransform;
        S.Session.ManaRegenFractionalSeconds=LHWave2::PrototypeNumber(0); S.Session.EffectPolicy=ELHEffectSavePolicy::CompletedActionBoundaryOnly;
        FLHAreaRecord Hub; Hub.Area=LHWorld::Registry()[0].Id; S.World.Areas.Add(Hub);
        FLHAreaRecord B1; if(LHRewards::PopulateArea(B1,LHWorld::Registry()[1],[](const auto&) { return LHWave2::PrototypeNumber(25); })!=ELHCommandReason::None) return false;
        auto& E=B1.Encounters[0]; E.Definition=Row.Id; E.CurrentHealth=Row.Health;
        Facts={E.Life.Area,E.Life,Row.Id,true}; S.World.Areas.Add(B1);
        FLHRngState Rng; Rng.StreamId=TEXT("RNG.Loot"); Rng.Algorithm=TEXT("UE.FRandomStream"); Rng.AlgorithmRevision=1; Rng.State={42,0,0,0}; S.Session.GameplayRng.Add(Rng);
        FLHSaveError Error; return LHSave::Validate(S,Error);
    }
};

}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHMonsterCatalogTest,"Lighthaven.Visual.Monsters.CatalogDeterminism",LHMonsterTestsPrivate::Flags)
bool FLHMonsterCatalogTest::RunTest(const FString&) {
    LHMonsterTestsPrivate::FWorld F; TSet<uint32> Hashes;
    TestEqual(TEXT("Eleven roster species"),LHEnemyData::Catalog().Num(),11);
    for(const auto& R:LHEnemyData::Catalog()) {
        TestTrue(*R.Id.Value.ToString(),ULHMonsterVisual::Known(R.Id.Value));
        auto* A=F.Spawn(R); auto* B=F.Spawn(R,FVector(1000,0,0));
        if(!TestNotNull(TEXT("A"),A)||!TestNotNull(TEXT("B"),B)) continue;
        auto* X=A->GetMonsterVisual(); auto* Y=B->GetMonsterVisual();
        TestFalse(TEXT("Never silent placeholder"),X->ComponentHasTag(TEXT("LH.Monster.UnknownPlaceholder")));
        TestTrue(TEXT("Intentional multi-part body"),X->GetParts().Num()>2);
        TestEqual(TEXT("Part count"),X->GetParts().Num(),Y->GetParts().Num());
        const uint32 Hash=ULHMonsterVisual::RecipeFingerprint(R.Id.Value); TestFalse(TEXT("Distinct recipe"),Hashes.Contains(Hash)); Hashes.Add(Hash);
        TestEqual(TEXT("Checksum repeat"),Hash,ULHMonsterVisual::RecipeFingerprint(R.Id.Value));
        for(int32 I=0;I<X->GetParts().Num();++I) {
            auto* P=X->GetParts()[I].Get(); auto* Q=Y->GetParts()[I].Get();
            TestTrue(*FString::Printf(TEXT("Rest pose %s part %d (1e-6 tolerance)"),*R.Id.Value.ToString(),I),LHMonsterTestsPrivate::SameScenePose(P->GetRelativeTransform(),Q->GetRelativeTransform()));
            if(auto* Kit=Cast<ALHVisualPiece>(P->GetOwner())) {
                auto* Kit2=Cast<ALHVisualPiece>(Q->GetOwner()); TestNotNull(TEXT("Kit counterpart"),Kit2);
                if(Kit2) {
                    TestEqual(TEXT("Kit checksum"),Kit->GetRecipe().Fingerprint(),Kit2->GetRecipe().Fingerprint());
                    const auto* V=Kit->GetMesh()->GetProcMeshSection(0); const auto* V2=Kit2->GetMesh()->GetProcMeshSection(0);
                    TestTrue(TEXT("Procedural indices identical"),V->ProcIndexBuffer==V2->ProcIndexBuffer);
                    for(int32 J=0;J<V->ProcVertexBuffer.Num();++J) TestTrue(TEXT("Procedural vertex identical"),V->ProcVertexBuffer[J].Position==V2->ProcVertexBuffer[J].Position);
                }
            }
        }
        X->AdvancePresentation(.25f,true); Y->AdvancePresentation(.25f,true);
        for(int32 I=0;I<X->GetParts().Num();++I) TestTrue(*FString::Printf(TEXT("Animated pose %s part %d (1e-6 tolerance)"),*R.Id.Value.ToString(),I),LHMonsterTestsPrivate::SameScenePose(X->GetParts()[I]->GetRelativeTransform(),Y->GetParts()[I]->GetRelativeTransform()));
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHMonsterCollisionTest,"Lighthaven.Visual.Monsters.CollisionAndFallback",LHMonsterTestsPrivate::Flags)
bool FLHMonsterCollisionTest::RunTest(const FString&) {
    LHMonsterTestsPrivate::FWorld F;
    for(const auto& R:LHEnemyData::Catalog()) {
        auto* E=F.Spawn(R); if(!TestNotNull(TEXT("Enemy"),E)) continue;
        auto* C=E->GetCapsuleComponent(); const auto Responses=C->GetCollisionResponseToChannels();
        const auto Enabled=C->GetCollisionEnabled(); const bool Nav=C->CanEverAffectNavigation();
        auto* V=E->GetMonsterVisual(); const int32 Count=V->GetParts().Num();
        TestTrue(TEXT("Rebuild"),V->Build(R.Id.Value,R.Runtime.CapsuleRadiusCm.Value,R.Runtime.CapsuleHalfHeightCm.Value));
        TestEqual(TEXT("No accumulated pieces"),Count,V->GetParts().Num());
        V->Attack(1); V->AdvancePresentation(.5,true); V->Hit(); V->AdvancePresentation(.1,false); V->Die(); V->AdvancePresentation(1,false);
        TestEqual(TEXT("Radius unchanged"),double(C->GetUnscaledCapsuleRadius()),R.Runtime.CapsuleRadiusCm.Value);
        TestEqual(TEXT("Height unchanged"),double(C->GetUnscaledCapsuleHalfHeight()),R.Runtime.CapsuleHalfHeightCm.Value);
        TestTrue(TEXT("Responses unchanged"),C->GetCollisionResponseToChannels()==Responses);
        TestTrue(TEXT("Enabled unchanged"),C->GetCollisionEnabled()==Enabled); TestEqual(TEXT("Navigation unchanged"),C->CanEverAffectNavigation(),Nav);
        for(const auto& Part:V->GetParts()) if(auto* P=Cast<UPrimitiveComponent>(Part.Get())) {
            TestTrue(TEXT("Body never collides"),P->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
            TestFalse(TEXT("Body never navigates"),P->CanEverAffectNavigation()); TestFalse(TEXT("No overlaps"),P->GetGenerateOverlapEvents());
        }
    }
    auto* E=F.Spawn(LHEnemyData::Catalog()[0]);
    AddExpectedError(TEXT("UNKNOWN MONSTER Enemy.Unregistered"),EAutomationExpectedErrorFlags::Contains,2);
    TestFalse(TEXT("Unknown returns explicit failure"),E->GetMonsterVisual()->Build(TEXT("Enemy.Unregistered"),25,25));
    TestTrue(TEXT("Named placeholder"),E->GetMonsterVisual()->ComponentHasTag(TEXT("LH.Monster.UnknownPlaceholder")));
    TestEqual(TEXT("Visible unknown cube"),E->GetMonsterVisual()->GetParts().Num(),1);
    auto Unknown=LHEnemyData::Catalog()[0]; Unknown.Runtime.ContentId.Value=TEXT("Enemy.Unregistered");
    auto* Fallback=F.Spawn(Unknown); TestNotNull(TEXT("Character accepts valid runtime with unknown definition"),Fallback);
    if(Fallback) TestTrue(TEXT("Character definition hook logs/marks placeholder"),Fallback->GetMonsterVisual()->ComponentHasTag(TEXT("LH.Monster.UnknownPlaceholder")));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHMonsterEventsTest,"Lighthaven.Visual.Monsters.StateAndCombatEvents",LHMonsterTestsPrivate::Flags)
bool FLHMonsterEventsTest::RunTest(const FString&) {
    LHMonsterTestsPrivate::FWorld F;
    for(const auto& R:LHEnemyData::Catalog()) {
        auto* E=F.Spawn(R); if(!TestNotNull(TEXT("Enemy"),E)) continue;
        auto* V=E->GetMonsterVisual(); auto* C=E->GetCombatComponent(); FLHAttackEvent Event; Event.ImpactSeconds=.7;
        V->AdvancePresentation(.1,false); TestTrue(TEXT("Idle"),V->GetMotion()==ELHMonsterMotion::Idle);
        V->AdvancePresentation(.1,true); TestTrue(TEXT("Move"),V->GetMotion()==ELHMonsterMotion::Move);
        C->OnAttackCommitted.Broadcast(Event); V->TickComponent(.6,LEVELTICK_All,nullptr); TestTrue(TEXT("Late-frame commit does not consume frame delta"),V->GetMotion()==ELHMonsterMotion::Telegraph); V->AdvancePresentation(.69,false); TestTrue(TEXT("Before actual impact delay"),V->GetMotion()==ELHMonsterMotion::Telegraph);
        V->AdvancePresentation(.02,false); TestTrue(TEXT("Strike at actual delay"),V->GetMotion()==ELHMonsterMotion::Strike);
        C->OnAttackCancelled.Broadcast(Event,ELHAttackCancelReason::Explicit); V->AdvancePresentation(0,false); TestTrue(TEXT("Cancel clears attack"),V->GetMotion()==ELHMonsterMotion::Idle);
        C->OnAttackCommitted.Broadcast(Event); C->OnAttackFinished.Broadcast(Event,ELHAttackOutcome::ResolvedMiss); V->AdvancePresentation(0,false); TestTrue(TEXT("Miss still swings"),V->GetMotion()==ELHMonsterMotion::Strike);
        V->AdvancePresentation(.2,false); C->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),R.Health.Value-1); V->AdvancePresentation(0,false); TestTrue(TEXT("Damage hit reaction"),V->GetMotion()==ELHMonsterMotion::Hit);
        V->AdvancePresentation(.3,false); C->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),R.Health.Value); V->AdvancePresentation(0,false); TestTrue(TEXT("Healing never hit reacts"),V->GetMotion()==ELHMonsterMotion::Idle);
        auto* AI=F.W->SpawnActor<ALHEnemyAIController>(); AI->Possess(E); AI->EnterDead();
        V->TickComponent(.1,LEVELTICK_All,nullptr); TestTrue(TEXT("AI dead read"),V->GetMotion()==ELHMonsterMotion::Dead);
        E->MarkCorpse(); C->OnAttackCommitted.Broadcast(Event); V->AdvancePresentation(1,true); TestTrue(TEXT("Death latches"),V->GetMotion()==ELHMonsterMotion::Dead);
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHMonsterSettlementTest,"Lighthaven.Visual.Monsters.PresentationInvariantSettlement",LHMonsterTestsPrivate::Flags)
bool FLHMonsterSettlementTest::RunTest(const FString&) {
    for(const auto& R:LHEnemyData::Catalog()) {
        LHMonsterTestsPrivate::FWorld F;
        LHMonsterTestsPrivate::FSettlement Saved;
        if(!TestTrue(TEXT("Canonical reward fixture"),Saved.Init(R))) continue;
        auto OnSave=Saved.S,OffSave=Saved.S;
        auto* A=F.Spawn(R,FVector(0,0,0)); auto* B=F.Spawn(R,FVector(100,0,0),&Saved.Facts.Life);
        auto* X=F.Spawn(R,FVector(1000,0,0)); auto* Y=F.Spawn(R,FVector(1100,0,0),&Saved.Facts.Life);
        if(!A||!B||!X||!Y) { AddError(TEXT("Fixture spawn")); continue; }
        X->GetMonsterVisual()->SetPresentationEnabled(false); Y->GetMonsterVisual()->SetPresentationEnabled(false);
        // Exercise actual GAS commit/impact/death and single-settlement observer with equal RNG.
        for(auto* E:{A,X}) {
            auto Attack=R.Runtime.Attack; Attack.Combat.HitBase.Value=Attack.Combat.MinimumChance.Value=Attack.Combat.MaximumChance.Value=1;
            Attack.WeaponMinimum.Value=Attack.WeaponMaximum.Value=R.Health.Value+100;
            E->GetCombatComponent()->ConfigureAttack(Attack,R.Runtime.Requirements); E->GetCombatComponent()->SetCombatRandomState(FRandomStream(42));
        }
        int32 OnCount=0,OffCount=0;
        ELHCommandReason OnReason=ELHCommandReason::InvalidRequest,OffReason=ELHCommandReason::InvalidRequest;
        B->GetCombatComponent()->OnDeath.AddLambda([&](const FLHHitIdentity&){++OnCount; OnReason=LHRewards::SettleKill(OnSave,Saved.Facts,R.Reward,Saved.Profile,{});});
        Y->GetCombatComponent()->OnDeath.AddLambda([&](const FLHHitIdentity&){++OffCount; OffReason=LHRewards::SettleKill(OffSave,Saved.Facts,R.Reward,Saved.Profile,{});});
        TestTrue(TEXT("Enabled commit"),A->GetCombatComponent()->RequestBasicAttack(B->GetCombatComponent())==ELHCommandReason::None);
        TestTrue(TEXT("Disabled commit"),X->GetCombatComponent()->RequestBasicAttack(Y->GetCombatComponent())==ELHCommandReason::None);
        A->GetMonsterVisual()->AdvancePresentation(.5,true); X->GetMonsterVisual()->AdvancePresentation(.5,true);
        const auto OnId=A->GetCombatComponent()->GetPendingIdentity(), OffId=X->GetCombatComponent()->GetPendingIdentity();
        TestTrue(TEXT("Enabled impact"),A->GetCombatComponent()->ResolveImpact(OnId)); TestTrue(TEXT("Disabled impact"),X->GetCombatComponent()->ResolveImpact(OffId));
        TestFalse(TEXT("Enabled duplicate rejected"),A->GetCombatComponent()->ResolveImpact(OnId)); TestFalse(TEXT("Disabled duplicate rejected"),X->GetCombatComponent()->ResolveImpact(OffId));
        TestEqual(TEXT("Exactly one enabled death settlement event"),OnCount,1); TestEqual(TEXT("Equal disabled settlement events"),OffCount,OnCount);
        TestEqual(TEXT("Health"),B->GetCombatComponent()->GetCombatAttributes()->GetHealth(),Y->GetCombatComponent()->GetCombatAttributes()->GetHealth());
        TestEqual(TEXT("Mana"),A->GetCombatComponent()->GetCombatAttributes()->GetMana(),X->GetCombatComponent()->GetCombatAttributes()->GetMana());
        TestEqual(TEXT("Random state"),A->GetCombatComponent()->GetCombatRandomState().GetCurrentSeed(),X->GetCombatComponent()->GetCombatRandomState().GetCurrentSeed());
        TestEqual(TEXT("Cooldown"),A->GetCombatComponent()->GetRemainingCooldown(),X->GetCombatComponent()->GetRemainingCooldown());
        TestEqual(TEXT("Corpse lifecycle"),B->IsCorpse(),Y->IsCorpse());
        TestTrue(TEXT("Enabled reward settlement"),OnReason==ELHCommandReason::None); TestTrue(TEXT("Disabled reward settlement"),OffReason==ELHCommandReason::None);
        TArray<uint8> OnBytes,OffBytes; FLHSaveError Error;
        TestTrue(TEXT("Encode enabled settlement"),LHSave::Encode(OnSave,OnBytes,Error)); TestTrue(TEXT("Encode disabled settlement"),LHSave::Encode(OffSave,OffBytes,Error));
        TestTrue(TEXT("XP loot gold RNG lifecycle and boss claims byte-identical"),OnBytes==OffBytes);
        TestTrue(TEXT("Duplicate domain settlement refused"),LHRewards::SettleKill(OnSave,Saved.Facts,R.Reward,Saved.Profile,{})==ELHCommandReason::InvalidLifeState);
        TArray<uint8> AfterDuplicate; TestTrue(TEXT("Encode after duplicate"),LHSave::Encode(OnSave,AfterDuplicate,Error)); TestTrue(TEXT("Duplicate leaves settlement bytes intact"),AfterDuplicate==OnBytes);
        A->GetCombatComponent()->FinishAttack(); X->GetCombatComponent()->FinishAttack();
        // The observer captures local counters: remove it before fixture destruction.
        B->GetCombatComponent()->OnDeath.Clear(); Y->GetCombatComponent()->OnDeath.Clear();
    }
    return true;
}
#endif
