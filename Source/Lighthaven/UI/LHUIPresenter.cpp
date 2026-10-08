#include "UI/LHUIPresenter.h"

FLHUIPresenter::FLHUIPresenter(ILHCommandHandler& C, ILHUIReadOwner& R, ILHUISessionOwner& S)
    : Commands(C), Read(R), Session(S) { Refresh(); Open(ELHUIScreen::Frontend); }
void FLHUIPresenter::Open(ELHUIScreen S)
{
    if (bPending) return;
    if (!Controls.IsEmpty()) RetainedFocus.Add(ActiveScreen, FocusedControl());
    if (S==ELHUIScreen::Creation && ActiveScreen!=S)
    {
        if (!Read.BeginCreation()) { Message=TEXT("Finish saving the current character first."); return; }
        bConfirmed=false; Confirmation={}; Preview={}; DisplayName.Empty(); AppearanceIds.Empty(); QuestionAnswers.Empty(); Refresh();
    }
    ActiveScreen = S;
    switch (S)
    {
    case ELHUIScreen::Frontend: Controls = {"New", "Continue", "Characters", "Settings", "Quit"}; break;
    case ELHUIScreen::Characters: Controls = {"Profiles", "Details", "Recovery", "Continue", "Back"}; break;
    case ELHUIScreen::Creation: Controls = {"Name", "Appearance", "Question1", "Question2", "Question3", "Question4", "Roll", "Reroll", "Review", "Confirm", "Back"}; break;
    case ELHUIScreen::CharacterSheet: Controls = {"CharacterTab", "InventoryTab", "Strength", "Endurance", "Agility", "Intelligence", "Wisdom", "Details", "Reset", "Confirm", "Back"}; break;
    case ELHUIScreen::Inventory: Controls = {"CharacterTab", "InventoryTab", "Items", "Head", "Torso", "MainHand", "OffHand", "Legs", "Feet", "Accessory", "Quiver", "Details", "Equip", "Unequip", "Confirm", "Back"}; break;
    case ELHUIScreen::Settings: Controls = {"Volume", "Controls", "Apply", "Revert", "Back"}; break;
    }
    Controls.AddUnique(TEXT("Quit"));
    Controls.Add(TEXT("RetrySave"));
    Focus = 0;
    if (const FName* Prior = RetainedFocus.Find(S))
    {
        const int32 Index = Controls.IndexOfByKey(*Prior);
        if (Index != INDEX_NONE) Focus = Index;
    }
}
void FLHUIPresenter::MoveFocus(int32 Delta) { Focus = FMath::Clamp(Focus + Delta, 0, Controls.Num() - 1); }
FName FLHUIPresenter::FocusedControl() const { return Controls.IsValidIndex(Focus) ? Controls[Focus] : NAME_None; }
void FLHUIPresenter::Refresh() { View = Read.Snapshot(); ProfileView = Read.Profiles(); }
bool FLHUIPresenter::EditCreation(FString N, TArray<FLHContentId> A, TArray<FLHQuestionAnswer> Q)
{
    if (bPending || bConfirmed) return false;
    DisplayName = MoveTemp(N); AppearanceIds = MoveTemp(A); QuestionAnswers = MoveTemp(Q);
    Preview = {}; Message.Empty(); return true;
}
void FLHUIPresenter::Roll(bool bReroll)
{
    if (bPending || bConfirmed) return;
    bPending = true;
    Preview = Read.Preview(DisplayName, AppearanceIds, QuestionAnswers, bReroll);
    bPending = false;
    Message = Preview.bLegal ? FString() : TEXT("Unavailable: creation requires authoritative legal review.");
}
FLHCommandResult FLHUIPresenter::Unavailable(ELHCommandReason R) const
{
    FLHCommandResult Result; Result.Reason = R; return Result;
}
void FLHUIPresenter::Apply(const FLHCommandResult& R)
{
    bPending = false;
    Message = R.Disposition == ELHCommandDisposition::Accepted ? FString() : Reason(R.Reason);
    Refresh(); // Refresh only the view; rejected forms and preview remain staged.
}
FLHCommandResult FLHUIPresenter::ConfirmCreation()
{
    if (bConfirmed) return Confirmation;
    if (bPending) return Unavailable(ELHCommandReason::Busy);
    if (!Preview.bLegal || !Preview.Token.IsValid() || !View.Session.RequestEpoch.IsValid())
    {
        Message = Reason(ELHCommandReason::UnresolvedRules);
        return Unavailable(ELHCommandReason::UnresolvedRules);
    }
    FLHCreateCharacterRequest R;
    R.Request.Epoch = View.Session.RequestEpoch; R.Request.Value = FGuid::NewGuid(); R.DisplayName = DisplayName; R.AppearanceIds = AppearanceIds;
    R.Creation = Preview.Record; R.PreviewToken = Preview.Token;
    bPending = true;
    const FLHCommandResult Result = Commands.Execute(R);
    // Synchronous contract: returned rejection is definitive. Accepted confirmation is latched.
    Confirmation = Result; bConfirmed = Result.Disposition == ELHCommandDisposition::Accepted;
    Apply(Result); return Result;
}
FLHCommandResult FLHUIPresenter::Allocate(const FLHAttributeBlock& Deltas)
{
    if (bPending) return Unavailable(ELHCommandReason::Busy);
    if (!View.Session.RequestEpoch.IsValid()) return Unavailable(ELHCommandReason::InvalidRequest);
    FLHAllocateAttributePointsRequest R; R.Request.Epoch = View.Session.RequestEpoch; R.Request.Value = FGuid::NewGuid(); R.Points = Deltas;
    bPending = true; const auto Result = Commands.Execute(R); Apply(Result); return Result;
}
FLHCommandResult FLHUIPresenter::Equip(const FLHEntityId& Item, ELHEquipmentSlot Slot, bool bUnequip)
{
    if (bPending) return Unavailable(ELHCommandReason::Busy);
    if (!View.Session.RequestEpoch.IsValid()) return Unavailable(ELHCommandReason::InvalidRequest);
    FLHEquipItemRequest R; R.Request.Epoch = View.Session.RequestEpoch; R.Request.Value = FGuid::NewGuid(); R.Item = Item; R.Slot = Slot; R.bUnequip = bUnequip;
    bPending = true; const auto Result = Commands.Execute(R); Apply(Result); return Result;
}
void FLHUIPresenter::SelectProfile(FLHCharacterId Id) { if (!bPending) SelectedProfile = Id; }
bool FLHUIPresenter::Continue(bool bAcknowledgeRecovery)
{
    if (bPending) return false;
    Refresh();
    const auto* Profile = ProfileView.FindByPredicate([this](const auto& P) { return P.Id.Value == SelectedProfile.Value; });
    if (!SelectedProfile.Value.IsValid() || !Profile || !Profile->bCanContinue) { Message = TEXT("Unavailable: select a validated character."); return false; }
    if (Profile->bRequiresRecoveryAcknowledgment && !bAcknowledgeRecovery)
    { Message = TEXT("Recovery: acknowledge loading the earlier save before continuing."); return false; }
    bPending = true; Message = Session.Continue(SelectedProfile, bAcknowledgeRecovery); bPending = false;
    return Message.IsEmpty();
}
bool FLHUIPresenter::Quit()
{
    if (bPending) return false;
    bPending = true; Message = Session.RequestExit(); bPending = false; return Message.IsEmpty();
}
FString FLHUIPresenter::Format(const FLHInteger& V)
{
    if (V.Resolution != ELHValueResolution::Resolved) return TEXT("— Unknown");
    FString S = LexToString(V.Value);
    if (V.Provenance.Status == ELHProvenanceStatus::Prototype) S += TEXT(" Prototype");
    if (V.Provenance.Status == ELHProvenanceStatus::Disputed) S += TEXT(" Disputed");
    return S;
}
FString FLHUIPresenter::Format(const FLHNumber& V)
{
    if (V.Resolution != ELHValueResolution::Resolved) return TEXT("— Unknown");
    FString S = FString::SanitizeFloat(V.Value);
    if (V.Provenance.Status == ELHProvenanceStatus::Prototype) S += TEXT(" Prototype");
    if (V.Provenance.Status == ELHProvenanceStatus::Disputed) S += TEXT(" Disputed");
    return S;
}
FString FLHUIPresenter::Reason(ELHCommandReason V)
{
    switch (V)
    {
    case ELHCommandReason::UnresolvedRules: return TEXT("Unavailable: required rules data is unknown.");
    case ELHCommandReason::InsufficientPoints: return TEXT("Not enough points.");
    case ELHCommandReason::InvalidEquipment: return TEXT("Equipment does not support this action.");
    case ELHCommandReason::Ineligible: return TEXT("Requirements not met.");
    case ELHCommandReason::Busy: return TEXT("Another action is finishing.");
    case ELHCommandReason::SaveRequired: return TEXT("Progress must be saved first.");
    default: return TEXT("This action could not be validated. Your changes were not applied.");
    }
}
