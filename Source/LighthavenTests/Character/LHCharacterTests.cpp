#include "Misc/AutomationTest.h"
#include "Character/LHCharacterAuthority.h"
#include "Misc/SecureHash.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace LHCharacterTestsPrivate
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
    P.InitialGold = I(0);
    P.InitialSkillPoints = I(0);
    P.InventorySlots = I(2);
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
    Item.Id.Value = TEXT("Item.TestBow");
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
    Q.Value = TEXT("Item.TestQuiver");
    Item.CompatibleQuivers.Add(Q);
    P.Items.Add(Item);
    Item.Id = Q;
    Item.Slot = ELHEquipmentSlot::Quiver;
    Item.bBow = false;
    Item.CompatibleQuivers.Empty();
    Item.Modifier.Attributes.Strength = I(0);
    Item.Modifier.Accuracy = N(0);
    P.Items.Add(Item);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCharacterCreationTest, "Lighthaven.Character.CreationAndReplay", LHCharacterTestsPrivate::Flags)
bool FLHCharacterCreationTest::RunTest(const FString &)
{
    using namespace LH::Rules;
    FLHCharacterAuthority A;
    TestTrue(TEXT("Initialize"), A.Initialize(LHCharacterTestsPrivate::Profile(), LHCharacterTestsPrivate::Epoch(), 123));
    auto First = LHCharacterTestsPrivate::Create(A);
    auto Current = LHCharacterTestsPrivate::Create(A);
    TestTrue(TEXT("Old reroll token invalid"), A.Execute(First).Disposition == ELHCommandDisposition::Rejected);
    auto Forged = Current;
    Forged.Creation.AcceptedAttributes.Strength.Value++;
    TestTrue(TEXT("Caller roll forgery rejects"), A.Execute(Forged).Disposition == ELHCommandDisposition::Rejected);
    auto Result = A.Execute(Current);
    TestTrue(TEXT("Commit"), Result.Disposition == ELHCommandDisposition::Accepted);
    FLHSaveSnapshot Before;
    A.Export(Before);
    auto Replay = A.Execute(Current);
    FLHSaveSnapshot After;
    A.Export(After);
    TestTrue(TEXT("Idempotent receipt"),
             Replay.bReplay && Replay.CommittedSequence == Result.CommittedSequence && LHCharacterTestsPrivate::Equal(Before, After));
    auto Changed = Current;
    Changed.DisplayName = TEXT("Other");
    TestTrue(TEXT("Reused ID changed payload"), A.Execute(Changed).Reason == ELHCommandReason::ReusedRequestId);
    TestEqual(TEXT("Body expands bundled face"), A.Record().AppearanceIds.Num(), 5);
    TestEqual(TEXT("Creation retained"), A.Record().Creation.AcceptedAttributes.Strength.Value, int64(10));
    FLHCharacterAuthority Loaded;
    Loaded.Initialize(LHCharacterTestsPrivate::Profile(), LHCharacterTestsPrivate::Epoch(), 321);
    TestTrue(TEXT("Import"), Loaded.Import(Before) == ELHCommandReason::None);
    TestTrue(TEXT("Replay after load"), Loaded.Execute(Current).bReplay);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCharacterAtomicTest, "Lighthaven.Character.AtomicRejectionsAndCapacity", LHCharacterTestsPrivate::Flags)
