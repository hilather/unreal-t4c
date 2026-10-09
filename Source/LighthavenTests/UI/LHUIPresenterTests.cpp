#include "Misc/AutomationTest.h"
#include "UI/LHUIPresenter.h"
#include "UI/LHUIStyle.h"
#include "UI/LHUIWidgetHarness.h"
#include "InputCoreTypes.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace
{
// The presenter receives only commands and copy-returning reads. There are no stat setters here.
struct FOwners : ILHCommandHandler, ILHUIReadOwner, ILHUISessionOwner
{
    FLHSaveSnapshot State;
    FLHCreateCharacterRequest Last;
    int32 Creates = 0, Loads = 0, Allocates = 0, Equips = 0, Exits = 0;
    bool bReadable = true;
    bool bOwnsSaveStatus=false;
    FString SaveStatus;
    bool OwnsPersistenceStatus() const override { return bOwnsSaveStatus; }
    FString OwnerStatus() const override { return SaveStatus; }
    FString RetryPersistence() override { return SaveStatus; }
    int32 Resumes=0;
    bool ResumeGameplay() override { ++Resumes; return true; }
    FLHAttributeBlock LastAllocation;
    FLHEquipItemRequest LastEquipment;
    bool bAccept = false;
    bool bLegalAllocationReview=true;
    FOwners() { State.Session.RequestEpoch = FGuid::NewGuid(); }
    FLHCommandResult Execute(const FLHCreateCharacterRequest& R) override
    {
        Last = R; ++Creates; FLHCommandResult V; V.Request = R.Request;
        V.Disposition = bAccept ? ELHCommandDisposition::Accepted : ELHCommandDisposition::Rejected;
        V.Reason = bAccept ? ELHCommandReason::None : ELHCommandReason::InvalidRequest; return V;
    }
#define UNUSED_COMMAND(T) FLHCommandResult Execute(const T&) override { return {}; }
    FLHCommandResult Execute(const FLHAllocateAttributePointsRequest& R) override
    {
        ++Allocates; LastAllocation = R.Points; FLHCommandResult V;
        V.Disposition = bAccept ? ELHCommandDisposition::Accepted : ELHCommandDisposition::Rejected;
        V.Reason = bAccept ? ELHCommandReason::None : ELHCommandReason::InsufficientPoints; return V;
    }
    FLHUIIntentReview ReviewAllocation(const FLHAttributeBlock&) const override { FLHUIIntentReview R; R.bLegal = bLegalAllocationReview; R.Summary = bLegalAllocationReview ? TEXT("Fixture allocation review") : TEXT("Allocation rejected by character authority"); return R; }
    FLHUIIntentReview ReviewEquipment(const FLHEntityId&,ELHEquipmentSlot,bool) const override { FLHUIIntentReview R; R.bLegal = true; R.Summary = TEXT("Fixture equipment review"); return R; }
    UNUSED_COMMAND(FLHTrainSkillRequest)
    UNUSED_COMMAND(FLHLearnSpellRequest)
    UNUSED_COMMAND(FLHBuyItemRequest)
    UNUSED_COMMAND(FLHSellItemRequest)
    FLHCommandResult Execute(const FLHEquipItemRequest& R) override
    {
        ++Equips; LastEquipment = R; FLHCommandResult V;
        V.Disposition = bAccept ? ELHCommandDisposition::Accepted : ELHCommandDisposition::Rejected;
        V.Reason = bAccept ? ELHCommandReason::None : ELHCommandReason::Ineligible; return V;
    }
    UNUSED_COMMAND(FLHUseAbilityRequest)
    UNUSED_COMMAND(FLHUseItemRequest)
    UNUSED_COMMAND(FLHInteractRequest)
    UNUSED_COMMAND(FLHTakeLootRequest)
    UNUSED_COMMAND(FLHRequestTravelRequest)
#undef UNUSED_COMMAND
    FLHSaveSnapshot Snapshot() const override { return State; }
    TArray<FLHUIProfile> Profiles() const override
    {
        FLHUIProfile P; P.Id = State.Header.CharacterId; P.Name = TEXT("Existing");
        P.bCanContinue = bReadable; P.bRequiresRecoveryAcknowledgment = true; return {P};
    }
    TArray<FLHContentId> AppearanceCatalog() const override { return {}; }
    FLHUICreationPreview Preview(const FString&, const TArray<FLHContentId>&,
        const TArray<FLHQuestionAnswer>& Answers, bool) override
    { FLHUICreationPreview P; P.Record.QuestionAnswers = Answers; P.Token = FGuid::NewGuid(); P.bLegal = true; return P; }
    FString Continue(FLHCharacterId, bool) override { ++Loads; return {}; }
    FString RequestExit() override { ++Exits; return TEXT("Storage unavailable"); }
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
    TestEqual(TEXT("unresolved storage never shown"), FLHUIPresenter::Format(V), FString(TEXT("— (not available in this prototype)")));
    V.Resolution = ELHValueResolution::Resolved; V.Value = 0; V.Provenance.Status = ELHProvenanceStatus::Prototype;
    TestEqual(TEXT("resolved prototype zero explicit"), FLHUIPresenter::Format(V), FString(TEXT("0 Prototype")));
    TestEqual(TEXT("720 uniform scale"), FLHUIStyle::Scale(FVector2D(1280,720)), 2.f/3.f);
    return true;
}
// These tests construct Slate trees, then send the same key events used by a gamepad.
// They do not claim physical-device coverage or rendered readability.
namespace
{
void Pad(const TUniquePtr<ILHUIWidgetHarness>& W, ELHUITestKey Key, bool Repeat = false) { W->Key(Key,Repeat); }
void Reach(const TUniquePtr<ILHUIWidgetHarness>& W, FLHUIPresenter& P, FName Id)
{
    for (int32 I=0; I<P.FocusOrder().Num() && P.FocusedControl()!=Id; ++I) Pad(W,ELHUITestKey::Down);
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHUIWidgetConstructionTest,"Lighthaven.UI.NativeScreenConstructionAndGamepad",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHUIWidgetConstructionTest::RunTest(const FString&)
{
    FOwners O; FLHUIPresenter P(O,O,O);
    const auto W = ILHUIWidgetHarness::Create(P);
    const TArray<ELHUIScreen> Screens = {ELHUIScreen::Frontend,ELHUIScreen::Characters,ELHUIScreen::Creation,ELHUIScreen::CharacterSheet,ELHUIScreen::Inventory};
    const TArray<FName> Confirm = {"New","Continue","Confirm","Confirm","Confirm"};
    for (int32 I=0; I<Screens.Num(); ++I)
    {
        W->Open(Screens[I]);
        for (FName Id : P.FocusOrder()) TestTrue(TEXT("native control exists without an asset"), W->HasControl(Id));
        Reach(W,P,Confirm[I]); TestTrue(TEXT("gamepad reaches confirm control"), P.FocusedControl()==Confirm[I]);
    }
    TestTrue(TEXT("quiver widget exists"),W->HasControl("Quiver"));
    W->Open(ELHUIScreen::Creation); W->Activate("Name");
    TestTrue(TEXT("gamepad name keyboard exists"),W->HasControl("AppendLetter"));
    Pad(W,ELHUITestKey::South); // default focus appends A
    TestEqual(TEXT("pad keyboard edits presenter form"),P.NameInput(),FString(TEXT("A")));
    Pad(W,ELHUITestKey::East);
    TestFalse(TEXT("back closes keyboard"),W->IsModal());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHUIWidgetSubmitTest,"Lighthaven.UI.WidgetRejectedFieldsAndDuplicateSubmit",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHUIWidgetSubmitTest::RunTest(const FString&)
{
    FOwners O; FLHUIPresenter P(O,O,O); const auto W = ILHUIWidgetHarness::Create(P);
    W->Open(ELHUIScreen::Creation); W->EditName(TEXT("Keep this name")); W->Activate("Roll");
    Reach(W,P,"Confirm"); Pad(W,ELHUITestKey::South);
    TestTrue(TEXT("creation confirms in modal"),W->IsModal());
    TestFalse(TEXT("background confirm removed from focus tree"),W->HasControl("Confirm"));
    Pad(W,ELHUITestKey::Down); Pad(W,ELHUITestKey::South);
    TestEqual(TEXT("reject executes once"),O.Creates,1);
    TestEqual(TEXT("actual input field survives rejection"),W->FieldName(),FString(TEXT("Keep this name")));
    TestTrue(TEXT("presenter error survives widget rebuild"),!P.Error().IsEmpty());
    O.bAccept = true; Pad(W,ELHUITestKey::South); Pad(W,ELHUITestKey::Down); Pad(W,ELHUITestKey::South);
    Pad(W,ELHUITestKey::South,true); Pad(W,ELHUITestKey::South);
    TestEqual(TEXT("accepted duplicate confirm submits once"),O.Creates,2);
    W->Activate("Quit"); Pad(W,ELHUITestKey::Down); Pad(W,ELHUITestKey::South);
    TestEqual(TEXT("creation latch cannot swallow confirmed quit"),O.Exits,1);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHUIWidgetAllocationTest,"Lighthaven.UI.WidgetAllocationEquipmentAndRecovery",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHUIWidgetAllocationTest::RunTest(const FString&)
{
    FOwners O; O.State.Header.CharacterId.Value = FGuid::NewGuid();
    FLHItemInstance Item; Item.Id.InstanceId = FGuid::NewGuid(); Item.Definition.Value = TEXT("Fixture.Quiver"); O.State.Character.Inventory.Add(Item);
    FLHUIPresenter P(O,O,O); const auto W = ILHUIWidgetHarness::Create(P);
    W->Open(ELHUIScreen::CharacterSheet); W->Activate("Strength"); W->Activate("Confirm");
    Pad(W,ELHUITestKey::Down); Pad(W,ELHUITestKey::South);
    TestEqual(TEXT("staged strength reaches owner"),O.LastAllocation.Strength.Value,int64(1));
    TestTrue(TEXT("unchanged wisdom explicitly resolved zero"),O.LastAllocation.Wisdom.Resolution == ELHValueResolution::Resolved && O.LastAllocation.Wisdom.Value == 0);
    TestEqual(TEXT("canonical stats unchanged by UI"),O.State.Character.BaseAttributes.Strength.Value,int64(0));
    W->Activate("Confirm"); Pad(W,ELHUITestKey::Down); Pad(W,ELHUITestKey::South);
    TestEqual(TEXT("rejected allocation draft retained"),O.LastAllocation.Strength.Value,int64(1));
    W->Activate("Reset");
    TestTrue(TEXT("reset clears allocation rejection and summary"),W->MessageText().IsEmpty());
    O.bLegalAllocationReview=false; W->Activate("Strength"); W->Activate("Confirm");
    TestTrue(TEXT("authority review rejection displayed"),W->MessageText().Contains(TEXT("Allocation rejected by character authority")));
    W->Activate("Reset"); TestTrue(TEXT("reset clears rejected review summary"),W->MessageText().IsEmpty());
    O.bLegalAllocationReview=true;
    W->Activate("Strength");
    O.bAccept = true; W->Activate("Confirm"); Pad(W,ELHUITestKey::Down); Pad(W,ELHUITestKey::South);
    TestTrue(TEXT("accepted allocation has no stale local acceptance"),W->MessageText().IsEmpty());
    W->Activate("Confirm"); Pad(W,ELHUITestKey::South,true);
    TestEqual(TEXT("accepted allocation duplicate suppressed"),O.Allocates,3);
    Pad(W,ELHUITestKey::Inventory);
    TestTrue(TEXT("I switches character to inventory"),P.Screen()==ELHUIScreen::Inventory);
    Pad(W,ELHUITestKey::Character);
    TestTrue(TEXT("C switches inventory to character"),P.Screen()==ELHUIScreen::CharacterSheet);
    Pad(W,ELHUITestKey::Character,true); TestEqual(TEXT("held tab key does not close"),O.Resumes,0);
    Pad(W,ELHUITestKey::Character); TestEqual(TEXT("current C closes through resume"),O.Resumes,1);
    Pad(W,ELHUITestKey::Inventory); Pad(W,ELHUITestKey::Inventory); TestEqual(TEXT("current I closes through resume"),O.Resumes,2);
    W->Open(ELHUIScreen::Inventory); W->Activate("Items"); W->Activate("Quiver"); W->Activate("Confirm");
    Pad(W,ELHUITestKey::Down); Pad(W,ELHUITestKey::South); W->Activate("Confirm");
    TestEqual(TEXT("quiver equipment submits once"),O.Equips,1);
    TestTrue(TEXT("exact selected stable identity and quiver slot"),O.LastEquipment.Item.InstanceId == Item.Id.InstanceId && O.LastEquipment.Slot == ELHEquipmentSlot::Quiver);
    // A separate screen host models a fresh session adapter after a transition.
    FLHUIPresenter RecoveryP(O,O,O); const auto Recovery = ILHUIWidgetHarness::Create(RecoveryP);
    Recovery->Open(ELHUIScreen::Characters); Recovery->Activate("Profiles"); Recovery->Activate("Continue");
    Pad(Recovery,ELHUITestKey::Down); Pad(Recovery,ELHUITestKey::South);
    TestEqual(TEXT("unacknowledged recovery cannot load"),O.Loads,0);
    Recovery->Activate("Recovery"); Recovery->Activate("Continue"); Pad(Recovery,ELHUITestKey::Down); Pad(Recovery,ELHUITestKey::South);
    Recovery->Activate("Continue"); Pad(Recovery,ELHUITestKey::Down); Pad(Recovery,ELHUITestKey::South);
    TestEqual(TEXT("acknowledged continue duplicate suppressed"),O.Loads,1);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHUIUnreadableFocusTest,"Lighthaven.UI.UnreadableContinueAndStaleErrors",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHUIUnreadableFocusTest::RunTest(const FString&)
{
    FOwners O; O.State.Header.CharacterId.Value=FGuid::NewGuid(); O.bReadable=false;
    FLHUIPresenter P(O,O,O); const auto W=ILHUIWidgetHarness::Create(P);
    W->Open(ELHUIScreen::Characters); W->Activate("Profiles");
    TestFalse(TEXT("unreadable Continue disabled"),P.IsControlEnabled("Continue"));
    TestFalse(TEXT("unreadable recovery disabled"),P.IsControlEnabled("Recovery"));
    for (int32 I=0; I<P.FocusOrder().Num(); ++I)
    { TestTrue(TEXT("unreadable Continue skipped by navigation"),P.FocusedControl()!="Continue"); Pad(W,ELHUITestKey::Down); }
    W->Activate("Continue"); TestFalse(TEXT("disabled activation cannot open review"),W->IsModal());
    TestEqual(TEXT("disabled activation never loads"),O.Loads,0);
    O.bReadable=true; P.Refresh(); W->Activate("Continue");
    Pad(W,ELHUITestKey::Down); Pad(W,ELHUITestKey::South);
    TestTrue(TEXT("unacknowledged recovery shows error"),!P.Error().IsEmpty());
    W->Activate("Recovery"); TestTrue(TEXT("acknowledgment clears stale error"),P.Error().IsEmpty());
    P.Continue(false); P.SelectProfile(O.State.Header.CharacterId);
    TestTrue(TEXT("selection clears stale error"),P.Error().IsEmpty());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHUIAsyncSaveStatus,"Lighthaven.UI.AsyncSaveStatus",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHUIAsyncSaveStatus::RunTest(const FString&)
{
    FOwners O; O.bOwnsSaveStatus=true; O.SaveStatus=TEXT("Save failed: fixture");
    FLHUIPresenter P(O,O,O); const auto W=ILHUIWidgetHarness::Create(P);
    W->Activate("RetrySave");
    TestTrue(TEXT("Failure visible until owner completes retry"),W->MessageText().Contains(TEXT("Save failed")));
    O.SaveStatus.Empty(); P.Refresh();
    TestTrue(TEXT("Async owner durability clears copied retry failure"),W->MessageText().IsEmpty());
    O.SaveStatus=TEXT("Arrival refused; save retained: fixture");
    TestTrue(TEXT("Transition failure remains visible"),W->MessageText().Contains(TEXT("Arrival refused")));
    return true;
}
#endif
