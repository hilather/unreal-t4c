#include "Misc/AutomationTest.h"
#include "UI/LHUIPresenter.h"
#include "UI/LHUIStyle.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace
{
// The presenter receives only commands and copy-returning reads. There are no stat setters here.
struct FOwners : ILHCommandHandler, ILHUIReadOwner, ILHUISessionOwner
{
    FLHSaveSnapshot State;
    FLHCreateCharacterRequest Last;
    int32 Creates = 0, Loads = 0;
    bool bAccept = false;
    FOwners() { State.Session.RequestEpoch = FGuid::NewGuid(); }
    FLHCommandResult Execute(const FLHCreateCharacterRequest& R) override
    {
        Last = R; ++Creates; FLHCommandResult V; V.Request = R.Request;
        V.Disposition = bAccept ? ELHCommandDisposition::Accepted : ELHCommandDisposition::Rejected;
        V.Reason = bAccept ? ELHCommandReason::None : ELHCommandReason::InvalidRequest; return V;
    }
#define UNUSED_COMMAND(T) FLHCommandResult Execute(const T&) override { return {}; }
    UNUSED_COMMAND(FLHAllocateAttributePointsRequest)
    UNUSED_COMMAND(FLHTrainSkillRequest)
    UNUSED_COMMAND(FLHLearnSpellRequest)
    UNUSED_COMMAND(FLHBuyItemRequest)
    UNUSED_COMMAND(FLHSellItemRequest)
    UNUSED_COMMAND(FLHEquipItemRequest)
    UNUSED_COMMAND(FLHUseAbilityRequest)
    UNUSED_COMMAND(FLHInteractRequest)
    UNUSED_COMMAND(FLHTakeLootRequest)
    UNUSED_COMMAND(FLHRequestTravelRequest)
#undef UNUSED_COMMAND
    FLHSaveSnapshot Snapshot() const override { return State; }
    TArray<FLHUIProfile> Profiles() const override
    {
        FLHUIProfile P; P.Id = State.Header.CharacterId; P.Name = TEXT("Existing");
        P.bCanContinue = true; P.bRequiresRecoveryAcknowledgment = true; return {P};
    }
    TArray<FLHContentId> AppearanceCatalog() const override { return {}; }
    FLHUICreationPreview Preview(const FString&, const TArray<FLHContentId>&,
        const TArray<FLHQuestionAnswer>& Answers, bool) override
    { FLHUICreationPreview P; P.Record.QuestionAnswers = Answers; P.Token = FGuid::NewGuid(); P.bLegal = true; return P; }
    FString Continue(FLHCharacterId, bool) override { ++Loads; return {}; }
    FString RequestExit() override { return TEXT("Storage unavailable"); }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHUIFormTest, "Lighthaven.UI.RejectionPreservesFormAndDuplicateConfirm", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHUIFormTest::RunTest(const FString&)
{
    FOwners O; FLHUIPresenter P(O,O,O);
    FLHContentId Appearance; Appearance.Value = TEXT("Presentation.Player.Human");
    FLHQuestionAnswer Answer; Answer.Question.Value = TEXT("Question.One"); Answer.Answer.Value = TEXT("Answer.One");
    P.EditCreation(TEXT("Name kept"), {Appearance}, {Answer}); P.Roll(false);
    P.ConfirmCreation();
    TestEqual(TEXT("rejection retains name"), P.NameInput(), FString(TEXT("Name kept")));
    TestTrue(TEXT("error remains visible"), !P.Error().IsEmpty());
    TestEqual(TEXT("appearance retained in command"), O.Last.AppearanceIds.Num(), 1);
    TestEqual(TEXT("answers retained in command"), O.Last.Creation.QuestionAnswers.Num(), 1);
    const auto RejectedId = O.Last.Request.Value;
    TestTrue(TEXT("request uses owner epoch"), O.Last.Request.Epoch == O.State.Session.RequestEpoch);
    TestEqual(TEXT("canonical snapshot unchanged"), O.State.Character.DisplayName, FString());
    O.bAccept = true; P.ConfirmCreation(); P.ConfirmCreation();
    TestEqual(TEXT("accepted confirm latched"), O.Creates, 2);
    TestTrue(TEXT("definitive rejection retry uses fresh ID"), O.Last.Request.Value != RejectedId);
    TestFalse(TEXT("cannot edit accepted form"), P.EditCreation(TEXT("Other"), {}, {}));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHUIFocusTest, "Lighthaven.UI.FocusTraversal", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHUIFocusTest::RunTest(const FString&)
{
    FOwners O; FLHUIPresenter P(O,O,O);
    const TArray<ELHUIScreen> Screens = {ELHUIScreen::Frontend, ELHUIScreen::Characters, ELHUIScreen::Creation,
        ELHUIScreen::CharacterSheet, ELHUIScreen::Inventory, ELHUIScreen::Settings};
    const TArray<FName> Commits = {"New", "Continue", "Confirm", "Confirm", "Confirm", "Apply"};
    for (int32 S = 0; S < Screens.Num(); ++S)
    {
        P.Open(Screens[S]); P.MoveFocus(-1000); bool bFound = false;
        for (int32 I = 0; I < P.FocusOrder().Num(); ++I)
        { bFound |= P.FocusedControl() == Commits[S]; P.MoveFocus(1); }
        TestTrue(TEXT("commit is reachable"), bFound);
        const auto End = P.FocusedControl(); P.MoveFocus(1);
        TestTrue(TEXT("does not wrap"), P.FocusedControl() == End);
    }
    P.Open(ELHUIScreen::Inventory);
    TestTrue(TEXT("quiver first-class focus target"), P.FocusOrder().Contains("Quiver"));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHUIRecoveryTest, "Lighthaven.UI.RecoveryAndUnknownValues", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHUIRecoveryTest::RunTest(const FString&)
{
    FOwners O; O.State.Header.CharacterId.Value = FGuid::NewGuid(); FLHUIPresenter P(O,O,O);
    P.SelectProfile(O.State.Header.CharacterId);
    TestFalse(TEXT("recovery requires acknowledgment"), P.Continue(false));
    TestEqual(TEXT("no load before acknowledgment"), O.Loads, 0);
    TestTrue(TEXT("acknowledged recovery loads"), P.Continue(true));
    TestFalse(TEXT("failed exit remains visible"), P.Quit());
    FLHInteger V; V.Value = 100;
    TestEqual(TEXT("unresolved storage never shown"), FLHUIPresenter::Format(V), FString(TEXT("— Unknown")));
    V.Resolution = ELHValueResolution::Resolved; V.Value = 0; V.Provenance.Status = ELHProvenanceStatus::Prototype;
    TestEqual(TEXT("resolved prototype zero explicit"), FLHUIPresenter::Format(V), FString(TEXT("0 Prototype")));
    TestEqual(TEXT("720 uniform scale"), FLHUIStyle::Scale(FVector2D(1280,720)), 2.f/3.f);
    return true;
}
#endif
