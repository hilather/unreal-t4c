// W1-02 — NOT YET COMPILED (UE 5.8.3 pending)
#include "Misc/AutomationTest.h"
#include "Rules/LHRules.h"
#include <limits>

#if WITH_DEV_AUTOMATION_TESTS
// Keep helpers out of the unity translation unit's anonymous/global namespace:
// engine headers included by later test files must not see generic test names.
namespace LHRulesTestsPrivate
{
using namespace LH::Rules;
// Synthetic test coefficients are NOT ledger proposals, runtime defaults or authentic T4C values.
FLHFieldProvenance Synthetic(const TCHAR* Row)
{
    FLHFieldProvenance P; P.Status=ELHProvenanceStatus::Prototype;
    P.SourceBaseline=TEXT("W1-02 synthetic native test only"); P.RetrievedDate=TEXT("2026-10-07"); P.Notes=Row; return P;
}
FLHInteger Int(int64 V)
{
    FLHInteger P; P.Value=V; P.Resolution=ELHValueResolution::Resolved; P.Provenance=Synthetic(TEXT("Synthetic test parameter")); return P;
}
FLHNumber Num(double V)
{
    FLHNumber P; P.Value=V; P.Resolution=ELHValueResolution::Resolved; P.Provenance=Synthetic(TEXT("Synthetic test parameter")); return P;
}
FAttributeParameters Attributes(int64 S,int64 E,int64 A,int64 I,int64 W)
{
    return {Int(S),Int(E),Int(A),Int(I),Int(W)};
}
FLHAttributeBlock Minimum(int64 S,int64 E,int64 A,int64 I,int64 W)
{
    FLHAttributeBlock P; P.Strength=Int(S); P.Endurance=Int(E); P.Agility=Int(A); P.Intelligence=Int(I); P.Wisdom=Int(W); return P;
}
FLinearFormula Linear(double C,double S=0,double E=0,double A=0,double I=0,double W=0)
{
    return {Num(C),Num(S),Num(E),Num(A),Num(I),Num(W)};
}
TArray<FLHQuestionAnswer> Answers()
{
    TArray<FLHQuestionAnswer> Out;
    const TCHAR* Questions[]={TEXT("Question.One"),TEXT("Question.Two"),TEXT("Question.Three"),TEXT("Question.Four")};
    for (const TCHAR* Q : Questions) { FLHQuestionAnswer A; A.Question.Value=FName(Q); A.Answer.Value=TEXT("Answer.Synthetic"); Out.Add(A); }
    return Out;
}
FCreationParameters Creation()
{
    auto P=MakeLedgerPrototypeRuleset().Creation;
    P.Minimum=Attributes(10,10,10,10,10); P.Maximum=Attributes(20,20,20,20,20); P.TotalPoints=Int(60);
    P.OutcomesResolution=ELHValueResolution::Resolved; P.OutcomesProvenance=Synthetic(TEXT("Creation RNG"));
    FCreationOutcome A; A.Answers=Answers(); A.Attributes=Attributes(10,10,10,10,10); A.UnspentPoints=Int(10); P.Outcomes.Add(A);
    A.Attributes=Attributes(20,10,10,10,10); A.UnspentPoints=Int(0); P.Outcomes.Add(A); return P;
}
FStatsParameters Stats()
{
    return {Linear(0,0,0,1),Linear(0,0,0,0.5),Linear(0,0.25),Linear(0,0,0.5),Linear(0,2)};
}
FProgressionParameters Progression()
{
    auto P=MakeLedgerPrototypeRuleset().Progression;
    P.InitialLevel=Int(1); P.Thresholds={Int(0),Int(100),Int(300),Int(600)};
    P.HealthGrowth=Linear(1,0,0.5); P.ManaGrowth=Linear(0,0,0,0,0.25,0.25);
    P.HealthRollScale=Num(2); P.ManaRollScale=Num(2); return P;
}
FGrowthRoll Growth(int32 Id,double H=0,double M=0)
{
    FGrowthRoll R; R.Health=H; R.Mana=M; R.AwardId.Value=FGuid(1,2,3,Id); return R;
}
FProgressionInput ProgressionInput()
{
    FProgressionInput I; I.EarnedLevel=1; I.GrowthAttributes={10,19,10,10,10}; I.EarnedHealth=20; I.EarnedMana=10;
    I.Ruleset.Id.Value=TEXT("Ruleset.SyntheticTestOnly"); I.Ruleset.Revision=1; I.Ruleset.ContentHash=TEXT("synthetic-test-only"); return I;
}
FRequirementPolicy Requirements()
{
    auto P=MakeLedgerPrototypeRuleset().Requirements; P.Basis=EAttributeBasis::Base;
    P.BasisProvenance=Synthetic(TEXT("Requirement evaluation")); return P;
}
FCombatParameters Combat()
{
    return {Num(0.5),Num(0.01),Num(0.01),Num(0.1),Num(0.9),Num(1),Num(1),Num(0),Num(1)};
}
constexpr auto LHRulesTestsFlags=EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCreationTests,"Lighthaven.Rules.Prototype.Synthetic.CreationRNGAndPointConservation",LHRulesTestsPrivate::LHRulesTestsFlags)
bool FLHCreationTests::RunTest(const FString& Parameters)
{
    using namespace LHRulesTestsPrivate;
    using namespace LH::Rules;
    // Ledger rows: Creation question flow, Creation RNG, Reachable maximum examples (NOT used as RNG).
    auto P=Creation(); auto A=Answers();
    auto First=RollCreation(P,A,0), Second=RollCreation(P,A,1);
    TestTrue(TEXT("Synthetic legal first roll"),First.Diagnostic.IsAccepted());
    TestTrue(TEXT("Synthetic reroll upper boundary"),Second.Diagnostic.IsAccepted());
    TestEqual(TEXT("Reroll does not mutate first roll"),First.Value.Attributes.Strength,int64(10));
    TestEqual(TEXT("Synthetic reroll strength"),Second.Value.Attributes.Strength,int64(20));
    TestEqual(TEXT("Point conservation"),First.Value.Attributes.Strength+First.Value.Attributes.Endurance+First.Value.Attributes.Agility+
        First.Value.Attributes.Intelligence+First.Value.Attributes.Wisdom+First.Value.UnspentPoints,int64(60));
    TestFalse(TEXT("Below lower boundary"),ValidateCreation(P,{9,10,10,10,10},11).Diagnostic.IsAccepted());
    TestFalse(TEXT("Above upper boundary"),ValidateCreation(P,{21,10,10,10,10},0).Diagnostic.IsAccepted());
    TestFalse(TEXT("Incorrect budget"),ValidateCreation(P,{10,10,10,10,10},9).Diagnostic.IsAccepted());
    TestFalse(TEXT("Negative pool"),ValidateCreation(P,{20,10,10,10,10},-1).Diagnostic.IsAccepted());
    TestFalse(TEXT("Invalid roll index"),RollCreation(P,A,2).Diagnostic.IsAccepted());
    A.RemoveAt(0); TestFalse(TEXT("Incomplete answers"),RollCreation(P,A,0).Diagnostic.IsAccepted());
    A=Answers(); A[1].Question=A[0].Question; TestFalse(TEXT("Duplicate questions"),RollCreation(P,A,0).Diagnostic.IsAccepted());
    A=Answers(); A[0].Answer.Value=TEXT("Answer.Other"); TestFalse(TEXT("Outcome tied to answers"),RollCreation(P,A,0).Diagnostic.IsAccepted());
    P.TotalPoints.Resolution=ELHValueResolution::Unresolved;
    TestTrue(TEXT("Unresolved rejects instead of zero"),ValidateCreation(P,{10,10,10,10,10},10).Diagnostic.Reason==EReason::Unresolved);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStatsTests,"Lighthaven.Rules.Prototype.Synthetic.DerivedStatsEquipUnequipSymmetry",LHRulesTestsPrivate::LHRulesTestsFlags)