bool FLHCharacterAtomicTest::RunTest(const FString &)
{
    using namespace LH::Rules;
    FLHCharacterAuthority A;
    A.Initialize(LHCharacterTestsPrivate::Profile(), LHCharacterTestsPrivate::Epoch(), 123);
    FLHSaveSnapshot Before;
    A.Export(Before);
    auto Bad = LHCharacterTestsPrivate::Answers();
    Bad[1].Question = Bad[0].Question;
    FLHCharacterPreview Preview;
    TestTrue(TEXT("Invalid answers"), A.Preview(Bad, Preview) != ELHCommandReason::None);
    FLHSaveSnapshot After;
    A.Export(After);
    TestTrue(TEXT("Invalid preview no state change"), LHCharacterTestsPrivate::Equal(Before, After));
    auto CreateRequest = LHCharacterTestsPrivate::Create(A);
    TestTrue(TEXT("Create"), A.Execute(CreateRequest).Disposition == ELHCommandDisposition::Accepted);
    FLHAllocateAttributePointsRequest R;
    R.Request = LHCharacterTestsPrivate::Request();
    R.Points = LHCharacterTestsPrivate::B(-1);
    A.Export(Before);
    TestTrue(TEXT("Negative allocation rejects"), A.Execute(R).Disposition == ELHCommandDisposition::Rejected);
    R.Request = LHCharacterTestsPrivate::Request();
    R.Points = LHCharacterTestsPrivate::B(11);
    TestTrue(TEXT("Excess pool rejects"), A.Execute(R).Reason == ELHCommandReason::InsufficientPoints);
    R.Request = LHCharacterTestsPrivate::Request();
    R.Points = LHCharacterTestsPrivate::B(1);
    R.Points.Wisdom.Resolution = ELHValueResolution::Unresolved;
    TestTrue(TEXT("Missing zero rejects"), A.Execute(R).Disposition == ELHCommandDisposition::Rejected);
    auto Unknown = LHCharacterTestsPrivate::Item(A, TEXT("Item.Unknown"));
    TestTrue(TEXT("Unknown definition"), A.AddItem(Unknown) == ELHCommandReason::UnresolvedRules);
    A.Export(After);
    TestTrue(TEXT("Rejected commands no partial mutation"), LHCharacterTestsPrivate::Equal(Before, After));
    R.Request = LHCharacterTestsPrivate::Request();
    R.Points = LHCharacterTestsPrivate::B(2);
    TestTrue(TEXT("Legal allocation"), A.Execute(R).Disposition == ELHCommandDisposition::Accepted);
    TestEqual(TEXT("Pool conservation"), A.Record().UnspentAttributePoints.Value, int64(8));
    TestEqual(TEXT("Accepted roll remains original"), A.Record().Creation.AcceptedAttributes.Strength.Value, int64(10));
    auto Bow = LHCharacterTestsPrivate::Item(A, TEXT("Item.TestBow"));
    auto Q = LHCharacterTestsPrivate::Item(A, TEXT("Item.TestQuiver"));
    TestTrue(TEXT("Add bow"), A.AddItem(Bow) == ELHCommandReason::None);
    TestTrue(TEXT("Add quiver"), A.AddItem(Q) == ELHCommandReason::None);
    A.Export(Before);
    TestTrue(TEXT("Full capacity"), A.AddItem(LHCharacterTestsPrivate::Item(A, TEXT("Item.TestQuiver"))) == ELHCommandReason::InventoryFull);
    A.Export(After);
    TestTrue(TEXT("Capacity atomic"), LHCharacterTestsPrivate::Equal(Before, After));
    TestTrue(TEXT("Remove"), A.RemoveItem(Q.Id, 1) == ELHCommandReason::None);
    TestTrue(TEXT("Add after removal"), A.AddItem(Q) == ELHCommandReason::None);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCharacterGrowthTest, "Lighthaven.Character.MultiLevelAndDebt", LHCharacterTestsPrivate::Flags)
bool FLHCharacterGrowthTest::RunTest(const FString &)
{
    using namespace LH::Rules;
    FLHCharacterAuthority A;
    A.Initialize(LHCharacterTestsPrivate::Profile(), LHCharacterTestsPrivate::Epoch(), 123);
    A.Execute(LHCharacterTestsPrivate::Create(A));
    TestTrue(TEXT("Multi-level"), A.GrantExperience(300) == ELHCommandReason::None);
    TestEqual(TEXT("Earned level"), A.Record().EarnedLevel.Value, int64(3));
    TestEqual(TEXT("One record each"), A.Record().GrowthAwards.Num(), 2);
    TestEqual(TEXT("Grants once"), A.Record().UnspentAttributePoints.Value, int64(20));
    TestEqual(TEXT("Skill grants"), A.Record().UnspentSkillPoints.Value, int64(30));
    if (A.Record().GrowthAwards.Num() != 2)
        return false;
    TestEqual(TEXT("Saved roll pair"), A.Record().GrowthAwards[0].RollInputs[0].State.Num(), 16);
    const double Health = A.Record().EarnedBaseHealth.Value;
    TestTrue(TEXT("Debt added"), A.AddExperienceDebt(30) == ELHCommandReason::None);
    TestTrue(TEXT("Partial recovery"), A.GrantExperience(20) == ELHCommandReason::None);
    TestEqual(TEXT("Retained balance"), A.Record().ExperienceBalance.Value, int64(300));
    TestEqual(TEXT("Debt remainder"), A.Record().ExperienceDebt.Value, int64(10));
    TestTrue(TEXT("Finish recovery"), A.GrantExperience(15) == ELHCommandReason::None);
    TestEqual(TEXT("Only excess XP retained"), A.Record().ExperienceBalance.Value, int64(305));
    TestEqual(TEXT("No recovered grants"), A.Record().GrowthAwards.Num(), 2);
    TestEqual(TEXT("Historical maxima unchanged"), A.Record().EarnedBaseHealth.Value, Health);
    TestTrue(TEXT("New threshold after recovery"), A.GrantExperience(295) == ELHCommandReason::None);
    TestEqual(TEXT("One new award"), A.Record().GrowthAwards.Num(), 3);
    FLHSaveSnapshot Before;
    A.Export(Before);
    TestTrue(TEXT("Overflow rejects"), A.GrantExperience(MAX_int64) == ELHCommandReason::InvalidRequest);
    FLHSaveSnapshot After;
    A.Export(After);
    TestTrue(TEXT("Overflow atomic"), LHCharacterTestsPrivate::Equal(Before, After));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCharacterEquipmentTest, "Lighthaven.Character.EquipmentQuiverAndSnapshot", LHCharacterTestsPrivate::Flags)
