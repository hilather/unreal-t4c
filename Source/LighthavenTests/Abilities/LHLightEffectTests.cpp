#include "Misc/AutomationTest.h"
#include "Abilities/LHLightEffect.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHLightLifecycleTest,"Lighthaven.Abilities.LightEffectLifecycle",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHLightLifecycleTest::RunTest(const FString&)
{
    auto* C=NewObject<ULHCombatComponent>(); auto* Reload=NewObject<ULHCombatComponent>();
    FLHEntityId Owner; Owner.RunId=FGuid(1,2,3,4); Owner.InstanceId=FGuid(5,6,7,8); Owner.Area.Content.Value=TEXT("Area.TempleB1");
    FLHSessionRecord S; S.RequestEpoch=FGuid(9,10,11,12); S.EffectPolicy=ELHEffectSavePolicy::CompletedActionBoundaryOnly;
    TestTrue(TEXT("Apply Light"),C->RestoreLightRemainingSeconds(600));
    C->AdvanceLightEffect(10,false,false,false);
    TestTrue(TEXT("Capture"),LHAbilities::CaptureLightEffect(*C,Owner,S));
    TestTrue(TEXT("Restore"),LHAbilities::RestoreLightEffect(*Reload,Owner,S));
    TestEqual(TEXT("Reload consumes no tick"),Reload->GetLightRemainingSeconds(),590.0);
    TestTrue(TEXT("Repeated restore replaces"),LHAbilities::RestoreLightEffect(*Reload,Owner,S));
    Reload->AdvanceLightEffect(10,true,false,false); Reload->AdvanceLightEffect(10,false,true,false); Reload->AdvanceLightEffect(10,false,false,true);
    TestEqual(TEXT("Pause AI and travel preserve"),Reload->GetLightRemainingSeconds(),590.0);
    Reload->AdvanceLightEffect(1,false,false,false);
    TestEqual(TEXT("Single resumed tick"),Reload->GetLightRemainingSeconds(),589.0);
    TestFalse(TEXT("Invalid restore rejected"),Reload->RestoreLightRemainingSeconds(601));
    TestEqual(TEXT("Rejected restore is atomic"),Reload->GetLightRemainingSeconds(),589.0);
    Reload->AdvanceLightEffect(600,false,false,false);
    TestEqual(TEXT("Expires"),Reload->GetLightRemainingSeconds(),0.0);
    TestTrue(TEXT("Capture expired"),LHAbilities::CaptureLightEffect(*Reload,Owner,S)); TestTrue(TEXT("Expiry clears record"),S.DurableEffects.IsEmpty());
    C->RestoreLightRemainingSeconds(100); TestTrue(TEXT("Empty restores off"),LHAbilities::RestoreLightEffect(*C,Owner,S)); TestEqual(TEXT("Off"),C->GetLightRemainingSeconds(),0.0);
    return true;
}
#endif