bool FLHStatsTests::RunTest(const FString& Parameters)
{
    using namespace LHRulesTestsPrivate;
    using namespace LH::Rules;
    // Ledger rows: Derived stats, Retroactive growth.
    auto P=Stats(); FStatsInput I; I.Base={10,19,11,13,15}; I.EarnedHealth=42; I.EarnedMana=31;
    auto Base=DeriveStats(P,I); TestTrue(TEXT("Synthetic base"),Base.Diagnostic.IsAccepted());
    TestEqual(TEXT("Fractional formula retained"),Base.Value.Avoidance,5.5);
    TestEqual(TEXT("All named attribute formula fixture"),Base.Value.Capacity,20.0);
    TestEqual(TEXT("Historical health retained"),Base.Value.MaxHealth,42.0);
    FModifier M; M.Attributes=Attributes(2,1,3,4,5); M.Health=Num(5); M.Mana=Num(3); M.Accuracy=Num(1);
    M.Avoidance=Num(0); M.DamageBonus=Num(0); M.Armor=Num(0); M.Capacity=Num(0); I.Equipment.Add(M);
    auto Equipped=DeriveStats(P,I); TestTrue(TEXT("Equip"),Equipped.Diagnostic.IsAccepted());
    TestEqual(TEXT("Effective END20 boundary"),Equipped.Value.Effective.Endurance,int64(20));
    TestEqual(TEXT("Equipment never retroactively grows HP"),Equipped.Value.MaxHealth,47.0);
    TestEqual(TEXT("Accuracy uses effective attributes plus modifier"),Equipped.Value.Accuracy,15.0);
    I.Equipment.Empty(); auto Unequipped=DeriveStats(P,I);
    TestEqual(TEXT("Unequip health symmetry"),Unequipped.Value.MaxHealth,Base.Value.MaxHealth);
    TestEqual(TEXT("Unequip accuracy symmetry"),Unequipped.Value.Accuracy,Base.Value.Accuracy);
    TestEqual(TEXT("Base input untouched"),I.Base.Strength,int64(10));
    P.Armor.Constant.Resolution=ELHValueResolution::Unresolved;
    TestTrue(TEXT("Unresolved coefficient rejection"),DeriveStats(P,I).Diagnostic.Reason==EReason::Unresolved);
    P=Stats(); I.Equipment.Add(M); I.Equipment[0].Health.Value=std::numeric_limits<double>::infinity();
    TestFalse(TEXT("Nonfinite modifier rejection"),DeriveStats(P,I).Diagnostic.IsAccepted());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHProgressionTests,"Lighthaven.Rules.Prototype.Synthetic.LevelEntitlementGrowthAndDebt",LHRulesTestsPrivate::LHRulesTestsFlags)
