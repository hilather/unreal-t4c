#include "Misc/AutomationTest.h"
#include "Character/LHCharacterAuthority.h"
#include "Misc/SecureHash.h"
#include "Services/LHServiceAuthority.h"
#include "Persistence/LHSaveCodec.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace LHServiceTestsPrivate
{
using namespace LH::Rules;
constexpr auto Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
FLHInteger I(int64 V)
{
    FLHInteger R;
    R.Resolution = ELHValueResolution::Resolved;
    R.Value = V;
    R.Provenance.Status = ELHProvenanceStatus::Prototype;
    R.Provenance.Notes = TEXT("W2-02 synthetic native test only; not runtime tuning");
    return R;
}
FLHNumber N(double V)
{
    FLHNumber R;
    R.Resolution = ELHValueResolution::Resolved;
    R.Value = V;
    R.Provenance = I(0).Provenance;
    return R;
}
FLHAttributeBlock B(int64 S = 0, int64 E = 0, int64 A = 0, int64 Int = 0, int64 W = 0)
{
    FLHAttributeBlock R;
    R.Strength = I(S);
    R.Endurance = I(E);
    R.Agility = I(A);
    R.Intelligence = I(Int);
    R.Wisdom = I(W);
    return R;
}
FLinearFormula Linear(double C)
{
    return {N(C), N(0), N(0), N(0), N(0), N(0)};
}
TArray<FLHQuestionAnswer> Answers()
{
    TArray<FLHQuestionAnswer> R;
    for (int32 Index = 0; Index < 4; ++Index)
    {
        FLHQuestionAnswer A;
        A.Question.Value = FName(*FString::Printf(TEXT("Question.Test%d"), Index));
        A.Answer.Value = TEXT("Answer.Test");
        R.Add(A);
    }
    return R;
}
FLHCharacterProfile Profile()
{
    FLHCharacterProfile P;
    P.Rules = MakeLedgerPrototypeRuleset();
    P.Reference.Id.Value = TEXT("Ruleset.CharacterTest");
    P.Reference.Revision = 1;
    P.Reference.ContentHash = FString::ChrN(64, 'a');
    P.Reference.HashAlgorithm = TEXT("SHA256");
    P.CreationPolicy.Value = TEXT("CreationPolicy.SyntheticTable");
    P.CreationRevision = I(1);
    P.InitialHealth = N(20);
    P.InitialMana = N(10);
    P.InitialGold = I(10000);
    P.InitialSkillPoints = I(100);
    P.InventorySlots = I(20);
    P.GrowthBasis = EAttributeBasis::Base;
    auto &C = P.Rules.Creation;
    C.Minimum = {I(10), I(10), I(10), I(10), I(10)};
    C.Maximum = {I(20), I(20), I(20), I(20), I(20)};
    C.TotalPoints = I(60);
    C.OutcomesResolution = ELHValueResolution::Resolved;
    C.OutcomesProvenance = I(0).Provenance;
    FCreationOutcome O;
    O.Answers = Answers();
    O.Attributes = C.Minimum;
    O.UnspentPoints = I(10);
    C.Outcomes.Add(O);
    P.Rules.Stats = {Linear(1), Linear(1), Linear(1), Linear(1), Linear(100)};
    auto &G = P.Rules.Progression;
    G.InitialLevel = I(1);
    G.Thresholds = {I(0), I(100), I(300), I(600)};
    G.HealthGrowth = Linear(2);
    G.ManaGrowth = Linear(1);
    G.HealthRollScale = N(2);
    G.ManaRollScale = N(2);
    P.Rules.Requirements.Basis = EAttributeBasis::Base;
    P.Rules.Requirements.BasisProvenance = I(0).Provenance;
    FLHCharacterItemDefinition Item;
    Item.Id.Value = TEXT("Item.AshwoodFlatbow");
    Item.Slot = ELHEquipmentSlot::MainHand;
    Item.StackLimit = I(1);
    Item.Weight = N(1);
    Item.Eligibility.MinimumAttributes = B();
    Item.Eligibility.MinimumLevel = I(0);
    Item.Modifier.Attributes = {I(2), I(0), I(0), I(0), I(0)};
    Item.Modifier.Health = N(0);
    Item.Modifier.Mana = N(0);
    Item.Modifier.Accuracy = N(3);
    Item.Modifier.Avoidance = N(0);
    Item.Modifier.DamageBonus = N(0);
    Item.Modifier.Armor = N(0);
    Item.Modifier.Capacity = N(0);
    Item.bBow = true;
    FLHContentId Q;
    Q.Value = TEXT("Item.WoodenArrows");
    Item.CompatibleQuivers.Add(Q);
    P.Items.Add(Item);
    Item.Id = Q;
    Item.Slot = ELHEquipmentSlot::Quiver;
    Item.bBow = false;
    Item.CompatibleQuivers.Empty();
    Item.Modifier.Attributes.Strength = I(0);
    Item.Modifier.Accuracy = N(0);
    P.Items.Add(Item);
    Item.Id.Value=TEXT("Item.RustedDirk"); Item.Slot=ELHEquipmentSlot::MainHand; P.Items.Add(Item);
    Item.Id.Value=TEXT("Item.PotionOfMana"); Item.Slot=ELHEquipmentSlot::Unspecified; P.Items.Add(Item);
    // Test-only reflection fingerprint. Production must inject the shared D04 canonical SHA256 codec.
    P.RequestDigest = [](FName Kind, const UScriptStruct *Type, const void *Value) {
        FString Text;
        Type->ExportText(Text, Value, nullptr, nullptr, PPF_None, nullptr);
        Text = Kind.ToString() + Text;
        FTCHARToUTF8 UTF8(*Text);
        uint8 Hash[20];
        FSHA1::HashBuffer(UTF8.Get(), UTF8.Length(), Hash);
        FString Digest = BytesToHex(Hash, 20).ToLower();
        return Digest + Digest.Left(24);
    };
    // Test-only deterministic stand-in; production must inject D06 SHA256 mapping.
    P.GrowthId = [](const FGuid &Run, const FLHCharacterId &Character, int64 Level) {
        FLHRewardId R;
        R.Value = FGuid(Run.A, Character.Value.B, Character.Value.C, uint32(Level));
        return R;
    };
    return P;
}
FGuid Epoch()
{
    return FGuid(1, 2, 3, 4);
}
FLHRequestId Request()
{
    FLHRequestId R;
    R.Epoch = Epoch();
    R.Value = FGuid::NewGuid();
    return R;
}
FLHCreateCharacterRequest Create(FLHCharacterAuthority &A)
{
    FLHCharacterPreview V;
    A.Preview(Answers(), V);
    FLHCreateCharacterRequest R;
    R.Request = Request();
    R.DisplayName = TEXT("Test human");
    R.Creation = V.Creation;
    R.PreviewToken = V.Token;
    for (const TCHAR *S : {TEXT("Presentation.Player.Body.A"), TEXT("Presentation.Player.Hair.Cropped"),
                           TEXT("Presentation.Player.Skin.LightWarm"), TEXT("Presentation.Player.Outfit.StarterLinen")})
    {
        FLHContentId ID;
        ID.Value = S;
        R.AppearanceIds.Add(ID);
    }
    return R;
}
FLHItemInstance Item(const FLHCharacterAuthority &A, FName Definition)
{
    FLHSaveSnapshot S;
    A.Export(S);
    FLHItemInstance R;
    R.Id.RunId = S.World.RunId;
    R.Id.Area.Content.Value = TEXT("Area.LighthavenTempleDistrict");
    R.Id.InstanceId = FGuid::NewGuid();
    R.Definition.Value = Definition;
    R.Quantity = I(1);
    return R;
}
FLHEquipItemRequest Equip(const FLHItemInstance &I, ELHEquipmentSlot Slot, bool Undo = false)
{
    FLHEquipItemRequest R;
    R.Request = Request();
    R.Item = I.Id;
    R.Slot = Slot;
    R.bUnequip = Undo;
    return R;
}
bool Equal(const FLHSaveSnapshot &A, const FLHSaveSnapshot &B)
{
    return FLHSaveSnapshot::StaticStruct()->CompareScriptStruct(&A, &B, 0);
}
} // namespace

