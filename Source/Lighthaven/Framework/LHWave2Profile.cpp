#include "Framework/LHWave2Profile.h"
#include "Framework/LHWave2Closure.h"
#include "Persistence/LHSaveCodec.h"
namespace LHWave2ProfilePrivate
{
using namespace LH::Rules;
FLHInteger I(int64 V)
{
    FLHInteger R;
    R.Resolution = ELHValueResolution::Resolved;
    R.Value = V;
    R.Provenance.Status = ELHProvenanceStatus::Prototype;
    R.Provenance.Notes = TEXT("W2-04 synthetic Prototype profile; no historical authenticity claim");
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
    P.Rules.Creation.AnswerCount = I(4);
    P.Rules.Progression.AttributePointsPerLevel = I(5);
    P.Rules.Progression.SkillPointsPerLevel = I(15);
    P.Rules.Requirements.BowRequiresQuiver = I(1);
    P.Reference.Id.Value = TEXT("Ruleset.Wave2Prototype");
    P.Reference.Revision = 1;

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
    P.StarterItems = {P.Items[0].Id, P.Items[1].Id};
    P.RequestDigest = LHSave::RequestDigest;
    P.GrowthId = LHSave::GrowthId;
    P.Reference.ContentHash = LHSave::Sha256(LHWave2::MechanicalClosure(P));
    return P;
}
}
FLHCharacterProfile LHWave2::PrototypeProfile() { static const FLHCharacterProfile P=LHWave2ProfilePrivate::Profile(); return P; }
FLHNumber LHWave2::PrototypeNumber(double V) { return LHWave2ProfilePrivate::N(V); }
FLHInteger LHWave2::PrototypeInteger(int64 V) { return LHWave2ProfilePrivate::I(V); }
TArray<FLHQuestionAnswer> LHWave2::PrototypeAnswers() { return LHWave2ProfilePrivate::Answers(); }
FString LHWave2::CatalogHash() { static const FString Hash=LHSave::Sha256(LHWave2::GameplayCatalogClosure()); return Hash; }