bool FLHProgressionTests::RunTest(const FString& Parameters)
{
    using namespace LHRulesTestsPrivate;
    using namespace LH::Rules;
    // Ledger: Level entitlement (+5/+15 documented), XP curve (synthetic), HP/MP growth dependence, Atomic level-up / debt recovery.
    auto P=Progression(); auto I=ProgressionInput(); I.ExperienceGain=99;
    TestEqual(TEXT("Below first synthetic threshold"),Advance(P,I).Value.Awards.Num(),0);
    I.ExperienceGain=100; I.Rolls={Growth(1,0.25,0.49)}; auto One=Advance(P,I);
    TestTrue(TEXT("Exact threshold accepted"),One.Diagnostic.IsAccepted());
    if (!One.Diagnostic.IsAccepted() || One.Value.Awards.Num() != 1) return false;
    TestEqual(TEXT("Source documented attribute grant"),One.Value.UnspentAttributes,int64(5));
    TestEqual(TEXT("Source documented skill grant"),One.Value.UnspentSkills,int64(15));
    TestEqual(TEXT("Synthetic END19 floor rounding"),One.Value.Awards[0].HealthIncrement.Value,11.0);
    TestEqual(TEXT("Synthetic mana floor rounding"),One.Value.Awards[0].ManaIncrement.Value,5.0);
    I.Rolls[0].Health=0; auto End19=Advance(P,I);
    if (!End19.Diagnostic.IsAccepted() || End19.Value.Awards.Num() != 1) return false;
    TestEqual(TEXT("Synthetic END19 boundary floor"),End19.Value.Awards[0].HealthIncrement.Value,10.0);
    I.GrowthAttributes.Endurance=20; auto End20=Advance(P,I);
    if (!End20.Diagnostic.IsAccepted() || End20.Value.Awards.Num() != 1) return false;
    TestEqual(TEXT("Synthetic END20 growth boundary"),End20.Value.Awards[0].HealthIncrement.Value,11.0);
    I=ProgressionInput(); I.ExperienceGain=600; I.Rolls={Growth(1),Growth(2,0.5,0.5),Growth(3,0.99,0.99)};
    auto Multi=Advance(P,I); TestTrue(TEXT("Multi-level operation"),Multi.Diagnostic.IsAccepted());
    if (!Multi.Diagnostic.IsAccepted() || Multi.Value.Awards.Num() != 3) return false;
    TestEqual(TEXT("Every growth award retained"),Multi.Value.Awards.Num(),3);
    TestEqual(TEXT("Each explicit roll retained for serializer"),Multi.Value.AcceptedRolls.Num(),3);
    TestEqual(TEXT("Source-backed grants per newly earned level"),Multi.Value.UnspentAttributes,int64(15));
    TestEqual(TEXT("Source-backed skill grants"),Multi.Value.UnspentSkills,int64(45));
    TestEqual(TEXT("Last earned level"),Multi.Value.EarnedLevel,int64(4));
    TestEqual(TEXT("Award records original growth inputs"),Multi.Value.Awards[0].GrowthInputs.Endurance.Value,int64(19));
    I=ProgressionInput(); I.EarnedLevel=3; I.ExperienceBalance=300; I.ExperienceDebt=100; I.UnspentAttributes=10; I.UnspentSkills=30;
    I.ExperienceGain=99; auto Partial=Advance(P,I);
    TestEqual(TEXT("Debt partly repaid"),Partial.Value.ExperienceDebt,int64(1));
    TestEqual(TEXT("Gain pays separate debt before balance"),Partial.Value.ExperienceBalance,int64(300));
    I.ExperienceGain=100; auto Recovered=Advance(P,I); TestTrue(TEXT("Debt recovery accepted"),Recovered.Diagnostic.IsAccepted());
    TestEqual(TEXT("Debt zero"),Recovered.Value.ExperienceDebt,int64(0));
    TestEqual(TEXT("Recovery never re-awards points"),Recovered.Value.UnspentAttributes,int64(10));
    TestEqual(TEXT("Recovery never re-awards growth"),Recovered.Value.Awards.Num(),0);
    I.ExperienceGain=400; I.Rolls={Growth(4)}; auto Beyond=Advance(P,I);
    TestEqual(TEXT("Gain spans recovery and one new level"),Beyond.Value.Awards.Num(),1);
    TestEqual(TEXT("Only new level grant"),Beyond.Value.UnspentAttributes,int64(15));
    I=ProgressionInput(); I.ExperienceGain=100; I.Rolls={Growth(1)}; P.HealthGrowth.Constant.Resolution=ELHValueResolution::Unresolved;
    auto Missing=Advance(P,I); TestTrue(TEXT("Unresolved growth rejects entire operation"),Missing.Diagnostic.Reason==EReason::Unresolved);
    TestEqual(TEXT("No partial XP output on rejection"),Missing.Value.ExperienceBalance,int64(0));
    P=Progression(); I.Rolls[0].Health=1; TestFalse(TEXT("Normalized roll upper boundary excluded"),Advance(P,I).Diagnostic.IsAccepted());
    I.Rolls={Growth(1)}; I.UnspentAttributes=std::numeric_limits<int64>::max();
    TestTrue(TEXT("Grant overflow rejection"),Advance(P,I).Diagnostic.Reason==EReason::Overflow);
    I=ProgressionInput(); I.ExperienceGain=300; I.Rolls={Growth(1),Growth(1)};
    TestFalse(TEXT("Duplicate award IDs rejected"),Advance(P,I).Diagnostic.IsAccepted());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHRequirementTests,"Lighthaven.Rules.Prototype.Synthetic.EquipmentSpellAndQuiverRequirements",LHRulesTestsPrivate::LHRulesTestsFlags)
