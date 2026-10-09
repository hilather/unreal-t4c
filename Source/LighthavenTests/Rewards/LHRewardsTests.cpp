#include "Misc/AutomationTest.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "Persistence/LHSaveCodec.h"
#include "Framework/LHWave2Profile.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHRewardsTestsPrivate
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
    ELHCommandReason Kill(int32 Index=0) { return LHRewards::SettleKill(S,Facts(Index),Spec(),Profile,{}); }
    bool Reload()
    { TArray<uint8> B; FLHSaveError E; FLHSaveSnapshot Loaded; if (!LHSave::Encode(S,B,E) || !LHSave::Decode(B,S.Header.CharacterId,{S.Header.Ruleset,S.Header.ContentRevision},Loaded,E)) return false; S=MoveTemp(Loaded); return true; }
};
TArray<uint8> Bytes(const FLHSaveSnapshot& S) { TArray<uint8> B; FLHSaveError E; LHSave::Encode(S,B,E); return B; }
FLHTakeLootRequest Loot(const FFixture& F,bool Gold=false)
{
    const auto& C=F.B1().Corpses.Last(); FLHTakeLootRequest R; R.Request.Epoch=F.S.Session.RequestEpoch; R.Request.Value=FGuid::NewGuid(); R.Container=C.Container;
    R.Kind=Gold?ELHLootTransferKind::Gold:ELHLootTransferKind::Item; R.Quantity=Gold?C.RemainingGold:C.RemainingItems[0].Quantity; if (!Gold) R.Item=C.RemainingItems[0].Id; return R;
}
struct FObserver : ILHKillObserver
{
    int32 Calls=0; bool Fail=false;
    ELHCommandReason OnSettledKill(const FLHKillFacts&,FLHSaveSnapshot& S) override { ++Calls; S.Character.Gold.Value++; return Fail?ELHCommandReason::Ineligible:ELHCommandReason::None; }
};
}
#define LH_REWARD_TEST(Class, Name) IMPLEMENT_SIMPLE_AUTOMATION_TEST(Class,"Lighthaven.Rewards." Name,EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter) bool Class::RunTest(const FString&)
LH_REWARD_TEST(FLHOneKillOneReward,"OneKillOneReward")
{
    using namespace LHRewardsTestsPrivate; FFixture F; if (!TestTrue(TEXT("Fixture"),F.Init())) return false;
    auto Facts=F.Facts(); auto Spec=F.Spec(); FObserver Observer; ILHKillObserver* O=&Observer; auto View=MakeArrayView(&O,1);
    const auto XP=F.S.Character.ExperienceBalance.Value;
    TestTrue(TEXT("Kill accepted"),LHRewards::SettleKill(F.S,Facts,Spec,F.Profile,View)==ELHCommandReason::None);
    const auto Settled=Bytes(F.S);
    TestTrue(TEXT("Duplicate kill rejected"),LHRewards::SettleKill(F.S,Facts,Spec,F.Profile,View)==ELHCommandReason::InvalidLifeState);
    TestEqual(TEXT("One XP award"),F.S.Character.ExperienceBalance.Value,XP+10); TestEqual(TEXT("One observer"),Observer.Calls,1);
    TestEqual(TEXT("One finalized corpse"),F.B1().Corpses.Num(),1); TestTrue(TEXT("Repeat unchanged bytes"),Bytes(F.S)==Settled);
    auto Other=F.Facts(1); Observer.Fail=true; const auto Before=Bytes(F.S);
    TestTrue(TEXT("Observer rejection propagates"),LHRewards::SettleKill(F.S,Other,Spec,F.Profile,View)==ELHCommandReason::Ineligible);
    TestTrue(TEXT("Observer failure rolls back XP loot RNG and lifecycle"),Bytes(F.S)==Before);
    return true;
}
LH_REWARD_TEST(FLHFullInventoryLootIntact,"FullInventoryLootIntact")
{
    using namespace LHRewardsTestsPrivate; FFixture F;
    // Creation grants one inventory entry per starter item. Fill this fixture exactly.
    F.Profile.InventorySlots=I(F.Profile.StarterItems.Num());
    if (!TestTrue(TEXT("Fixture"),F.Init())) return false;
    TestEqual(TEXT("Fixture inventory fills every slot"),int64(F.S.Character.Inventory.Num()),F.Profile.InventorySlots.Value);
    if (!TestTrue(TEXT("Kill"),F.Kill()==ELHCommandReason::None)) return false; const auto Before=Bytes(F.S); auto R=Loot(F);
    TestTrue(TEXT("Full inventory rejects"),LHRewards::ExecuteTakeLoot(F.S,F.Profile,R)==ELHCommandReason::InventoryFull);
    TestTrue(TEXT("Both sides byte identical"),Bytes(F.S)==Before);
    R=Loot(F,true); FLHCommandResult Result; FString Digest;
    TestTrue(TEXT("Begin gold transfer"),LHSave::BeginRequest(F.S,TEXT("TakeLoot"),R.StaticStruct(),&R,Result,Digest));
    TestTrue(TEXT("Gold succeeds despite full inventory"),LHRewards::ExecuteTakeLoot(F.S,F.Profile,R)==ELHCommandReason::None);
    LHSave::CommitRequest(F.S,R.Request,Digest,Result); const auto Committed=Bytes(F.S);
    TestFalse(TEXT("Replay skips domain"),LHSave::BeginRequest(F.S,TEXT("TakeLoot"),R.StaticStruct(),&R,Result,Digest)); TestTrue(TEXT("Replay accepted"),Result.bReplay);
    TestTrue(TEXT("Replay changes nothing"),Bytes(F.S)==Committed); TestFalse(TEXT("Item still recoverable"),F.B1().Corpses[0].bClaimed);
    return true;
}
LH_REWARD_TEST(FLHSaveReloadNoDuplicate,"SaveReloadNoDuplicate")
{
    using namespace LHRewardsTestsPrivate; FFixture F; if (!TestTrue(TEXT("Fixture"),F.Init())) return false;
    auto Facts=F.Facts(); if (!TestTrue(TEXT("Kill"),F.Kill()==ELHCommandReason::None)) return false; const auto Before=Bytes(F.S);
    TestTrue(TEXT("Encode decode reload"),F.Reload()); TestTrue(TEXT("No reroll on reload"),Bytes(F.S)==Before);
    TestTrue(TEXT("Repeat callback rejected"),LHRewards::SettleKill(F.S,Facts,F.Spec(),F.Profile,{})==ELHCommandReason::InvalidLifeState);
    TestTrue(TEXT("XP RNG corpse claim unchanged"),Bytes(F.S)==Before); return true;
}
LH_REWARD_TEST(FLHRespawnGenerationAndClock,"RespawnGenerationAndClock")
{
    using namespace LHRewardsTestsPrivate; FFixture F; if (!TestTrue(TEXT("Fixture"),F.Init())) return false;
    auto Facts=F.Facts(); if (!TestTrue(TEXT("Kill"),F.Kill()==ELHCommandReason::None)) return false; auto Old=F.Encounter(Facts.Life.SpawnSlot).KillReward;
    TArray<FLHSpawnLifeId> Lives; auto Advance=[&](double Time,bool Safe) { return LHRewards::AdvanceRespawns(F.B1(),F.S.World.RunId,Time,[&](const auto&) { return Safe; },[](const auto&) { return N(25); },Lives); };
    Advance(0,true); TestEqual(TEXT("Paused/unloaded contributes zero"),F.Encounter(Facts.Life.SpawnSlot).RespawnRemainingSeconds.Value,120.0);
    Advance(100,true); TestTrue(TEXT("Travel reload"),F.Reload()); TestEqual(TEXT("Travel does not reset remainder"),F.Encounter(Facts.Life.SpawnSlot).RespawnRemainingSeconds.Value,20.0);
    Advance(20,false); TestEqual(TEXT("Unsafe waits"),Lives.Num(),0); TestEqual(TEXT("Generation not advanced"),F.Encounter(Facts.Life.SpawnSlot).Life.LifeGeneration,int64(0));
    Advance(1,true); if (!TestEqual(TEXT("One new life"),Lives.Num(),1)) return false; TestEqual(TEXT("Generation advances once"),Lives[0].LifeGeneration,int64(1)); TestEqual(TEXT("Full HP"),F.Encounter(Facts.Life.SpawnSlot).CurrentHealth.Value,25.0);
    TestTrue(TEXT("New reward id"),F.Encounter(Facts.Life.SpawnSlot).KillReward.Value!=Old.Value);
    TestTrue(TEXT("Old callback rejected"),LHRewards::SettleKill(F.S,Facts,F.Spec(),F.Profile,{})==ELHCommandReason::InvalidLifeState);
    Advance(120,true); TestEqual(TEXT("Alive not advanced again"),Lives.Num(),1);
    TestTrue(TEXT("New life can settle"),LHRewards::SettleKill(F.S,FLHKillFacts{Facts.Area,Lives[0],Facts.Enemy,true},F.Spec(),F.Profile,{})==ELHCommandReason::None); return true;
}
LH_REWARD_TEST(FLHBossRespawnSingleClaim,"BossRespawnSingleClaim")
{
    using namespace LHRewardsTestsPrivate; FFixture F; if (!TestTrue(TEXT("Fixture"),F.Init())) return false;
    auto& E=F.B1().Encounters[0]; E.Definition.Value=TEXT("Enemy.Balork"); auto Facts=F.Facts(); auto Spec=F.Spec(false); Spec.bBoss=true; Spec.RespawnSeconds=N(900);
    if (!TestTrue(TEXT("First defeat"),LHRewards::SettleKill(F.S,Facts,Spec,F.Profile,{})==ELHCommandReason::None)) return false; auto Claim=F.S.World.Bosses[0].UniqueClaim;
    TestTrue(TEXT("Reload boss"),F.Reload()); TArray<FLHSpawnLifeId> Lives;
    LHRewards::AdvanceRespawns(F.B1(),F.S.World.RunId,899,[](const auto&) { return true; },[](const auto&) { return N(508); },Lives); TestEqual(TEXT("Not before 15 minutes"),Lives.Num(),0);
    LHRewards::AdvanceRespawns(F.B1(),F.S.World.RunId,1,[](const auto&) { return true; },[](const auto&) { return N(508); },Lives); if (!TestEqual(TEXT("Boss respawns"),Lives.Num(),1)) return false;
    F.S.Character.ActiveEntrance=LHWorld::Registry()[0].SafeFallback; TestTrue(TEXT("Travel save retains claim"),F.Reload()); F.S.Character.ActiveEntrance=LHWorld::Registry()[1].SafeFallback;
    TestTrue(TEXT("Second life reward"),LHRewards::SettleKill(F.S,FLHKillFacts{Facts.Area,Lives[0],Facts.Enemy,true},Spec,F.Profile,{})==ELHCommandReason::None);
    TestTrue(TEXT("Defeat retained"),F.S.World.Bosses[0].bDefeated); TestEqual(TEXT("Same permanent claim"),F.S.World.Bosses[0].UniqueClaim.Value,Claim.Value); TestEqual(TEXT("One claim"),F.S.World.ClaimedUniqueRewards.Num(),1);
    TestEqual(TEXT("Two ordinary XP awards"),F.S.Character.ExperienceBalance.Value,int64(20)); return true;
}
LH_REWARD_TEST(FLHPlayerDeathSettlesOnce,"PlayerDeathSettlesOnce")
{
    using namespace LHRewardsTestsPrivate; FFixture F; if (!TestTrue(TEXT("Fixture"),F.Init())) return false;
    FLHCharacterAuthority Character;
    if (!TestTrue(TEXT("Recovery stats authority initializes"),Character.Initialize(F.Profile,F.S.Session.RequestEpoch,0))) return false;
    if (!TestTrue(TEXT("Recovery stats import"),Character.Import(F.S)==ELHCommandReason::None)) return false;
    const auto Stats=Character.Stats();
    if (!TestTrue(TEXT("Recovery maxima resolve"),Stats.Diagnostic.IsAccepted())) return false;
    F.S.Character.CurrentHealth=N(0); F.S.Character.CurrentMana=N(0); const auto Before=F.S;
    TestTrue(TEXT("Unreviewed fails"),LHRewards::SettlePlayerDeath(F.S,F.Profile,F.S.Session.SafeRespawn,false)==ELHCommandReason::InvalidDestination);
    const auto Recovery=F.S.Session.SafeRespawn;
    TestTrue(TEXT("Death settled"),LHRewards::SettlePlayerDeath(F.S,F.Profile,Recovery,true)==ELHCommandReason::None); const auto Settled=Bytes(F.S);
    TestTrue(TEXT("Duplicate callback rejected"),LHRewards::SettlePlayerDeath(F.S,F.Profile,Recovery,true)==ELHCommandReason::InvalidLifeState); TestTrue(TEXT("Second callback unchanged"),Bytes(F.S)==Settled);
    TestEqual(TEXT("XP retained"),F.S.Character.ExperienceBalance.Value,Before.Character.ExperienceBalance.Value); TestEqual(TEXT("Level retained"),F.S.Character.EarnedLevel.Value,Before.Character.EarnedLevel.Value);
    TestEqual(TEXT("Gold retained"),F.S.Character.Gold.Value,Before.Character.Gold.Value); auto Expected=Before.Character; Expected.ActiveEntrance=F.S.Character.ActiveEntrance; Expected.CurrentHealth=F.S.Character.CurrentHealth; Expected.CurrentMana=F.S.Character.CurrentMana;
    TestTrue(TEXT("All non-recovery character fields identical"),FLHCharacterRecord::StaticStruct()->CompareScriptStruct(&F.S.Character,&Expected,0));
    TestEqual(TEXT("Full HP"),F.S.Character.CurrentHealth.Value,Stats.Value.MaxHealth); TestEqual(TEXT("Full MP"),F.S.Character.CurrentMana.Value,Stats.Value.MaxMana); return true;
}
LH_REWARD_TEST(FLHRenewableRatRoute,"RenewableRatRoute")
{
    using namespace LHRewardsTestsPrivate; FFixture F; if (!TestTrue(TEXT("Fixture"),F.Init())) return false;
    int32 Rats=0,Bats=0,Slimes=0; for (const auto& E:F.B1().Encounters) { if (E.Definition.Value==TEXT("Enemy.BrownRat")) ++Rats; if (E.Definition.Value==TEXT("Enemy.Bat")) ++Bats; if (E.Definition.Value==TEXT("Enemy.GreenSlime")) ++Slimes; }
    TestEqual(TEXT("B1 rat slots"),Rats,12); TestEqual(TEXT("B1 bat slots"),Bats,3); TestEqual(TEXT("B1 slime slots"),Slimes,2);
    const int64 StartingGold=F.S.Character.Gold.Value;
    const int64 StartingXP=F.S.Character.ExperienceBalance.Value;
    int32 Kills=0;
    for (int32 Round=0;Round<2;++Round)
    {
        for (int32 J=0;J<F.B1().Encounters.Num() && Kills<15;++J)
        {
            if (F.B1().Encounters[J].Definition.Value!=TEXT("Enemy.BrownRat")) continue;
            TestTrue(TEXT("Renewable rat settlement"),LHRewards::SettleKill(F.S,F.Facts(J),F.Spec(false),F.Profile,{})==ELHCommandReason::None); ++Kills;
            auto R=Loot(F,true); TestTrue(TEXT("Gold pickup"),LHRewards::ExecuteTakeLoot(F.S,F.Profile,R)==ELHCommandReason::None);
        }
        if (Kills<15) { TArray<FLHSpawnLifeId> Lives; LHRewards::AdvanceRespawns(F.B1(),F.S.World.RunId,120,[](const auto&) { return true; },[](const auto&) { return N(25); },Lives); TestTrue(TEXT("Round reload"),F.Reload()); }
    }
    TestEqual(TEXT("Fifteen rat kills"),Kills,15); TestEqual(TEXT("Earned gold without grants"),F.S.Character.Gold.Value-StartingGold,int64(30)); TestEqual(TEXT("Earned XP"),F.S.Character.ExperienceBalance.Value-StartingXP,int64(150)); return true;
}
LH_REWARD_TEST(FLHLootCleanupProtection,"LootCleanupProtection")
{
    using namespace LHRewardsTestsPrivate; FFixture F; if (!TestTrue(TEXT("Fixture"),F.Init())) return false; if (!TestTrue(TEXT("Kill"),F.Kill()==ELHCommandReason::None)) return false;
    auto Protected=[](const FLHContentId&) { return true; }; auto Unreferenced=[](const FLHEntityId&) { return false; };
    LHRewards::AdvanceLootCleanup(F.B1(),301,Protected,Unreferenced); TestEqual(TEXT("Protected unique/quest remains"),F.B1().Corpses.Num(),1);
    LHRewards::AdvanceLootCleanup(F.B1(),301,[](const auto&) { return false; },[](const auto&) { return true; }); TestEqual(TEXT("Live reference remains"),F.B1().Corpses.Num(),1);
    LHRewards::AdvanceLootCleanup(F.B1(),301,[](const auto&) { return false; },Unreferenced); TestEqual(TEXT("Ordinary expires"),F.B1().Corpses.Num(),0);
    TestTrue(TEXT("Cleanup cannot reaward kill"),F.Kill()==ELHCommandReason::InvalidLifeState); return true;
}
LH_REWARD_TEST(FLHPartialLootConservation,"PartialLootConservation")
{
    using namespace LHRewardsTestsPrivate; FFixture F; F.Profile.InventorySlots=I(16); F.Profile.Items[0].StackLimit=I(10);
    if (!TestTrue(TEXT("Fixture"),F.Init())) return false;
    auto Spec=F.Spec(); Spec.Loot[0].MinimumQuantity=Spec.Loot[0].MaximumQuantity=I(3);
    if (!TestTrue(TEXT("Kill"),LHRewards::SettleKill(F.S,F.Facts(),Spec,F.Profile,{})==ELHCommandReason::None)) return false;
    auto R=Loot(F); const auto Source=R.Item; R.Quantity=I(1);
    TestTrue(TEXT("Partial pickup"),LHRewards::ExecuteTakeLoot(F.S,F.Profile,R)==ELHCommandReason::None);
    TestEqual(TEXT("Source remainder"),F.B1().Corpses[0].RemainingItems[0].Quantity.Value,int64(2));
    const auto& Split=F.S.Character.Inventory.Last(); TestTrue(TEXT("Split has distinct identity"),Split.Id.InstanceId!=Source.InstanceId);
    TestEqual(TEXT("Split quantity"),Split.Quantity.Value,int64(1));
    TestTrue(TEXT("Reload split"),F.Reload()); R=Loot(F);
    TestTrue(TEXT("Full remainder transfer"),LHRewards::ExecuteTakeLoot(F.S,F.Profile,R)==ELHCommandReason::None);
    TestTrue(TEXT("Original identity retained on full transfer"),F.S.Character.Inventory.ContainsByPredicate([&](const auto& Item) { return Item.Id.InstanceId==Source.InstanceId && Item.Quantity.Value==2; }));
    R=Loot(F,true); TestTrue(TEXT("Final gold transfer"),LHRewards::ExecuteTakeLoot(F.S,F.Profile,R)==ELHCommandReason::None);
    TestTrue(TEXT("Empty remainder claimed"),F.B1().Corpses[0].bClaimed);
    const auto Before=Bytes(F.S); TestTrue(TEXT("Missing container rejected"),LHRewards::ExecuteTakeLoot(F.S,F.Profile,R)==ELHCommandReason::NotFound); TestTrue(TEXT("Stale pickup intact"),Bytes(F.S)==Before);
    return true;
}
LH_REWARD_TEST(FLHPermanentDefeatAndOverflow,"PermanentDefeatAndOverflow")
{
    using namespace LHRewardsTestsPrivate; FFixture F; if (!TestTrue(TEXT("Fixture"),F.Init())) return false;
    auto Facts=F.Facts(); auto Spec=F.Spec(false); Spec.RespawnPolicy=ELHRespawnPolicy::PermanentDefeat;
    if (!TestTrue(TEXT("Permanent settlement"),LHRewards::SettleKill(F.S,Facts,Spec,F.Profile,{})==ELHCommandReason::None)) return false;
    TArray<FLHSpawnLifeId> Lives; LHRewards::AdvanceRespawns(F.B1(),F.S.World.RunId,1000,[](const auto&) { return true; },[](const auto&) { return N(25); },Lives); TestEqual(TEXT("Permanent never respawns"),Lives.Num(),0);
    const auto Other=F.Facts(1); TestTrue(TEXT("Ordinary settlement"),LHRewards::SettleKill(F.S,Other,F.Spec(false),F.Profile,{})==ELHCommandReason::None);
    F.Encounter(Other.Life.SpawnSlot).Life.LifeGeneration=MAX_int64; F.B1().Corpses.Last().SourceLife.LifeGeneration=MAX_int64;
    const auto Before=F.B1();
    TestTrue(TEXT("Generation exhaustion busy"),LHRewards::AdvanceRespawns(F.B1(),F.S.World.RunId,120,[](const auto&) { return true; },[](const auto&) { return N(25); },Lives)==ELHCommandReason::Busy);
    TestTrue(TEXT("Overflow leaves area intact"),FLHAreaRecord::StaticStruct()->CompareScriptStruct(&Before,&F.B1(),0)); TestEqual(TEXT("No published new life"),Lives.Num(),0); return true;
}
#undef LH_REWARD_TEST
#endif
