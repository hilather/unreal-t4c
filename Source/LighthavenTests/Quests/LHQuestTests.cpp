#include "Misc/AutomationTest.h"
#include "Quests/LHQuests.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "Persistence/LHSaveCodec.h"
#include "Framework/LHWave2Profile.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHQuestTestsPrivate
{
FLHInteger I(int64 V) { return LHWave2::PrototypeInteger(V); }
FLHNumber N(double V) { return LHWave2::PrototypeNumber(V); }
struct FFixture
{
    FLHCharacterProfile Profile=LHWave2::PrototypeProfile();
    FLHSaveSnapshot S;
    bool Init()
    {
        FLHCharacterAuthority A; if (!A.Initialize(Profile,FGuid(1,2,3,4),42)) return false;
        FLHCharacterPreview V; if (A.Preview(LHWave2::PrototypeAnswers(),V)!=ELHCommandReason::None) return false;
        FLHCreateCharacterRequest R; R.Request.Epoch=FGuid(1,2,3,4); R.Request.Value=FGuid(5,6,7,8); R.DisplayName=TEXT("Reward fixture"); R.Creation=V.Creation; R.PreviewToken=V.Token;
        for (const TCHAR* Name:{TEXT("Presentation.Player.Body.A"),TEXT("Presentation.Player.Hair.Cropped"),TEXT("Presentation.Player.Skin.LightWarm"),TEXT("Presentation.Player.Outfit.StarterLinen")}) { FLHContentId ID; ID.Value=Name; R.AppearanceIds.Add(ID); }
        if (A.Execute(R).Disposition!=ELHCommandDisposition::Accepted) return false;
        A.Export(S); S.Header.BuildId=TEXT("W4RewardFixture"); S.Header.ContentRevision=LHWave2::CatalogHash(); S.Header.ChecksumAlgorithm=TEXT("SHA256"); S.Header.PayloadCodec=TEXT("LHCanonicalBinary1");
        S.Character.ActiveEntrance=LHWorld::Registry()[1].SafeFallback;
        S.Session.SafeRespawn.Entrance=LHWorld::Registry()[0].SafeFallback; S.Session.SafeRespawn.TransformResolution=ELHValueResolution::Resolved; S.Session.SafeRespawn.SafeTransform=LHWorld::FindEntrance(S.Session.SafeRespawn.Entrance)->SafeTransform;
        S.Session.ManaRegenFractionalSeconds=N(0); S.Session.EffectPolicy=ELHEffectSavePolicy::CompletedActionBoundaryOnly;
        FLHAreaRecord Hub; Hub.Area=LHWorld::Registry()[0].Id; S.World.Areas.Add(Hub);
        FLHAreaRecord B1; LHRewards::PopulateArea(B1,LHWorld::Registry()[1],[](const auto&) { return N(25); }); S.World.Areas.Add(B1);
        FLHRngState Rng; Rng.StreamId=TEXT("RNG.Loot"); Rng.Algorithm=TEXT("UE.FRandomStream"); Rng.AlgorithmRevision=1; Rng.State={42,0,0,0}; S.Session.GameplayRng.Add(Rng);
        FLHSaveError E; if (!LHSave::Validate(S,E)) return false;
        FLHCharacterAuthority Imported; if (!Imported.Initialize(Profile,S.Session.RequestEpoch,0)) return false;
        return Imported.Import(S)==ELHCommandReason::None;
    }
    FLHAreaRecord& B1() { return *S.World.Areas.FindByPredicate([](const auto& A) { return A.Area.Content.Value==TEXT("Area.TempleB1"); }); }
    const FLHAreaRecord& B1() const { return *S.World.Areas.FindByPredicate([](const auto& A) { return A.Area.Content.Value==TEXT("Area.TempleB1"); }); }
    FLHEncounterRecord& Encounter(const FGuid& Slot) { return *B1().Encounters.FindByPredicate([&](const auto& E) { return E.Life.SpawnSlot==Slot; }); }
    FLHKillFacts Facts(int32 Index=0) const
    { const auto& E=B1().Encounters[Index]; FLHKillFacts F; F.Area=E.Life.Area; F.Life=E.Life; F.Enemy=E.Definition; F.bKillerIsPlayer=true; return F; }
    FLHKillRewardSpec Spec(bool Item=true) const
    {
        FLHKillRewardSpec R; R.Experience=I(10); R.GoldMin=I(2); R.GoldMax=I(2); R.ItemDropChance=N(Item?1:0); R.RespawnSeconds=N(120); R.SafetyDistanceCm=N(1000);
        if (Item) { FLHLootEntry L; L.Item=Profile.Items[0].Id; L.MinimumQuantity=L.MaximumQuantity=I(1); L.Weight=N(1); R.Loot.Add(L); }
        return R;
    }
    ELHCommandReason Kill(int32 Index=0) { FLHQuestKillObserver O; ILHKillObserver* Ptr=&O; return LHRewards::SettleKill(S,Facts(Index),Spec(),Profile,MakeArrayView(&Ptr,1)); }
    bool Reload()
    { TArray<uint8> B; FLHSaveError E; FLHSaveSnapshot Loaded; if (!LHSave::Encode(S,B,E) || !LHSave::Decode(B,S.Header.CharacterId,{S.Header.Ruleset,S.Header.ContentRevision},Loaded,E)) return false; S=MoveTemp(Loaded); return true; }
};
TArray<uint8> Bytes(const FLHSaveSnapshot& S) { TArray<uint8> B; FLHSaveError E; LHSave::Encode(S,B,E); return B; }
FLHTakeLootRequest Loot(const FFixture& F,bool Gold=false)
{
    const auto& C=F.B1().Corpses.Last(); FLHTakeLootRequest R; R.Request.Epoch=F.S.Session.RequestEpoch; R.Request.Value=FGuid::NewGuid(); R.Container=C.Container;
    R.Kind=Gold?ELHLootTransferKind::Gold:ELHLootTransferKind::Item; R.Quantity=Gold?C.RemainingGold:C.RemainingItems[0].Quantity; if (!Gold) R.Item=C.RemainingItems[0].Id; return R;
}

FLHInteractContext Context(FFixture& F,const TCHAR* Npc=TEXT("NPC.Samaritan"))
{
    FLHInteractContext C; C.Npc.Value=Npc; C.Entity.RunId=F.S.World.RunId; C.Entity.Area=LHWorld::Registry()[0].Id; C.Entity.InstanceId=FGuid(9,8,7,6); C.DistanceCm=100; C.bLineOfSight=true; C.Profile=&F.Profile; return C;
}
FLHInteractRequest Request(const FFixture& F,const FLHInteractContext& C,const TCHAR* Topic)
{ FLHInteractRequest R; R.Request.Epoch=F.S.Session.RequestEpoch; R.Request.Value=FGuid::NewGuid(); R.Target=C.Entity; R.Topic.Value=Topic; return R; }
ELHCommandReason Interact(FFixture& F,const FLHInteractContext& C,const TCHAR* T) { return LHQuests::ExecuteInteract(F.S,C,Request(F,C,T)); }
FLHQuestRecord* Quest(FFixture& F,const TCHAR* ID=TEXT("Quest.SamaritanRats")) { return F.S.World.Quests.FindByPredicate([&](const auto& Q){return Q.Quest.Value==ID;}); }
bool Rats(FFixture& F,int32 Count)
{
    for(int32 I=0; I<Count; ++I)
    {
        if(F.Kill()!=ELHCommandReason::None) return false;
        TArray<FLHSpawnLifeId> Lives;
        if(LHRewards::AdvanceRespawns(F.B1(),F.S.World.RunId,120,[](const auto&){return true;},[](const auto&){return N(25);},Lives)!=ELHCommandReason::None) return false;
    } return true;
}
}
#define LH_QUEST_TEST(C,N) IMPLEMENT_SIMPLE_AUTOMATION_TEST(C,"Lighthaven.Quests." N,EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter) bool C::RunTest(const FString&)
LH_QUEST_TEST(FLHRatEligibility,"RatErrandEligibility")
{
    using namespace LHQuestTestsPrivate; FFixture F; if(!TestTrue(TEXT("fixture"),F.Init())) return false;
    TestTrue(TEXT("preaccept kill"),Rats(F,1)); TestNull(TEXT("no progress before acceptance"),Quest(F));
    auto C=Context(F); TestTrue(TEXT("accept"),Interact(F,C,TEXT("Topic.AcceptRats"))==ELHCommandReason::None);
    TestTrue(TEXT("postaccept kills settled"),Rats(F,3)); TestEqual(TEXT("eligible count"),Quest(F)->EligibleKillCount.Value,int64(3));
    FLHQuestKillObserver O; auto Other=F.Facts(); Other.Enemy.Value=TEXT("Enemy.Bat"); O.OnSettledKill(Other,F.S); TestEqual(TEXT("other species ignored"),Quest(F)->EligibleKillCount.Value,int64(3));
    LHRewards::AdvanceLootCleanup(F.B1(),301,[](const auto&){return false;},[](const auto&){return false;});
    TestEqual(TEXT("cleanup leaves count"),Quest(F)->EligibleKillCount.Value,int64(3)); TestTrue(TEXT("codec reload"),F.Reload()); TestEqual(TEXT("count survives reload"),Quest(F)->EligibleKillCount.Value,int64(3)); return true;
}
LH_QUEST_TEST(FLHRatSingleTurnIn,"SingleTurnIn")
{
    using namespace LHQuestTestsPrivate; FFixture F; if(!TestTrue(TEXT("fixture"),F.Init())) return false; auto C=Context(F); Interact(F,C,TEXT("Topic.AcceptRats"));
    const auto Before=Bytes(F.S); TestTrue(TEXT("premature rejects"),Interact(F,C,TEXT("Topic.TurnInRats"))==ELHCommandReason::Ineligible); TestTrue(TEXT("atomic rejection"),Before==Bytes(F.S));
    if(!TestTrue(TEXT("15 settled rat lives"),Rats(F,15))) return false;
    const auto XP=F.S.Character.ExperienceBalance.Value; auto R=Request(F,C,TEXT("Topic.TurnInRats")); FLHCommandResult Result; FString Digest;
    TestTrue(TEXT("begin receipt"),LHSave::BeginRequest(F.S,TEXT("Interact"),R.StaticStruct(),&R,Result,Digest));
    if(!TestTrue(TEXT("turn in"),LHQuests::ExecuteInteract(F.S,C,R)==ELHCommandReason::None)) return false;
    LHSave::CommitRequest(F.S,R.Request,Digest,Result); TestEqual(TEXT("quest XP distinct from kill XP"),F.S.Character.ExperienceBalance.Value,XP+2500);
    TestTrue(TEXT("accepted completed rewarded independent"),Quest(F)->bAccepted && Quest(F)->bCompleted && Quest(F)->bRewarded);
    const auto Committed=Bytes(F.S); TestFalse(TEXT("receipt replay skips domain"),LHSave::BeginRequest(F.S,TEXT("Interact"),R.StaticStruct(),&R,Result,Digest)); TestTrue(TEXT("replay result"),Result.bReplay);
    TestTrue(TEXT("reload"),F.Reload()); TestTrue(TEXT("repeat turn in rejects"),Interact(F,C,TEXT("Topic.TurnInRats"))==ELHCommandReason::Ineligible); TestTrue(TEXT("no second award"),Committed==Bytes(F.S)); return true;
}
LH_QUEST_TEST(FLHHealerInteraction,"HealerInteraction")
{
    using namespace LHQuestTestsPrivate; FFixture F; if(!TestTrue(TEXT("fixture"),F.Init())) return false; auto C=Context(F,TEXT("NPC.Nevanis")); F.S.Character.CurrentHealth=N(1);
    const auto Before=Bytes(F.S); C.DistanceCm=251; TestTrue(TEXT("range"),Interact(F,C,TEXT("Topic.Heal"))==ELHCommandReason::OutOfRange); C.DistanceCm=100; C.bLineOfSight=false; TestTrue(TEXT("LOS"),Interact(F,C,TEXT("Topic.Heal"))==ELHCommandReason::Obstructed); TestTrue(TEXT("rejections unchanged"),Before==Bytes(F.S));
    C.bLineOfSight=true; const auto MP=F.S.Character.CurrentMana.Value; const auto Gold=F.S.Character.Gold.Value; TestTrue(TEXT("heal"),Interact(F,C,TEXT("Topic.Heal"))==ELHCommandReason::None);
    FLHCharacterAuthority A; A.Initialize(F.Profile,F.S.Session.RequestEpoch,0); A.Import(F.S); TestEqual(TEXT("full derived HP"),F.S.Character.CurrentHealth.Value,A.Stats().Value.MaxHealth); TestEqual(TEXT("no mana"),F.S.Character.CurrentMana.Value,MP); TestEqual(TEXT("free"),F.S.Character.Gold.Value,Gold);
    TestTrue(TEXT("already full"),Interact(F,C,TEXT("Topic.Heal"))==ELHCommandReason::NoEffect); F.S.Character.CurrentHealth=N(0); TestTrue(TEXT("dead rejects"),Interact(F,C,TEXT("Topic.Heal"))==ELHCommandReason::InvalidLifeState); return true;
}
LH_QUEST_TEST(FLHBalorkFlow,"BalorkCompletionFlow")
{
    using namespace LHQuestTestsPrivate; FFixture F; if(!TestTrue(TEXT("fixture"),F.Init())) return false; FLHQuestKillObserver O; ILHKillObserver* Ptr=&O; F.B1().Encounters[0].Definition.Value=TEXT("Enemy.Balork"); auto Facts=F.Facts(); auto Spec=F.Spec(false); Spec.bBoss=true; Spec.RespawnSeconds=N(900);
    TestTrue(TEXT("settled boss defeat"),LHRewards::SettleKill(F.S,Facts,Spec,F.Profile,MakeArrayView(&Ptr,1))==ELHCommandReason::None); auto* Q=Quest(F,TEXT("Quest.BalorkReturn")); TestFalse(TEXT("death is not completion"),Q->bCompleted);
    F.S.Character.ActiveEntrance=LHWorld::Registry()[0].SafeFallback; TestFalse(TEXT("arrival not completion"),Q->bCompleted); auto C=Context(F,TEXT("NPC.BrotherKiran")); const auto XP=F.S.Character.ExperienceBalance.Value;
    TestTrue(TEXT("explicit topic completes"),Interact(F,C,TEXT("Topic.BalorkReturn"))==ELHCommandReason::None); TestTrue(TEXT("reload"),F.Reload()); TestTrue(TEXT("completed persisted"),Quest(F,TEXT("Quest.BalorkReturn"))->bCompleted);
    TestEqual(TEXT("completion has no numeric reward"),F.S.Character.ExperienceBalance.Value,XP);
    TArray<FLHSpawnLifeId> Lives; TestTrue(TEXT("900 second recurring boss respawn"),LHRewards::AdvanceRespawns(F.B1(),F.S.World.RunId,900,[](const auto&){return true;},[](const auto&){return N(25);},Lives)==ELHCommandReason::None); Facts=F.Facts();
    TestTrue(TEXT("second life settlement"),LHRewards::SettleKill(F.S,Facts,Spec,F.Profile,MakeArrayView(&Ptr,1))==ELHCommandReason::None);
    TestTrue(TEXT("new life retains complete stage"),Quest(F,TEXT("Quest.BalorkReturn"))->Stage==TEXT("Complete")); TestTrue(TEXT("repeat topic unavailable"),Interact(F,C,TEXT("Topic.BalorkReturn"))==ELHCommandReason::Ineligible); TestEqual(TEXT("only second life kill XP"),F.S.Character.ExperienceBalance.Value,XP+Spec.Experience.Value); return true;
}
#endif