bool FLHRequirementTests::RunTest(const FString& Parameters)
{
    using namespace LHRulesTestsPrivate;
    using namespace LH::Rules;
    // Ledger: Requirement evaluation, Weapon requirements, Wooden Arrows quiver, Fire Dart (min level2/WIS15/INT21).
    auto P=Requirements(); FLHEligibility E; E.MinimumAttributes=Minimum(0,0,0,21,15); E.MinimumLevel=Int(2);
    FRequirementInput I; I.Base={10,10,10,21,15}; I.Effective=I.Base; I.Level=2;
    TestTrue(TEXT("Fire Dart documented minimum boundary under synthetic base-stat policy"),CheckRequirements(P,E,I).Diagnostic.IsAccepted());
    I.Base.Intelligence=20; I.Effective.Intelligence=21;
    TestFalse(TEXT("Equipment bonus does not qualify under selected base policy"),CheckRequirements(P,E,I).Diagnostic.IsAccepted());
    P.Basis=EAttributeBasis::Effective; TestTrue(TEXT("Explicit effective policy"),CheckRequirements(P,E,I).Diagnostic.IsAccepted());
    I.Level=1; TestFalse(TEXT("Below level requirement"),CheckRequirements(P,E,I).Diagnostic.IsAccepted());
    E.MinimumAttributes=Minimum(0,0,0,0,0); E.MinimumLevel=Int(0); I.Level=1; I.bBow=true;
    TestFalse(TEXT("Bow without quiver rejected"),CheckRequirements(P,E,I).Diagnostic.IsAccepted());
    I.bCompatibleQuiverEquipped=true; TestTrue(TEXT("Bow with equipped compatible quiver"),CheckRequirements(P,E,I).Diagnostic.IsAccepted());
    FLHContentId Spell; Spell.Value=TEXT("Spell.SyntheticPrerequisite"); E.RequiredSpells.Add(Spell);
    TestFalse(TEXT("Missing spell prerequisite"),CheckRequirements(P,E,I).Diagnostic.IsAccepted());
    I.Spells.Add(Spell); TestTrue(TEXT("Learned spell prerequisite"),CheckRequirements(P,E,I).Diagnostic.IsAccepted());
    FLHContentId Skill; Skill.Value=TEXT("Skill.Archery"); E.RequiredSkills.Add(Skill);
    FLHMechanicalField Min; Min.Key=Skill.Value; Min.Value=Num(10); E.SkillMinimums.Add(Min);
    FLHLearnedSkill Learned; Learned.Skill=Skill; Learned.TrainedValue=Int(9); I.Skills.Add(Learned);
    TestFalse(TEXT("Below synthetic skill minimum"),CheckRequirements(P,E,I).Diagnostic.IsAccepted());
    I.Skills[0].TrainedValue=Int(10); TestTrue(TEXT("Exact synthetic skill minimum"),CheckRequirements(P,E,I).Diagnostic.IsAccepted());
    E.MinimumAttributes.Strength.Resolution=ELHValueResolution::Unresolved;
    TestTrue(TEXT("Unresolved is not unrestricted"),CheckRequirements(P,E,I).Diagnostic.Reason==EReason::Unresolved);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCombatTests,"Lighthaven.Rules.Prototype.Synthetic.HitDamageBoundariesAndRounding",LHRulesTestsPrivate::LHRulesTestsFlags)
