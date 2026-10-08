#include "Misc/AutomationTest.h"
#include "Rules/LHBibleRules.h"
#if WITH_DEV_AUTOMATION_TESTS
using namespace LH::Rules;
namespace { constexpr auto BibleFlags=EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHBibleBands,"Lighthaven.Rules.Bible.BandBoundaries",BibleFlags)
bool FLHBibleBands::RunTest(const FString&)
{
    auto R=MakeBibleRuleset();
    TestEqual(TEXT("HP bands"),R.Health.Num(),16); TestEqual(TEXT("INT bands"),R.IntelligenceMana.Num(),20); TestEqual(TEXT("WIS bands"),R.WisdomMana.Num(),9);
    for (const auto* Bands:{&R.Health,&R.IntelligenceMana,&R.WisdomMana}) for (const auto& B:*Bands)
    {
        auto Low=LookupBibleBand(*Bands,B.Lower.Value), High=LookupBibleBand(*Bands,B.Upper.Value);
        TestTrue(TEXT("Lower accepted"),Low.Diagnostic.IsAccepted()); TestTrue(TEXT("Upper accepted"),High.Diagnostic.IsAccepted());
        TestEqual(TEXT("Lower output"),Low.Value.Minimum.Value,B.Minimum.Value); TestEqual(TEXT("Upper output"),High.Value.Maximum.Value,B.Maximum.Value);
    }
    TestEqual(TEXT("END19"),LookupBibleBand(R.Health,19).Value.Minimum.Value,int64(6));
    TestEqual(TEXT("END20"),LookupBibleBand(R.Health,20).Value.Minimum.Value,int64(7));
    TestEqual(TEXT("INT29"),LookupBibleBand(R.IntelligenceMana,29).Value.Minimum.Value,int64(3));
    TestEqual(TEXT("INT30"),LookupBibleBand(R.IntelligenceMana,30).Value.Minimum.Value,int64(4));
    TestEqual(TEXT("WIS59"),LookupBibleBand(R.WisdomMana,59).Value.Minimum.Value,int64(0));
    TestEqual(TEXT("WIS60"),LookupBibleBand(R.WisdomMana,60).Value.Minimum.Value,int64(1));
    TestTrue(TEXT("No extrapolation"),LookupBibleBand(R.Health,320).Diagnostic.Reason==EReason::Unresolved);
    R.Health[0].Minimum.Resolution=ELHValueResolution::Unresolved;
    TestTrue(TEXT("Missing rejects"),LookupBibleBand(R.Health,19).Diagnostic.Reason==EReason::Unresolved); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHBibleXPTest,"Lighthaven.Rules.Bible.XPLevelEdges",BibleFlags)
bool FLHBibleXPTest::RunTest(const FString&)
{
    auto R=MakeBibleRuleset(); TestEqual(TEXT("200 rows"),R.Experience.Num(),200);
    for (int64 Level=1;Level<=200;++Level) { auto X=LookupBibleXP(R,Level); TestTrue(TEXT("Each level accepted"),X.Diagnostic.IsAccepted()); TestEqual(TEXT("Explicit level"),X.Value.Level.Value,Level); }
    TestEqual(TEXT("Level1"),LookupBibleXP(R,1).Value.Threshold.Value,int64(0));
    TestEqual(TEXT("Level2"),LookupBibleXP(R,2).Value.Threshold.Value,int64(1000));
    auto X=LookupBibleXP(R,3); TestEqual(TEXT("Classic rounded threshold"),X.Value.Threshold.Value,int64(5700));
    TestTrue(TEXT("Disputed status retained"),X.Value.Threshold.Provenance.Status==ELHProvenanceStatus::Disputed);
    TestTrue(TEXT("201 unresolved, not capped"),LookupBibleXP(R,201).Diagnostic.Reason==EReason::Unresolved);
    TestFalse(TEXT("Level0 invalid"),LookupBibleXP(R,0).Diagnostic.IsAccepted()); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHBibleCreation,"Lighthaven.Rules.Bible.CreationChartAndCap",BibleFlags)
