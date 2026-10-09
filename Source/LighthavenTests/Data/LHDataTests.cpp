#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Internationalization/Regex.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Data/Encounters/LHEncounterCatalog.h"
#include "Data/Items/LHItemCatalog.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "Framework/LHWave2Profile.h"
#include "Persistence/LHSaveCodec.h"
namespace LHDataTestsPrivate
{
constexpr auto Flags=EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter;
bool Provenance(const FLHFieldProvenance& P)
{ return !P.FieldPath.IsNone() && !P.SourceBaseline.IsEmpty() && !P.RetrievedDate.IsEmpty() && !P.Notes.IsEmpty() && P.Status!=ELHProvenanceStatus::Missing && (P.Status==ELHProvenanceStatus::Prototype || P.Status==ELHProvenanceStatus::Modernized || !P.SourceUrl.IsEmpty()); }
template<class T> bool Ready(const T& V)
{ return V.Resolution==ELHValueResolution::Resolved && Provenance(V.Provenance); }
FLHContentId Id(const TCHAR* S) { FLHContentId I; I.Value=S; return I; }
bool RewardFixture(const FLHEnemyCatalogRow& Enemy, FLHSaveSnapshot& S, FLHCharacterProfile& P, FLHKillFacts& Facts)
{
    P=LHWave2::PrototypeProfile();
    // Own the fixture catalog explicitly; the production profile is already populated.
    P.Items.Reset();
    for (const auto& Item:LHItemData::Catalog()) P.Items.Add(Item.Character);
    FLHCharacterAuthority A; if (!A.Initialize(P,FGuid(1,2,3,4),42)) return false;
    FLHCharacterPreview V; if (A.Preview(LHWave2::PrototypeAnswers(),V)!=ELHCommandReason::None) return false;
    FLHCreateCharacterRequest R; R.Request.Epoch=FGuid(1,2,3,4); R.Request.Value=FGuid(5,6,7,8); R.DisplayName=TEXT("Data fixture"); R.Creation=V.Creation; R.PreviewToken=V.Token;
    for (const TCHAR* Name:{TEXT("Presentation.Player.Body.A"),TEXT("Presentation.Player.Hair.Cropped"),TEXT("Presentation.Player.Skin.LightWarm"),TEXT("Presentation.Player.Outfit.StarterLinen")}) R.AppearanceIds.Add(Id(Name));
    if (A.Execute(R).Disposition!=ELHCommandDisposition::Accepted) return false;
    A.Export(S); S.Header.BuildId=TEXT("W4DataFixture"); S.Header.ContentRevision=LHWave2::CatalogHash(); S.Header.ChecksumAlgorithm=TEXT("SHA256"); S.Header.PayloadCodec=TEXT("LHCanonicalBinary1");
    const FLHAreaDefinition* Selected=nullptr; const FLHSpawnAuthoring* Spawn=nullptr;
    for (const auto& Area:LHWorld::Registry()) for (const auto& Slot:Area.Spawns) if (!Spawn && Slot.Enemy.Value==Enemy.Id.Value) { Selected=&Area; Spawn=&Slot; }
    if (!Spawn) return false;
    S.Character.ActiveEntrance=Selected->SafeFallback;
    S.Session.SafeRespawn.Entrance=LHWorld::Registry()[0].SafeFallback; S.Session.SafeRespawn.TransformResolution=ELHValueResolution::Resolved;
    S.Session.SafeRespawn.SafeTransform=LHWorld::FindEntrance(S.Session.SafeRespawn.Entrance)->SafeTransform;
    S.Session.ManaRegenFractionalSeconds=LHWave2::PrototypeNumber(0); S.Session.EffectPolicy=ELHEffectSavePolicy::CompletedActionBoundaryOnly;
    FLHAreaRecord Hub; Hub.Area=LHWorld::Registry()[0].Id; S.World.Areas.Add(Hub);
    FLHAreaRecord Floor;
    if (LHRewards::PopulateArea(Floor,*Selected,[](const auto& I){return LHEnemyData::Find(I)->Health;})!=ELHCommandReason::None) return false;
    S.World.Areas.Add(Floor);
    Facts.Area=Selected->Id; Facts.Life=Floor.Encounters.FindByPredicate([&](const auto& E){return E.Life.SpawnSlot==Spawn->SpawnId;})->Life;
    Facts.Enemy=Enemy.Id; Facts.bKillerIsPlayer=true;
    FLHRngState Rng; Rng.StreamId=TEXT("RNG.Loot"); Rng.Algorithm=TEXT("UE.FRandomStream"); Rng.AlgorithmRevision=1; Rng.State={42,0,0,0}; S.Session.GameplayRng.Add(Rng);
    FLHSaveError Error; return LHSave::Validate(S,Error);
}
}
#define LH_DATA_TEST(Class,Name) IMPLEMENT_SIMPLE_AUTOMATION_TEST(Class,"Lighthaven.Data." Name,LHDataTestsPrivate::Flags) bool Class::RunTest(const FString&)
LH_DATA_TEST(FLHDataRoster,"RosterContractParity")
{
    FString Text; if (!TestTrue(TEXT("Roster contract readable"),FFileHelper::LoadFileToString(Text,*(FPaths::ProjectDir()/TEXT("docs/plan/contracts/enemy-roster.json"))))) return false;
    // Frozen planning contract has flat enemy objects; avoid adding an out-of-scope Json module dependency.
    FRegexMatcher Rows(FRegexPattern(TEXT(R"LH("id"\s*:\s*"([^"]+)"[\s\S]*?"prototype_floors"\s*:\s*\[([^\]]*)\])LH")),Text);
    TSet<FName> Seen;
    while (Rows.FindNext())
    {
        FLHContentId Id; Id.Value=FName(Rows.GetCaptureGroup(1)); Seen.Add(Id.Value);
        TestNotNull(TEXT("Contract ID resolves"),LHEnemyData::Find(Id));
        TArray<FString> Floors; Rows.GetCaptureGroup(2).ParseIntoArray(Floors,TEXT(","),true);
        for (const auto& Floor:Floors)
        {
            const FString AreaName=FString::Printf(TEXT("Area.TempleB%d"),FCString::Atoi(*Floor));
            const auto* Area=LHWorld::Registry().FindByPredicate([&](const auto& A){return A.Id.Content.Value==FName(AreaName);});
            TestTrue(TEXT("Required floor has slot"),Area && Area->Spawns.ContainsByPredicate([&](const auto& S){return S.Enemy.Value==Id.Value;}));
        }
    }
    TestEqual(TEXT("Exactly eleven contract IDs"),Seen.Num(),11); TestEqual(TEXT("Exactly eleven definitions"),LHEnemyData::Catalog().Num(),11);
    for (const auto& E:LHEnemyData::Catalog()) TestTrue(TEXT("No extra species"),Seen.Contains(E.Id.Value)); return true;
}
LH_DATA_TEST(FLHDataSpawns,"RegistrySpawnsResolve")
{
    TestEqual(TEXT("77 encounter rows"),LHEncounterData::Catalog().Num(),77); TSet<FGuid> Seen; int Bosses=0; const int Budget[]={0,17,25,22,13};
    for (int I=0;I<LHWorld::Registry().Num();++I)
    {
        const auto& A=LHWorld::Registry()[I]; TestEqual(TEXT("Floor budget"),A.Spawns.Num(),Budget[I]);
        for (const auto& S:A.Spawns)
        {
            const auto* R=LHEncounterData::ForSpawn(S.SpawnId); if (!TestNotNull(TEXT("Encounter resolves"),R)) continue;
            TestFalse(TEXT("Unique GUID"),Seen.Contains(R->SpawnId)); Seen.Add(R->SpawnId);
            TestTrue(TEXT("Area and enemy unchanged"),R->Area.Content.Value==A.Id.Content.Value && R->Enemy.Value==S.Enemy.Value);
            TestNotNull(TEXT("Enemy resolves"),LHEnemyData::Find(R->Enemy));
            TestTrue(TEXT("Ordinary repeat"),R->RespawnPolicy==ELHRespawnPolicy::OrdinaryRepeat);
            TestEqual(TEXT("Safety distance"),R->SafetyDistanceCm.Value,1000.);
            if (R->bBoss) { ++Bosses; TestTrue(TEXT("Only Balork is boss"),R->Enemy.Value==TEXT("Enemy.Balork")); TestEqual(TEXT("Bible respawn"),R->RespawnSeconds.Value,900.); }
        }
    }
    TestEqual(TEXT("One boss slot"),Bosses,1); return true;
}
LH_DATA_TEST(FLHDataProvenance,"ProvenanceAndUnresolved")
{
    using namespace LHDataTestsPrivate;
    FString Lookup; if (!TestTrue(TEXT("R-03 lookup readable"),FFileHelper::LoadFileToString(Lookup,*(FPaths::ProjectDir()/TEXT("docs/implementation/research/w4-bible-lookup.md"))))) return false;
    for (const TCHAR* MissingRow:{TEXT("All11 enemies: numeric attack rating / attack speed | missing"),TEXT("All11 enemies: unconditional XP / damage distribution / gold distribution | missing"),TEXT("All11 enemies: drop odds / quantity / simultaneous drops | missing"),TEXT("All10 ordinary roster enemies: respawn | missing"),TEXT("Dungeon Bat floors | missing"),TEXT("Respawn safety distance | missing"),TEXT("Balork arena / phases / summons | confirmed (floor); missing (arena/script)")})
        TestTrue(TEXT("Prototype dependency remains explicitly missing in R-03"),Lookup.Contains(MissingRow));
    for (const auto& E:LHEnemyData::Catalog())
    {
        for (const auto* V:{&E.Level,&E.ExperienceColumns[0],&E.ExperienceColumns[1],&E.ExperienceColumns[2],&E.GoldMinimum,&E.GoldMaximum,&E.Reward.Experience,&E.Reward.GoldMin,&E.Reward.GoldMax}) TestTrue(TEXT("Integer provenance"),Ready(*V));
        for (const auto* V:{&E.Health,&E.MinimumDamage,&E.MaximumDamage,&E.Runtime.CapsuleRadiusCm,&E.Runtime.CapsuleHalfHeightCm,&E.Runtime.Accuracy,&E.Runtime.Avoidance,&E.Runtime.Armor,&E.Runtime.Resistance,&E.Runtime.DamageBonus,&E.Runtime.MaxMana,&E.Runtime.Attack.ManaCost,&E.Runtime.Attack.CooldownSeconds,&E.Runtime.Attack.ImpactSeconds,&E.Runtime.Attack.RangeCm,&E.Runtime.Attack.WeaponMinimum,&E.Runtime.Attack.WeaponMaximum,&E.Reward.RespawnSeconds,&E.Reward.SafetyDistanceCm,&E.Reward.ItemDropChance}) TestTrue(TEXT("Number provenance"),Ready(*V));
        for (const auto& F:E.AIParameters) TestTrue(TEXT("Registered AI provenance"),Ready(F.Value));
        TestTrue(TEXT("Loot membership provenance"),Provenance(E.LootMembership));
        for (const auto& L:E.Reward.Loot) { TestTrue(TEXT("Loot quantity provenance"),Ready(L.MinimumQuantity)&&Ready(L.MaximumQuantity)); TestTrue(TEXT("Loot weight provenance"),Ready(L.Weight)); }
    }
    int Dungeon=0;
    for (const auto& R:LHEncounterData::Catalog())
    {
        TestTrue(TEXT("Placement provenance"),Provenance(R.PlacementProvenance));
        for (const auto& P:R.Policies) TestTrue(TEXT("Policy provenance"),Provenance(P.Provenance));
        if (R.Enemy.Value==TEXT("Enemy.DungeonBat")) { ++Dungeon; TestTrue(TEXT("Provisional B2"),R.Area.Content.Value==TEXT("Area.TempleB2") && R.PlacementProvenance.Status==ELHProvenanceStatus::Prototype && R.PlacementProvenance.Notes.Contains(TEXT("floors missing"))); }
    }
    TestEqual(TEXT("Two provisional dungeon bats"),Dungeon,2);
    for (const auto& I:LHItemData::Catalog())
    {
        TestTrue(TEXT("Carry values traced"),Ready(I.Character.Weight)&&Ready(I.Character.StackLimit));
        if (!I.bEquipable && !I.Combat.bConsumable) { TestTrue(TEXT("Unknown stats unresolved"),I.Encumbrance.Resolution==ELHValueResolution::Unresolved && I.Character.Eligibility.MinimumLevel.Resolution==ELHValueResolution::Unresolved); TestTrue(TEXT("Cannot equip"),I.Character.Slot==ELHEquipmentSlot::Unspecified); }
        if (I.SellGold.Resolution==ELHValueResolution::Resolved) TestTrue(TEXT("Resolved sale provenance"),Ready(I.SellGold));
    }
    return true;
}
LH_DATA_TEST(FLHDataRuntime,"RuntimeSpecsValidate")
{
    using namespace LHDataTestsPrivate;
    for (const auto& E:LHEnemyData::Catalog())
    {
        FString Error; const bool Valid=LHAI::ValidateSpec(E.Runtime,Error);
        TestTrue(*FString::Printf(TEXT("%s runtime: %s"),*E.Id.Value.ToString(),*Error),Valid);
        auto Invalid=E.Runtime; Invalid.Attack.WeaponMinimum.Resolution=ELHValueResolution::Unresolved;
        TestFalse(TEXT("Unresolved damage rejects"),LHAI::ValidateSpec(Invalid,Error));
        FLHSaveSnapshot S; FLHCharacterProfile P; FLHKillFacts F;
        if (!TestTrue(TEXT("Reward fixture validates"),RewardFixture(E,S,P,F))) continue;
        auto InvalidReward=E.Reward; InvalidReward.Experience.Resolution=ELHValueResolution::Unresolved;
        TArray<uint8> Before,After; FLHSaveError SaveError; LHSave::Encode(S,Before,SaveError);
        TestTrue(TEXT("Unresolved reward rejects"),LHRewards::SettleKill(S,F,InvalidReward,P,{})==ELHCommandReason::UnresolvedRules);
        LHSave::Encode(S,After,SaveError); TestTrue(TEXT("Rejected reward leaves snapshot unchanged"),Before==After);
        TestTrue(TEXT("Catalog reward accepted by real settlement validation"),LHRewards::SettleKill(S,F,E.Reward,P,{})==ELHCommandReason::None);
    }
    return true;
}
LH_DATA_TEST(FLHDataItems,"ItemCatalogClosure")
{
    using namespace LHDataTestsPrivate;
    TSet<FName> Seen; for (const auto& I:LHItemData::Catalog()) { TestFalse(TEXT("Unique items"),Seen.Contains(I.Id.Value)); Seen.Add(I.Id.Value); }
    for (const auto& E:LHEnemyData::Catalog()) for (const auto& L:E.Reward.Loot) TestNotNull(TEXT("Loot resolves"),LHItemData::Find(L.Item));
    for (const auto& G:LHItemData::StartingKit()) TestNotNull(TEXT("Starting grant resolves"),LHItemData::Find(G.Item));
    for (const TCHAR* Offer:{TEXT("Item.RustedDirk"),TEXT("Item.AshwoodFlatbow"),TEXT("Item.WoodenArrows"),TEXT("Item.PotionOfMana")}) TestNotNull(TEXT("Offer resolves"),LHItemData::Find(Id(Offer)));
    const auto* Bow=LHItemData::Find(Id(TEXT("Item.AshwoodFlatbow"))); const auto* Q=LHItemData::Find(Id(TEXT("Item.WoodenArrows"))); const auto* Potion=LHItemData::Find(Id(TEXT("Item.PotionOfMana")));
    TestEqual(TEXT("Bow damage min"),Bow->Combat.Weapon.MinimumDamage.Value,1.); TestEqual(TEXT("Bow damage max"),Bow->Combat.Weapon.MaximumDamage.Value,3.); TestEqual(TEXT("Bow enc"),Bow->Encumbrance.Value,7.); TestEqual(TEXT("Bow price"),Bow->BuyGold.Value,int64(29));
    TestTrue(TEXT("Compatibility"),Bow->Combat.Weapon.CompatibleQuivers.ContainsByPredicate([&](const auto& I){return I.Value==Q->Id.Value;}));
    TestEqual(TEXT("Quiver +1"),Q->Combat.Weapon.QuiverDamageBonus.Value,1.); TestEqual(TEXT("Quiver enc"),Q->Encumbrance.Value,3.); TestEqual(TEXT("Quiver price"),Q->BuyGold.Value,int64(100)); TestTrue(TEXT("Unlimited"),Q->Combat.Weapon.QuiverConsumption==ELHQuiverConsumption::Unlimited);
    TestEqual(TEXT("Potion MP"),Potion->Combat.Consumable.ManaRestore.Value,25.); TestEqual(TEXT("Potion enc"),Potion->Encumbrance.Value,2.); TestEqual(TEXT("Potion price"),Potion->BuyGold.Value,int64(50));
    return true;
}
#endif