bool FLHCombatTests::RunTest(const FString& Parameters)
{
    using namespace LHRulesTestsPrivate;
    using namespace LH::Rules;
    // Ledger: Physical hit/damage, Spell targeting. All damage/chance coefficients here are synthetic.
    auto P=Combat(); FCombatInput I; I.WeaponMinimum=6; I.WeaponMaximum=10; I.DamageRoll=0.49; I.HitRoll=0.49;
    auto Hit=ResolveCombat(P,I); TestTrue(TEXT("Below synthetic chance hits"),Hit.Value.bHit);
    TestTrue(TEXT("Fractional raw damage"),FMath::IsNearlyEqual(Hit.Value.RawDamage,7.96));
    TestEqual(TEXT("Floor to data quantum"),Hit.Value.Damage,7.0);
    I.HitRoll=0.5; auto Miss=ResolveCombat(P,I); TestTrue(TEXT("Miss calculation accepted"),Miss.Diagnostic.IsAccepted()); TestFalse(TEXT("Exact chance boundary misses"),Miss.Value.bHit);
    TestEqual(TEXT("Miss no damage"),Miss.Value.Damage,0.0);
    I.Accuracy=100; I.HitRoll=0.89; TestEqual(TEXT("Upper chance clamp"),ResolveCombat(P,I).Value.Chance,0.9);
    I.Accuracy=0; I.Avoidance=100; TestEqual(TEXT("Lower chance clamp"),ResolveCombat(P,I).Value.Chance,0.1);
    I.Avoidance=0; I.HitRoll=0; I.DamageRoll=0; I.Armor=2; I.QuiverBonus=1; I.Resistance=0.5;
    TestEqual(TEXT("Synthetic flat armor then fractional resistance rounding"),ResolveCombat(P,I).Value.Damage,2.0);
    I.Resistance=1; TestEqual(TEXT("Full resistance zero damage"),ResolveCombat(P,I).Value.Damage,0.0);
    I.Resistance=0; I.Armor=100; TestEqual(TEXT("Armor floor zero"),ResolveCombat(P,I).Value.Damage,0.0);
    I.bSpell=true; I.HitRoll=0.99; auto Spell=ResolveCombat(P,I);
    TestTrue(TEXT("Documented spell no normal miss roll"),Spell.Value.bHit);
    TestEqual(TEXT("Synthetic spell bypasses physical armor"),Spell.Value.Damage,7.0);
    I.DamageRoll=1; TestFalse(TEXT("Damage roll excluded upper bound"),ResolveCombat(P,I).Diagnostic.IsAccepted());
    I.DamageRoll=0; P.DamageQuantum.Value=0; TestFalse(TEXT("Zero quantum invalid"),ResolveCombat(P,I).Diagnostic.IsAccepted());
    P=Combat(); P.ArmorScale.Resolution=ELHValueResolution::Unresolved;
    TestTrue(TEXT("Unresolved combat rejection"),ResolveCombat(P,I).Diagnostic.Reason==EReason::Unresolved);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHManaTests,"Lighthaven.Rules.Prototype.Ledger.ManaFractionalCarry",LHRulesTestsPrivate::LHRulesTestsFlags)