bool FLHCharacterEquipmentTest::RunTest(const FString &)
{
    using namespace LH::Rules;
    FLHCharacterAuthority A;
    A.Initialize(LHCharacterTestsPrivate::Profile(), LHCharacterTestsPrivate::Epoch(), 123);
    A.Execute(LHCharacterTestsPrivate::Create(A));
    auto Bow = LHCharacterTestsPrivate::Item(A, TEXT("Item.TestBow"));
    auto Q = LHCharacterTestsPrivate::Item(A, TEXT("Item.TestQuiver"));
    A.AddItem(Bow);
    A.AddItem(Q);
    const auto Base = A.Stats().Value;
    FLHSaveSnapshot Before;
    A.Export(Before);
    TestTrue(TEXT("Backpack quiver insufficient"),
             A.Execute(LHCharacterTestsPrivate::Equip(Bow, ELHEquipmentSlot::MainHand)).Reason == ELHCommandReason::Ineligible);
    FLHSaveSnapshot After;
    A.Export(After);
    TestTrue(TEXT("Failed equip atomic"), LHCharacterTestsPrivate::Equal(Before, After));
    TestTrue(TEXT("Quiver first class"),
             A.Execute(LHCharacterTestsPrivate::Equip(Q, ELHEquipmentSlot::Quiver)).Disposition == ELHCommandDisposition::Accepted);
    TestTrue(TEXT("Bow with equipped compatible quiver"),
             A.Execute(LHCharacterTestsPrivate::Equip(Bow, ELHEquipmentSlot::MainHand)).Disposition == ELHCommandDisposition::Accepted);
    TestEqual(TEXT("Canonical modifier applied once"), A.Stats().Value.Effective.Strength, int64(12));
    TestEqual(TEXT("Derived modifier"), A.Stats().Value.Accuracy, 4.0);
    TestTrue(TEXT("Cannot remove required quiver"),
             A.Execute(LHCharacterTestsPrivate::Equip(Q, ELHEquipmentSlot::Quiver, true)).Disposition == ELHCommandDisposition::Rejected);
    TestTrue(TEXT("Equipped inventory removal rejects"), A.RemoveItem(Q.Id, 1) == ELHCommandReason::InvalidEquipment);
    A.Export(Before);
    FLHCharacterAuthority Loaded;
    Loaded.Initialize(LHCharacterTestsPrivate::Profile(), LHCharacterTestsPrivate::Epoch(), 456);
    TestTrue(TEXT("Import equipped"), Loaded.Import(Before) == ELHCommandReason::None);
    Loaded.Export(After);
    TestTrue(TEXT("Equal full record roundtrip"), LHCharacterTestsPrivate::Equal(Before, After));
    TestEqual(TEXT("Derived rebuild"), Loaded.Stats().Value.Accuracy, 4.0);
    TestTrue(TEXT("Unequip bow"),
             A.Execute(LHCharacterTestsPrivate::Equip(Bow, ELHEquipmentSlot::MainHand, true)).Disposition == ELHCommandDisposition::Accepted);
    TestTrue(TEXT("Unequip quiver"),
             A.Execute(LHCharacterTestsPrivate::Equip(Q, ELHEquipmentSlot::Quiver, true)).Disposition == ELHCommandDisposition::Accepted);
    TestEqual(TEXT("Strength symmetry"), A.Stats().Value.Effective.Strength, Base.Effective.Strength);
    TestEqual(TEXT("Accuracy symmetry"), A.Stats().Value.Accuracy, Base.Accuracy);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCharacterUnresolvedTest, "Lighthaven.Character.UnresolvedAndImportValidation",
                                 LHCharacterTestsPrivate::Flags)
