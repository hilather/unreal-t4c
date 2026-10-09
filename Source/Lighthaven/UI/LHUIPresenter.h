#pragma once
#include "Core/LHCommands.h"

// Presentation-only seams. Owner adapters supply copies; no mutation API is exposed.
struct FLHUIProfile
{
    FLHCharacterId Id;
    FString Name;
    FString Status;
    bool bCanContinue = false;
    bool bRequiresRecoveryAcknowledgment = false;
};
struct FLHUICreationPreview
{
    FGuid Token;
    FLHCreationRecord Record;
    TMap<FName, FString> FieldErrors;
    bool bLegal = false;
};
struct FLHUIIntentReview
{
    bool bLegal = false;
    FString Summary = TEXT("Unavailable: authoritative preview has not been connected.");
};
struct FLHUIQuestion
{
    FLHContentId Id;
    FString Prompt;
    TArray<FLHContentId> Answers;
};
class LIGHTHAVEN_API ILHUIReadOwner
{
public:
    virtual ~ILHUIReadOwner() = default;
    virtual FString OwnerStatus() const { return {}; }
    virtual bool OwnsPersistenceStatus() const { return false; }
    virtual bool HasUnsavedChanges() const { return false; }
    virtual FString DerivedSummary() const { return TEXT("Effective attributes / gear effects: — (not available in this prototype)"); }
    virtual void ClearSelectionError() {}
    virtual bool BeginCreation() { return true; }
    virtual FLHSaveSnapshot Snapshot() const = 0;
    virtual TArray<FLHUIProfile> Profiles() const = 0;
    virtual TArray<FLHContentId> AppearanceCatalog() const = 0;
    virtual FLHUIIntentReview ReviewAllocation(const FLHAttributeBlock&) const { return {}; }
    virtual FLHUIIntentReview ReviewEquipment(const FLHEntityId&, ELHEquipmentSlot, bool) const { return {}; }
    virtual TArray<FLHUIQuestion> QuestionCatalog() const { return {}; }
    virtual FLHUICreationPreview Preview(const FString& Name, const TArray<FLHContentId>& Appearance,
        const TArray<FLHQuestionAnswer>& Answers, bool bReroll) = 0;
};
class LIGHTHAVEN_API ILHUISessionOwner
{
public:
    virtual ~ILHUISessionOwner() = default;
    // Session failures are separate from Core command reasons; empty string means accepted.
    virtual FString Continue(FLHCharacterId Character, bool bAcknowledgeRecovery) = 0;
    virtual FString RequestExit() = 0;
    virtual bool ResumeGameplay() { return false; }
    virtual FString RetryPersistence() { return TEXT("Save retry unavailable."); }
};
enum class ELHUIScreen : uint8 { Frontend, Characters, Creation, CharacterSheet, Inventory, Settings };

class LIGHTHAVEN_API FLHUIPresenter
{
public:
    FLHUIPresenter(ILHCommandHandler& InCommands, ILHUIReadOwner& InRead, ILHUISessionOwner& InSession);
    void Open(ELHUIScreen Screen);
    void MoveFocus(int32 Delta);
    FName FocusedControl() const;
    const TArray<FName>& FocusOrder() const { return Controls; }
    ELHUIScreen Screen() const { return ActiveScreen; }
    void Refresh();
    FLHUIIntentReview ReviewAllocation(const FLHAttributeBlock& Deltas) const { return Read.ReviewAllocation(Deltas); }
    FLHUIIntentReview ReviewEquipment(const FLHEntityId& Item, ELHEquipmentSlot Slot, bool bUnequip) const { return Read.ReviewEquipment(Item,Slot,bUnequip); }
    TArray<FLHContentId> AppearanceCatalog() const { return Read.AppearanceCatalog(); }
    TArray<FLHUIQuestion> QuestionCatalog() const { return Read.QuestionCatalog(); }
    const TArray<FLHContentId>& AppearanceInput() const { return AppearanceIds; }
    const TArray<FLHQuestionAnswer>& AnswerInput() const { return QuestionAnswers; }
    const FLHSaveSnapshot& Snapshot() const { return View; }
    const TArray<FLHUIProfile>& Profiles() const { return ProfileView; }
    FString Error() const { const FString Status=Read.OwnerStatus(); return Status.IsEmpty()?Message:Status; }
    void RetryPersistence() { Message=Session.RetryPersistence(); if (Read.OwnsPersistenceStatus()) Message.Empty(); }
    bool HasUnsavedChanges() const { return Read.HasUnsavedChanges(); }
    FString DerivedSummary() const { return Read.DerivedSummary(); }
    bool IsPending() const { return bPending; }
    bool IsConfirmed() const { return bConfirmed; }

    // Form edits are staged locally. Edits invalidate prior preview, never alter View.
    bool EditCreation(FString Name, TArray<FLHContentId> Appearance, TArray<FLHQuestionAnswer> Answers);
    void Roll(bool bReroll);
    const FString& NameInput() const { return DisplayName; }
    const FLHUICreationPreview& CreationPreview() const { return Preview; }
    FLHCommandResult ConfirmCreation();
    FLHCommandResult Allocate(const FLHAttributeBlock& Deltas);
    FLHCommandResult Equip(const FLHEntityId& Item, ELHEquipmentSlot Slot, bool bUnequip);
    void SelectProfile(FLHCharacterId Id);
    bool IsControlEnabled(FName Id) const;
    void ClearSelectionError() { Message.Empty(); Read.ClearSelectionError(); }
    bool Continue(bool bAcknowledgeRecovery);
    bool Quit();
    bool ResumeGameplay() { return Session.ResumeGameplay(); }
    static FString Format(const FLHInteger& Value);
    static FString Format(const FLHNumber& Value);
    static FString Reason(ELHCommandReason Value);
private:
    FLHCommandResult Unavailable(ELHCommandReason Reason) const;
    void Apply(const FLHCommandResult& Result);
    ILHCommandHandler& Commands;
    ILHUIReadOwner& Read;
    ILHUISessionOwner& Session;
    ELHUIScreen ActiveScreen = ELHUIScreen::Frontend;
    TArray<FName> Controls;
    int32 Focus = 0;
    TMap<ELHUIScreen, FName> RetainedFocus;
    FLHSaveSnapshot View;
    TArray<FLHUIProfile> ProfileView;
    FLHCharacterId SelectedProfile;
    FString DisplayName, Message;
    TArray<FLHContentId> AppearanceIds;
    TArray<FLHQuestionAnswer> QuestionAnswers;
    FLHUICreationPreview Preview;
    bool bPending = false, bConfirmed = false;
    FLHCommandResult Confirmation;
};