bool FLHManaTests::RunTest(const FString& Parameters)
{
    using namespace LHRulesTestsPrivate;
    using namespace LH::Rules;
    // Ledger: Mana regeneration rate (prototype 1 MP / 5s), Passive mana recovery.
    const auto P=MakeLedgerPrototypeRuleset().Mana; FManaInput I; I.Maximum=10; I.ActiveSeconds=4.5;
    auto First=RegenerateMana(P,I); TestTrue(TEXT("Ledger prototype regen"),First.Diagnostic.IsAccepted());
    TestEqual(TEXT("No early tick"),First.Value.Current,0.0); TestEqual(TEXT("Persist fractional seconds"),First.Value.FractionalSeconds,4.5);
    I.Current=First.Value.Current; I.FractionalSeconds=First.Value.FractionalSeconds; I.ActiveSeconds=0.5;
    auto Reloaded=RegenerateMana(P,I); TestEqual(TEXT("Reloaded carry yields one exact tick"),Reloaded.Value.Current,1.0);
    TestEqual(TEXT("Exact interval remainder"),Reloaded.Value.FractionalSeconds,0.0);
    I.FractionalSeconds=0; I.ActiveSeconds=0; I.Current=Reloaded.Value.Current;
    TestEqual(TEXT("Reload with zero active time grants no extra tick"),RegenerateMana(P,I).Value.Current,1.0);
    I.Current=9; I.ActiveSeconds=12; I.FractionalSeconds=0; auto Clamped=RegenerateMana(P,I);
    TestEqual(TEXT("Multi tick clamped"),Clamped.Value.Current,10.0); TestEqual(TEXT("Carry at full preserved"),Clamped.Value.FractionalSeconds,2.0);
    I.bPaused=true; TestEqual(TEXT("Paused clock frozen"),RegenerateMana(P,I).Value.FractionalSeconds,0.0);
    I.bPaused=false; I.bAlive=false; TestEqual(TEXT("Dead resource frozen"),RegenerateMana(P,I).Value.Current,9.0);
    I.bAlive=true; I.FractionalSeconds=5; TestFalse(TEXT("Noncanonical carry rejected"),RegenerateMana(P,I).Diagnostic.IsAccepted());
    I.FractionalSeconds=0; I.ActiveSeconds=-1; TestFalse(TEXT("Negative elapsed rejected"),RegenerateMana(P,I).Diagnostic.IsAccepted());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHUnresolvedTests,"Lighthaven.Rules.Prototype.Ledger.UnresolvedParametersReject",LHRulesTestsPrivate::LHRulesTestsFlags)