bool FLHCharacterUnresolvedTest::RunTest(const FString &)
{
    using namespace LH::Rules;
    auto P = LHCharacterTestsPrivate::Profile();
    P.InitialGold.Resolution = ELHValueResolution::Unresolved;
    FLHCharacterAuthority Missing;
    Missing.Initialize(P, LHCharacterTestsPrivate::Epoch(), 123);
    auto C = LHCharacterTestsPrivate::Create(Missing);
    FLHSaveSnapshot Before, After;
    Missing.Export(Before);
    TestTrue(TEXT("Missing starting gold rejects"), Missing.Execute(C).Reason == ELHCommandReason::UnresolvedRules);
    Missing.Export(After);
    TestTrue(TEXT("No partial creation"), LHCharacterTestsPrivate::Equal(Before, After));
    P = LHCharacterTestsPrivate::Profile();
    P.Items[1].Eligibility.MinimumAttributes.Wisdom.Resolution = ELHValueResolution::Unresolved;
    FLHCharacterAuthority A;
    A.Initialize(P, LHCharacterTestsPrivate::Epoch(), 123);
    A.Execute(LHCharacterTestsPrivate::Create(A));
    auto Q = LHCharacterTestsPrivate::Item(A, TEXT("Item.TestQuiver"));
    A.AddItem(Q);
    A.Export(Before);
    TestTrue(TEXT("Unresolved zero is not unrestricted"),
             A.Execute(LHCharacterTestsPrivate::Equip(Q, ELHEquipmentSlot::Quiver)).Reason == ELHCommandReason::UnresolvedRules);
    A.Export(After);
    TestTrue(TEXT("Requirements reject atomically"), LHCharacterTestsPrivate::Equal(Before, After));
    FLHCharacterAuthority B;
    B.Initialize(LHCharacterTestsPrivate::Profile(), LHCharacterTestsPrivate::Epoch(), 123);
    B.Execute(LHCharacterTestsPrivate::Create(B));
    B.GrantExperience(300);
    B.Export(Before);
    FLHCharacterAuthority Loaded;
    Loaded.Initialize(LHCharacterTestsPrivate::Profile(), LHCharacterTestsPrivate::Epoch(), 123);
    TestTrue(TEXT("Valid history"), Loaded.Import(Before) == ELHCommandReason::None);
    Loaded.Export(After);
    auto Corrupt = Before;
    Corrupt.Character.GrowthAwards[1].AwardId = Corrupt.Character.GrowthAwards[0].AwardId;
    TestTrue(TEXT("Repeated award ID rejects"), Loaded.Import(Corrupt) == ELHCommandReason::InvalidRequest);
    Corrupt = Before;
    Corrupt.Character.EarnedBaseHealth.Value++;
    TestTrue(TEXT("Historical maximum mismatch rejects"), Loaded.Import(Corrupt) == ELHCommandReason::InvalidRequest);
    Corrupt = Before;
    Corrupt.Character.BaseAttributes.Strength.Value++;
    TestTrue(TEXT("Allocation conservation rejects"), Loaded.Import(Corrupt) == ELHCommandReason::InvalidRequest);
    Corrupt = Before;
    Corrupt.Character.Inventory.Add(LHCharacterTestsPrivate::Item(B, TEXT("Item.TestQuiver")));
    Corrupt.Character.Inventory.Add(Corrupt.Character.Inventory[0]);
    TestTrue(TEXT("Duplicate instance rejects"), Loaded.Import(Corrupt) == ELHCommandReason::InvalidRequest);
    FLHSaveSnapshot Unchanged;
    Loaded.Export(Unchanged);
    TestTrue(TEXT("Bad loads retain previous state"), LHCharacterTestsPrivate::Equal(After, Unchanged));
    TestTrue(TEXT("Continue original"), B.GrantExperience(300) == ELHCommandReason::None);
    TestTrue(TEXT("Continue restored"), Loaded.GrantExperience(300) == ELHCommandReason::None);
    B.Export(Before);
    Loaded.Export(After);
    TestTrue(TEXT("RNG/history continuation equal"), LHCharacterTestsPrivate::Equal(Before, After));
    return true;
}
#endif
