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
// Views carry owner-authored prices and eligibility; UI performs no rule calculation.
struct FLHUIAbility { FLHContentId Id; FString Label, Feedback; bool bAvailable=false; bool bTargetsSelf=false; };
struct FLHUIHud
{
    FLHNumber Health, MaxHealth, Mana, MaxMana, TargetHealth, TargetMaxHealth;
    FLHEntityId Player;
    FString TargetName, Objective, SaveStatus, CompletionNotice;
    bool bDead=false;
    TArray<FLHUIAbility> Abilities;
};
struct FLHUIDialogueTopic { FLHContentId Id; FString Label, Text, Feedback; bool bEnabled=true; };
enum class ELHUIOfferKind : uint8 { Train, Learn, Buy, Sell };
struct FLHUIServiceOffer
{
    ELHUIOfferKind Kind=ELHUIOfferKind::Buy;
    FLHContentId Id; FLHEntityId Item; FLHInteger Quantity;
    FString Label, Price, Feedback; bool bEnabled=false;
};
struct FLHUILootRow
{
    ELHLootTransferKind Kind=ELHLootTransferKind::Unspecified;
    FLHEntityId Item; FLHInteger Quantity; FString Label;
};
class LIGHTHAVEN_API ILHUIReadOwner
{
public:
    virtual ~ILHUIReadOwner() = default;
    virtual FLHUIHud HudState() const { return {}; }
    virtual TArray<FLHUIAbility> AbilityCatalog() const { return HudState().Abilities; }
    virtual FString DialogueName(const FLHEntityId&) const { return {}; }
    virtual TArray<FLHUIDialogueTopic> DialogueTopics(const FLHEntityId&) const { return {}; }
    virtual TArray<FLHUIServiceOffer> ServiceOffers(const FLHEntityId&) const { return {}; }
    virtual TArray<FLHUILootRow> CorpseContents(const FLHEntityId&) const { return {}; }
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
    virtual FString RequestRespawn() { return TEXT("Respawn unavailable."); }
    virtual void SetGameplayPaused(bool) {}
    virtual bool ResumeGameplay() { return false; }
    virtual FString RetryPersistence() { return TEXT("Save retry unavailable."); }
};
enum class ELHUIScreen : uint8 { Frontend, Characters, Creation, CharacterSheet, Inventory, Settings, Hud, Dialogue, Services, Loot, Death, Pause };

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
    void OpenTarget(ELHUIScreen Screen, const FLHEntityId& Target);
    FLHUIHud Hud() const { return Read.HudState(); }
    TArray<FLHUIAbility> AbilityCatalog() const { return Read.AbilityCatalog(); }
    FString GameplaySummary() const;
    FString GameplayLabel(FName Control) const;
    bool ActivateGameplay(FName Control);
    void SelectAbility(int32 Slot);
    FLHContentId SelectedAbility() const;
    FLHCommandResult UseItem(const FLHEntityId& Item);
    FLHCommandResult UseHotbarItem();
    void AssignHotbarItem(const FLHEntityId& Item) { HotbarItem=Item; }
    FLHCommandResult UseAbility(const FLHEntityId& Target);
    FLHCommandResult RetryCommand();
    void SetPaused(bool Value) { Session.SetGameplayPaused(Value); }
    FString Respawn() { bMessageError=true; Message=Session.RequestRespawn(); return Message; }

    FLHUIIntentReview ReviewAllocation(const FLHAttributeBlock& Deltas) const { return Read.ReviewAllocation(Deltas); }
    FLHUIIntentReview ReviewEquipment(const FLHEntityId& Item, ELHEquipmentSlot Slot, bool bUnequip) const { return Read.ReviewEquipment(Item,Slot,bUnequip); }
    TArray<FLHContentId> AppearanceCatalog() const { return Read.AppearanceCatalog(); }
    TArray<FLHUIQuestion> QuestionCatalog() const { return Read.QuestionCatalog(); }
    const TArray<FLHContentId>& AppearanceInput() const { return AppearanceIds; }
    const TArray<FLHQuestionAnswer>& AnswerInput() const { return QuestionAnswers; }
    const FLHSaveSnapshot& Snapshot() const { return View; }
    const TArray<FLHUIProfile>& Profiles() const { return ProfileView; }
    bool HasError() const { return !Message.IsEmpty() && bMessageError; }
    FString FeedbackText() const { const FString Text=Error(); return HasError()?TEXT("Error: ")+Text:Text; }
    static FString DisplayItemNames(FString Text);
    static FString ItemName(const FLHContentId& Id);
    FString EquippedItemName(const FLHEntityId& Id) const;
    FString Error() const { const FString Status=Read.OwnerStatus(); return Status.IsEmpty()?Message:Message.IsEmpty()?Status:Message+TEXT("\n")+Status; }
    void RetryPersistence() { bMessageError=true; Message=Session.RetryPersistence(); if (Read.OwnsPersistenceStatus()) Message.Empty(); }
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
    FLHEntityId InteractionTarget, HotbarItem;
    int32 AbilitySlot=0;
    FLHContentId InspectedAbility;
    TMap<int32,FLHContentId> AbilityBindings;
    FLHUIAbility SelectedAbilityView() const;
    TFunction<FLHCommandResult()> Retry;
    FLHRequestId FreshRequest() const;
    FLHCommandResult Submit(TFunction<FLHCommandResult()> Command);
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
    bool bMessageError = true;
    FLHEntranceId FeedbackEntrance;
    bool bPending = false, bConfirmed = false;
    FLHCommandResult Confirmation;
};
