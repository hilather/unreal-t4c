#include "Misc/AutomationTest.h"
#include "Input/LHInputConfig.h"
#include "Input/LHTargeting.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHInputParityTest,"Lighthaven.Controls.InputConfigParity",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHInputParityTest::RunTest(const FString&)
{
    auto* Config=NewObject<ULHInputConfig>(); Config->Initialize();
    auto* Gameplay=Config->Context(ELHInputContext::Gameplay);
    TestNotNull(TEXT("Gameplay context"),Gameplay);
    TestNotNull(TEXT("UI context"),Config->Context(ELHInputContext::UI));
    TestNotNull(TEXT("Creation context"),Config->Context(ELHInputContext::Creation));
    TestTrue(TEXT("Distinct UI context"),Gameplay!=Config->Context(ELHInputContext::UI));
    for (FName Name : ULHInputConfig::GameplayActions()) { TestNotNull(Name.ToString(),Config->Action(Name)); TestTrue(Name.ToString()+TEXT(" KBM/gamepad parity"),Config->HasDeviceParity(Name)); }
    Config->Initialize(); TestTrue(TEXT("Idempotent construction"),Gameplay==Config->Context(ELHInputContext::Gameplay));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHMovementContextTest,"Lighthaven.Controls.ContextClearsMovement",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHMovementContextTest::RunTest(const FString&)
{
    LHControls::FMovementState State;
    for (auto Mode : {ELHInputContext::UI,ELHInputContext::Creation,ELHInputContext::Gameplay})
    {
        State.Held=FVector2D(1,1); State.SwitchContext(Mode);
        TestTrue(TEXT("Held movement cleared"),State.Held.IsZero()); TestTrue(TEXT("Requested context"),State.Context==Mode);
    }
    State.Held=FVector2D(0,1); State.Clear(); TestTrue(TEXT("Focus clear"),State.Held.IsZero());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHTargetCycleTest,"Lighthaven.Controls.TargetOrderAndFilter",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHTargetCycleTest::RunTest(const FString&)
{
    const TArray<FLHTargetCandidate> Candidates={{"B",100,true,true,true},{"A",100,true,true,true},{"Near",1,true,true,true},{"Dead",0,false,true,true},{"Unreachable",0,true,false,true},{"Hidden",0,true,true,false},{"Far",101,true,true,true},{"Negative",-1,true,true,true}};
    const auto Order=LHControls::OrderedTargets(Candidates,100);
    TestEqual(TEXT("Only valid within inclusive boundary"),Order.Num(),3);
    if (Order.Num()!=3) return false;
    TestEqual(TEXT("Nearest first"),Order[0],FString("Near")); TestEqual(TEXT("Tie key order"),Order[1],FString("A"));
    TestEqual(TEXT("Acquire nearest"),LHControls::CycleTarget(Order,"Missing",1),FString("Near"));
    TestEqual(TEXT("Next wrap"),LHControls::CycleTarget(Order,"B",1),FString("Near"));
    TestEqual(TEXT("Previous wrap"),LHControls::CycleTarget(Order,"Near",-1),FString("B"));
    TestTrue(TEXT("Empty clears"),LHControls::CycleTarget({},"B",1).IsEmpty());
    return true;
}
#endif