bool FLHUnresolvedTests::RunTest(const FString& Parameters)
{
    using namespace LHRulesTestsPrivate;
    using namespace LH::Rules;
    // Missing ledger rows must never become zero defaults in the play fixture.
    auto P=MakeLedgerPrototypeRuleset();
    TestTrue(TEXT("Creation RNG unresolved"),RollCreation(P.Creation,Answers(),0).Diagnostic.Reason==EReason::Unresolved);
    FStatsInput S; TestTrue(TEXT("Derived stats unresolved"),DeriveStats(P.Stats,S).Diagnostic.Reason==EReason::Unresolved);
    TestTrue(TEXT("XP curve unresolved"),Advance(P.Progression,ProgressionInput()).Diagnostic.Reason==EReason::Unresolved);
    FLHEligibility E; FRequirementInput I;
    TestTrue(TEXT("Requirement evaluation unresolved"),CheckRequirements(P.Requirements,E,I).Diagnostic.Reason==EReason::Unresolved);
    FCombatInput C; TestTrue(TEXT("Physical hit/damage unresolved"),ResolveCombat(P.Combat,C).Diagnostic.Reason==EReason::Unresolved);
    FManaInput M; P.Mana.Amount.Resolution=ELHValueResolution::Unresolved;
    TestTrue(TEXT("Mana unresolved explicit rejection"),RegenerateMana(P.Mana,M).Diagnostic.Reason==EReason::Unresolved);
    P=MakeLedgerPrototypeRuleset(); P.Mana.Amount.Provenance.Status=ELHProvenanceStatus::Missing;
    TestTrue(TEXT("Resolved storage with missing provenance rejects"),RegenerateMana(P.Mana,M).Diagnostic.Reason==EReason::Unresolved);
    return true;
}
#endif
