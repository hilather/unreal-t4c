#include "Misc/AutomationTest.h"
#include "Rules/LHRules.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Ratio,"Lighthaven.Rules.Prototype.Stage1Ratio",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHStage1Ratio::RunTest(const FString&)
{
    const auto P=LH::Rules::MakeStage1PrototypeCombat(); LH::Rules::FCombatInput I; I.Accuracy=20; I.Avoidance=10; I.WeaponMinimum=1; I.WeaponMaximum=4; I.DamageRoll=.999; I.DamageBonus=3; I.Armor=2;
    const auto R=LH::Rules::ResolveCombat(P,I); TestTrue(TEXT("Accepted"),R.Diagnostic.IsAccepted()); TestEqual(TEXT("Ratio"),R.Value.Chance,2.0/3); TestEqual(TEXT("Inclusive maximum weapon roll"),R.Value.Damage,5.0);
    I.Accuracy=0; I.Avoidance=0; TestEqual(TEXT("Zero denominator"),LH::Rules::ResolveCombat(P,I).Value.Chance,.5);
    TestEqual(TEXT("Bow attribute bonus"),LH::Rules::PhysicalAttributeBonus(21,19,true),4.0); return true;
}
#endif