bool FLHBibleCreation::RunTest(const FString&)
{
    auto R=MakeBibleRuleset(); TestEqual(TEXT("65 chart rows"),R.Rolls.Num(),65); TestEqual(TEXT("40 answers"),R.Answers.Num(),40);
    TestTrue(TEXT("22 allowed"),ValidateBibleStatCap(R,{22,22,22,22,22}).Diagnostic.IsAccepted());
    for (FAttributes A:{FAttributes{23,1,1,1,1},FAttributes{1,23,1,1,1},FAttributes{1,1,23,1,1},FAttributes{1,1,1,23,1},FAttributes{1,1,1,1,23}}) TestFalse(TEXT("23 rejects each stat"),ValidateBibleStatCap(R,A).Diagnostic.IsAccepted());
    for (const auto& Row:R.Rolls) { const auto& A=Row.Attributes; TestTrue(TEXT("Chart cap"),ValidateBibleStatCap(R,{A.Strength.Value,A.Endurance.Value,A.Agility.Value,A.Intelligence.Value,A.Wisdom.Value}).Diagnostic.IsAccepted()); }
    auto Chart=LookupBibleRoll(R,{4,0,0,0,0}); TestTrue(TEXT("4STR chart found"),Chart.Diagnostic.IsAccepted()); TestEqual(TEXT("4STR strength"),Chart.Value.Attributes.Strength.Value,int64(22));
    TestTrue(TEXT("Combined mana unresolved"),BibleCombinedMana(R,30,60).Diagnostic.Reason==EReason::Unresolved);
    auto Owl=LookupBibleAnswer(R,TEXT("Dream"),TEXT("owl")); TestTrue(TEXT("Owl found"),Owl.Diagnostic.IsAccepted()); TestEqual(TEXT("Owl WIS"),Owl.Value.Affinity.Wisdom,int64(1));
    TestTrue(TEXT("RNG missing rejects"),BibleCost(R.CreationRNG).Diagnostic.Reason==EReason::Unresolved); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHBibleCapacity,"Lighthaven.Rules.Bible.Encumbrance",BibleFlags)
bool FLHBibleCapacity::RunTest(const FString&)
{
    auto R=MakeBibleRuleset();
    const int64 Strengths[]={0,25,50,100,200,400,610}; const double Expected[]={0,100,500.0/3,250,1000.0/3,400,305000.0/710};
    for (int32 I=0;I<7;++I) { auto V=BibleEncumbrance(R,Strengths[I]); TestTrue(TEXT("Capacity accepted"),V.Diagnostic.IsAccepted()); TestTrue(TEXT("Unrounded formula"),FMath::Abs(V.Value-Expected[I])<1e-9); }
    TestFalse(TEXT("Negative invalid"),BibleEncumbrance(R,-1).Diagnostic.IsAccepted()); R.CapacityScale.Resolution=ELHValueResolution::Unresolved;
    TestTrue(TEXT("Missing rejects"),BibleEncumbrance(R,25).Diagnostic.Reason==EReason::Unresolved); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHBibleRequirements,"Lighthaven.Rules.Bible.RequirementsAndCosts",BibleFlags)
bool FLHBibleRequirements::RunTest(const FString&)
{
    auto R=MakeBibleRuleset(); TestEqual(TEXT("Classic skills only"),R.Skills.Num(),16);
    const auto* Fire=R.Spells.FindByPredicate([](const auto& S){return S.Name==TEXT("Fire Dart");});
    const auto* Arrow=R.Spells.FindByPredicate([](const auto& S){return S.Name==TEXT("Flaming Arrow");});
    const auto* Stun=R.Skills.FindByPredicate([](const auto& S){return S.Name==TEXT("Stun Blow");});
    if (!Fire||!Arrow||!Stun) return false;
    FRequirementInput I; I.Level=2; I.Base={0,0,0,21,15};
    TestTrue(TEXT("Exact Fire Dart requirements"),CheckBibleRequirements(*Fire,EAttributeBasis::Base,I).Diagnostic.IsAccepted());
    I.Base.Intelligence=20; TestFalse(TEXT("Below INT"),CheckBibleRequirements(*Fire,EAttributeBasis::Base,I).Diagnostic.IsAccepted());
    TestTrue(TEXT("Missing stat basis rejects"),CheckBibleRequirements(*Fire,R.RequirementBasis,I).Diagnostic.Reason==EReason::Unresolved);
    I.Level=10; I.Base.Intelligence=44; TestFalse(TEXT("Missing prerequisite rejects"),CheckBibleRequirements(*Arrow,EAttributeBasis::Base,I).Diagnostic.IsAccepted());
    FLHContentId Id; Id.Value=TEXT("Fire Dart"); I.Spells.Add(Id); TestTrue(TEXT("Known prerequisite accepts"),CheckBibleRequirements(*Arrow,EAttributeBasis::Base,I).Diagnostic.IsAccepted());
    TestEqual(TEXT("Fire mana classic"),BibleCost(Fire->Mana).Value,int64(1)); TestEqual(TEXT("Fire gold"),BibleCost(Fire->Gold).Value,int64(532));
    I.Level=3; I.Base={25,0,20,0,0}; TestTrue(TEXT("Classic Stun AGI20"),CheckBibleRequirements(*Stun,EAttributeBasis::Base,I).Diagnostic.IsAccepted());
    TestTrue(TEXT("Skill costs need unit"),BibleSkillCost(*Stun,true).Diagnostic.Reason==EReason::Unresolved);
    auto Missing=*Fire; Missing.Level.Resolution=ELHValueResolution::Unresolved; TestTrue(TEXT("Missing requirement rejects"),CheckBibleRequirements(Missing,EAttributeBasis::Base,I).Diagnostic.Reason==EReason::Unresolved); return true;
}
#endif