namespace LHServiceTestsPrivate
{
FLHContentId Id(const TCHAR* S) { FLHContentId I; I.Value=S; return I; }
struct Fixture
{
    FLHCharacterProfile P=Profile();
    FLHSaveSnapshot S;
    FLHServiceContext C;
    Fixture(const TCHAR* Npc=TEXT("NPC.Sigfried"))
    {
        FLHCharacterAuthority A; A.Initialize(P,Epoch(),123); A.Execute(Create(A)); A.Export(S);
        C.Profile=&P; C.Npc=Id(Npc); C.Entity.RunId=S.World.RunId; C.Entity.Area.Content.Value=TEXT("Area.LighthavenTempleDistrict"); C.Entity.InstanceId=FGuid::NewGuid(); C.bLineOfSight=true; C.DistanceCm=100;
    }
    FLHBuyItemRequest Buy(const TCHAR* Offer=TEXT("Offer.Sigfried.Item.AshwoodFlatbow")) { FLHBuyItemRequest R; R.Request=Request(); R.Vendor=C.Entity; R.Offer=Id(Offer); R.Quantity=I(1); return R; }
    FLHTrainSkillRequest Train(int64 Count) { FLHTrainSkillRequest R; R.Request=Request(); R.Trainer=C.Entity; R.Skill=Id(TEXT("Skill.Attack")); R.Points=I(Count); return R; }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHServiceTrainTest,"Lighthaven.Services.TrainSkillSuccessAndPool",LHServiceTestsPrivate::Flags)
bool FLHServiceTrainTest::RunTest(const FString&)
{
    using namespace LHServiceTestsPrivate; Fixture F(TEXT("NPC.Murmuntag")); auto R=F.Train(3);
    TestEqual(TEXT("Train"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::None);
    TestEqual(TEXT("Gold once"),F.S.Character.Gold.Value,int64(9970)); TestEqual(TEXT("Pool"),F.S.Character.UnspentSkillPoints.Value,int64(97));
    TestEqual(TEXT("Ranks"),F.S.Character.LearnedSkills[0].TrainedValue.Value,int64(3));
    auto Before=F.S; TestEqual(TEXT("Cap"),LHServices::Execute(F.C,F.S,F.Train(98)),ELHCommandReason::Ineligible); TestTrue(TEXT("Intact"),Equal(Before,F.S));
    F.S.Character.UnspentSkillPoints=I(0); Before=F.S; TestEqual(TEXT("Pool exhausted"),LHServices::Execute(F.C,F.S,F.Train(1)),ELHCommandReason::InsufficientPoints); TestTrue(TEXT("Pool rejection intact"),Equal(Before,F.S));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHServiceLearnTest,"Lighthaven.Services.LearnSpellPrerequisites",LHServiceTestsPrivate::Flags)
bool FLHServiceLearnTest::RunTest(const FString&)
{
    using namespace LHServiceTestsPrivate; Fixture F(TEXT("NPC.Kilhiam")); FLHLearnSpellRequest R; R.Request=Request(); R.Trainer=F.C.Entity; R.Spell=Id(TEXT("Spell.Light"));
    auto Before=F.S; TestEqual(TEXT("Level fails"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::Ineligible); TestTrue(TEXT("Intact"),Equal(Before,F.S));
    FLHCharacterAuthority A; A.Initialize(F.P,Epoch(),123); A.Import(F.S); A.GrantExperience(100); A.Export(F.S);
    TestEqual(TEXT("Attributes fail"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::Ineligible);
    FLHAllocateAttributePointsRequest Alloc; Alloc.Request=Request(); Alloc.Points=B(0,0,0,8,5); TestEqual(TEXT("Allocate"),A.Execute(Alloc).Reason,ELHCommandReason::None); A.Export(F.S);
    auto Modified=*LHAbilities::Find(R.Spell); Modified.Eligibility.RequiredSpells.Add(Id(TEXT("Spell.FireDart"))); F.C.AbilityLookup=[&](const FLHContentId&){return &Modified;};
    TestEqual(TEXT("Prior spell fails"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::Ineligible); F.C.AbilityLookup=LHAbilities::Find;
    const int64 Gold=F.S.Character.Gold.Value,Points=F.S.Character.UnspentSkillPoints.Value; const double Mana=F.S.Character.CurrentMana.Value;
    TestEqual(TEXT("Learn"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::None); TestEqual(TEXT("Learning gold"),F.S.Character.Gold.Value,Gold-233); TestEqual(TEXT("Learning points"),F.S.Character.UnspentSkillPoints.Value,Points-5);
    TestEqual(TEXT("Mana unchanged"),F.S.Character.CurrentMana.Value,Mana);
    TestEqual(TEXT("Relearn"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::Ineligible); TestEqual(TEXT("Once"),F.S.Character.LearnedSpells.Num(),1); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHServiceBuyTest,"Lighthaven.Services.BuyBowAndQuiver",LHServiceTestsPrivate::Flags)
bool FLHServiceBuyTest::RunTest(const FString&)
{
    using namespace LHServiceTestsPrivate; Fixture F;
    TestEqual(TEXT("Bow"),LHServices::Execute(F.C,F.S,F.Buy()),ELHCommandReason::None);
    TestEqual(TEXT("Quiver"),LHServices::Execute(F.C,F.S,F.Buy(TEXT("Offer.Sigfried.Item.WoodenArrows"))),ELHCommandReason::None); TestEqual(TEXT("Prices"),F.S.Character.Gold.Value,int64(9871));
    FLHCharacterAuthority A; A.Initialize(F.P,Epoch(),123); TestEqual(TEXT("Import"),A.Import(F.S),ELHCommandReason::None);
    TestEqual(TEXT("Equip quiver"),A.Execute(Equip(F.S.Character.Inventory[1],ELHEquipmentSlot::Quiver)).Reason,ELHCommandReason::None);
    TestEqual(TEXT("Equip bow"),A.Execute(Equip(F.S.Character.Inventory[0],ELHEquipmentSlot::MainHand)).Reason,ELHCommandReason::None);
    FLHSaveSnapshot Equipped; A.Export(Equipped); FLHCombatItemData Bow,Quiver; Bow.bWeapon=true; Bow.Weapon.bBow=true; Bow.Weapon.MinimumDamage=N(1); Bow.Weapon.MaximumDamage=N(3); Bow.Weapon.RangeCm=N(1200); Bow.Weapon.CompatibleQuivers.Add(Id(TEXT("Item.WoodenArrows"))); Quiver.bQuiver=true; Quiver.Weapon.QuiverDamageBonus=N(1);
    FLHCombatItemLookup Lookup=[&](const FLHContentId& K)->const FLHCombatItemData* { return K.Value==TEXT("Item.AshwoodFlatbow")?&Bow:K.Value==TEXT("Item.WoodenArrows")?&Quiver:nullptr; };
    FLHBasicAttackConfig Config; FString Error; TestTrue(TEXT("Bought gear builds ranged config"),LHAbilities::BuildAttackConfig(Equipped,Id(TEXT("Attack.Ranged.Bow")),Lookup,MakeStage1PrototypeCombat(),Config,Error)); TestEqual(TEXT("Unlimited quiver policy"),Quiver.Weapon.QuiverConsumption,ELHQuiverConsumption::Unlimited); TestEqual(TEXT("Quiver bonus"),Config.QuiverBonus,1.0);
    F.S.Character.Gold=I(0); auto Before=F.S; TestEqual(TEXT("No gold"),LHServices::Execute(F.C,F.S,F.Buy()),ELHCommandReason::InsufficientGold); TestTrue(TEXT("Intact"),Equal(Before,F.S)); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHServiceFullTest,"Lighthaven.Services.FullInventoryRejectsIntact",LHServiceTestsPrivate::Flags)
bool FLHServiceFullTest::RunTest(const FString&)
{
    using namespace LHServiceTestsPrivate; Fixture F; F.P.InventorySlots=I(0); auto Before=F.S;
    TestEqual(TEXT("Full"),LHServices::Execute(F.C,F.S,F.Buy()),ELHCommandReason::InventoryFull); TestTrue(TEXT("Intact"),Equal(Before,F.S)); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHServiceReplayTest,"Lighthaven.Services.DoubleClickIdempotent",LHServiceTestsPrivate::Flags)
bool FLHServiceReplayTest::RunTest(const FString&)
{
    using namespace LHServiceTestsPrivate; Fixture F; auto R=F.Buy(); FLHCommandResult Out; FString Digest;
    TestTrue(TEXT("Begin"),LHSave::BeginRequest(F.S,TEXT("BuyItem"),FLHBuyItemRequest::StaticStruct(),&R,Out,Digest));
    TestEqual(TEXT("Purchase"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::None); LHSave::CommitRequest(F.S,R.Request,Digest,Out); const auto Before=F.S;
    TestFalse(TEXT("Replay skips domain"),LHSave::BeginRequest(F.S,TEXT("BuyItem"),FLHBuyItemRequest::StaticStruct(),&R,Out,Digest)); TestTrue(TEXT("Replay"),Out.bReplay); TestTrue(TEXT("Intact"),Equal(Before,F.S));
    R.Request=Request(); TestTrue(TEXT("New request"),LHSave::BeginRequest(F.S,TEXT("BuyItem"),FLHBuyItemRequest::StaticStruct(),&R,Out,Digest)); TestEqual(TEXT("New purchase"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::None); LHSave::CommitRequest(F.S,R.Request,Digest,Out); TestEqual(TEXT("Two items"),F.S.Character.Inventory.Num(),2);
    Fixture T(TEXT("NPC.Murmuntag")); auto Train=T.Train(2);
    TestTrue(TEXT("Train begin"),LHSave::BeginRequest(T.S,TEXT("TrainSkill"),FLHTrainSkillRequest::StaticStruct(),&Train,Out,Digest));
    TestEqual(TEXT("Train purchase"),LHServices::Execute(T.C,T.S,Train),ELHCommandReason::None); LHSave::CommitRequest(T.S,Train.Request,Digest,Out); const auto Trained=T.S;
    TestFalse(TEXT("Train replay skips"),LHSave::BeginRequest(T.S,TEXT("TrainSkill"),FLHTrainSkillRequest::StaticStruct(),&Train,Out,Digest)); TestTrue(TEXT("Train replay flag"),Out.bReplay); TestTrue(TEXT("Train once"),Equal(Trained,T.S));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHServiceRangeTest,"Lighthaven.Services.WrongNpcOrRange",LHServiceTestsPrivate::Flags)
bool FLHServiceRangeTest::RunTest(const FString&)
{
    using namespace LHServiceTestsPrivate; Fixture F; auto R=F.Buy(); auto Before=F.S; F.C.Npc=Id(TEXT("NPC.Kilhiam")); TestEqual(TEXT("Wrong NPC"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::NotFound);
    F.C.Npc=Id(TEXT("NPC.Sigfried")); F.C.DistanceCm=251; TestEqual(TEXT("Range"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::OutOfRange); F.C.DistanceCm=250; F.C.bLineOfSight=false; TestEqual(TEXT("LOS"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::Obstructed); TestTrue(TEXT("Intact"),Equal(Before,F.S));
    F.C.bLineOfSight=true; F.C.bBusy=true; TestEqual(TEXT("Busy"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::Busy); F.C.bBusy=false;
    R.Vendor.InstanceId=FGuid::NewGuid(); TestEqual(TEXT("Wrong entity"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::NotFound); R.Vendor=F.C.Entity;
    F.S.Character.CurrentHealth=N(0); TestEqual(TEXT("Dead"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::InvalidLifeState); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHServiceSellTest,"Lighthaven.Services.SellRules",LHServiceTestsPrivate::Flags)
bool FLHServiceSellTest::RunTest(const FString&)
{
    using namespace LHServiceTestsPrivate; Fixture F; FLHItemInstance I0; I0.Id=F.C.Entity; I0.Id.InstanceId=FGuid::NewGuid(); I0.Definition=Id(TEXT("Item.RustedDirk")); I0.Quantity=I(1); F.S.Character.Inventory.Add(I0);
    FLHSellItemRequest R; R.Request=Request(); R.Vendor=F.C.Entity; R.Item=I0.Id; R.Quantity=I(1);
    FLHEquipmentBinding E; E.Slot=ELHEquipmentSlot::MainHand; E.Item=I0.Id; F.S.Character.Equipment.Add(E); auto Before=F.S;
    TestEqual(TEXT("Equipped"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::InvalidEquipment); TestTrue(TEXT("Intact"),Equal(Before,F.S)); F.S.Character.Equipment.Empty();
    F.S.Character.Inventory[0].Definition=Id(TEXT("Item.WoodenArrows")); TestEqual(TEXT("Unresolved"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::UnresolvedRules); F.S.Character.Inventory[0].Definition=I0.Definition;
    F.S.Character.Gold=I(MAX_int64); Before=F.S; TestEqual(TEXT("Overflow"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::InvalidRequest); TestTrue(TEXT("Overflow intact"),Equal(Before,F.S)); F.S.Character.Gold=I(0);
    TestEqual(TEXT("Sell"),LHServices::Execute(F.C,F.S,R),ELHCommandReason::None); TestEqual(TEXT("Price"),F.S.Character.Gold.Value,int64(9)); TestEqual(TEXT("Removed"),F.S.Character.Inventory.Num(),0); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHServiceCatalogTest,"Lighthaven.Services.CatalogSourcePositions",LHServiceTestsPrivate::Flags)
bool FLHServiceCatalogTest::RunTest(const FString&)
{
    using namespace LHServiceTestsPrivate; TestEqual(TEXT("Offer count"),LHServices::Catalog().Num(),13);
    for(const auto& O:LHServices::Catalog()) { TestTrue(TEXT("Offer namespace"),O.Id.Value.ToString().StartsWith(TEXT("Offer."))); TestTrue(TEXT("Paid"),O.Gold.Value>0); const FName Expected=O.Npc.Value==TEXT("NPC.Shovanis")?FName(TEXT("B1")):(O.Npc.Value==TEXT("NPC.Iraltok") || O.Npc.Value==TEXT("NPC.Uranos"))?FName(TEXT("MageTower")):FName(TEXT("TempleDistrict")); TestEqual(TEXT("Documented area"),O.Area,Expected); }
    TestEqual(TEXT("No Rolph invention"),LHServices::Offers(Id(TEXT("NPC.Rolph")),Fixture().S).Num(),0); return true;
}
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHServicePolicyTest,"Lighthaven.Services.AuthoredPolicyReadiness",LHServiceTestsPrivate::Flags)
bool FLHServicePolicyTest::RunTest(const FString&)
{
    using namespace LHServiceTestsPrivate;
    Fixture F;
    for(auto& D:F.P.Items) { D.StackLimit.Provenance.Status=ELHProvenanceStatus::Modernized; D.Weight=N(1000000); }
    F.P.InventorySlots.Provenance.Status=ELHProvenanceStatus::Modernized;
    TestEqual(TEXT("Authored stack/slots and weight above legacy capacity"),LHServices::Execute(F.C,F.S,F.Buy()),ELHCommandReason::None);
    for(auto Status:{ELHProvenanceStatus::Missing,ELHProvenanceStatus::Disputed}) {
        for(auto& D:F.P.Items) D.StackLimit.Provenance.Status=Status;
        const auto Before=F.S;
        TestEqual(TEXT("Unresolved policy rejected"),LHServices::Execute(F.C,F.S,F.Buy()),ELHCommandReason::UnresolvedRules);
        TestTrue(TEXT("No charge or mutation"),Equal(Before,F.S));
    }
    for(auto& D:F.P.Items) D.StackLimit.Provenance.Status=ELHProvenanceStatus::Modernized;
    F.S.Character.Gold.Provenance.Status=ELHProvenanceStatus::Modernized;
    TestEqual(TEXT("Authored policy cannot authorize a balance"),LHServices::Execute(F.C,F.S,F.Buy()),ELHCommandReason::UnresolvedRules);
    F.S.Character.Gold.Provenance.Status=ELHProvenanceStatus::Prototype;
    for(auto& D:F.P.Items) D.StackLimit.Resolution=ELHValueResolution::Unresolved;
    TestEqual(TEXT("Null policy rejected"),LHServices::Execute(F.C,F.S,F.Buy()),ELHCommandReason::UnresolvedRules);
    return true;
}
