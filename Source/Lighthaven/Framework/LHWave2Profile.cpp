#include "Framework/LHWave2Profile.h"
#include "Framework/LHWave2Closure.h"
#include "Persistence/LHSaveCodec.h"
#include "Rules/LHBibleRules.h"
#include "Data/Items/LHItemCatalog.h"
namespace LHWave2ProfilePrivate
{
using namespace LH::Rules;
FLHInteger I(int64 V)
{
    FLHInteger R;
    R.Resolution = ELHValueResolution::Resolved;
    R.Value = V;
    R.Provenance.Status = ELHProvenanceStatus::Prototype;
    R.Provenance.Notes = TEXT("R-03 missing starting grants/creation generator/runtime formula: authored Stage 1A Prototype; replace after evidence and balance review");
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
    P.Rules.Combat = MakeStage1PrototypeCombat();
    P.Rules.Requirements.BowRequiresQuiver = I(1);
    P.Reference.Id.Value = TEXT("Ruleset.Stage1Prototype");
    P.Reference.Revision = 4;
    P.Reference.HashAlgorithm = TEXT("SHA256");
    P.CreationPolicy.Value = TEXT("CreationPolicy.Stage1FiniteTable");
    P.CreationRevision = I(4);
    P.InitialHealth = N(30);
    P.InitialMana = N(10);
    P.InitialGold = I(100);
    P.InitialSkillPoints = I(0);
    P.InventorySlots = I(40); // Missing entry-slot limit: authored slice capacity.
    P.GrowthBasis = EAttributeBasis::Base;
    auto &C = P.Rules.Creation;
    C.Minimum = {I(10), I(10), I(10), I(10), I(10)};
    C.Maximum = {I(22), I(22), I(22), I(22), I(22)};
    C.TotalPoints = I(80);
    C.OutcomesResolution = ELHValueResolution::Resolved;
    C.OutcomesProvenance = I(0).Provenance;
    FCreationOutcome O;
    O.Answers = Answers();
    O.Attributes = {I(16), I(16), I(16), I(16), I(16)};
    O.UnspentPoints = I(0);
    C.Outcomes.Add(O); // Representative classic four-N/A chart; selection law is Prototype.
    P.Rules.Stats = {Linear(10), Linear(10), Linear(0), Linear(0), Linear(0)};
    P.Rules.Stats.Capacity.Constant.Provenance.Notes = TEXT("Unlimited carry capacity; deferred by owner decision (Matt, 2026-10-09); zero is an unused finite Rules placeholder, not a weight limit");
    auto &G = P.Rules.Progression;
    G.InitialLevel = I(1);
    // Runtime Rules rejects Disputed provenance; preserve the classic/PDF dispute in the note.
    for (auto X : MakeBibleRuleset().Experience)
    {
        X.Threshold.Provenance.Status = ELHProvenanceStatus::Modernized;
        X.Threshold.Provenance.Notes += TEXT("; Matt selects classic Required XPs; PDF disagreement retained in R-03");
        G.Thresholds.Add(X.Threshold);
    }
    G.HealthGrowth = Linear(7); G.HealthGrowth.Endurance = N(1.0/20);
    G.ManaGrowth = Linear(4); G.ManaGrowth.Intelligence = N(1.0/30); G.ManaGrowth.Wisdom = N(1.0/60);
    G.HealthRollScale = N(0); G.ManaRollScale = N(0);
    P.Rules.Requirements.Basis = EAttributeBasis::Base;
    P.Rules.Requirements.BasisProvenance = I(0).Provenance;
    for (const auto& Item : LHItemData::Catalog()) P.Items.Add(Item.Character);
    for (const auto& Grant : LHItemData::StartingKit())
        for (int64 Index=0; Index<Grant.Quantity.Value; ++Index) P.StarterItems.Add(Grant.Item);
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

LHWave2::FLHCarryCapacityPolicy LHWave2::CarryCapacityPolicy() { return {}; }
