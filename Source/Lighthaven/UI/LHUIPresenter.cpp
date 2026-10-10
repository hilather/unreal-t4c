#include "UI/LHUIPresenter.h"
#include "Data/Items/LHItemCatalog.h"

FLHUIPresenter::FLHUIPresenter(ILHCommandHandler& C, ILHUIReadOwner& R, ILHUISessionOwner& S)
    : Commands(C), Read(R), Session(S) { Refresh(); Open(ELHUIScreen::Frontend); }
void FLHUIPresenter::Open(ELHUIScreen S)
{
    if (bPending) return;
    if (!Controls.IsEmpty()) RetainedFocus.Add(ActiveScreen, FocusedControl());
    if (S==ELHUIScreen::Creation && ActiveScreen!=S)
    {
        if (!Read.BeginCreation()) { bMessageError=true; Message=TEXT("Finish saving the current character first."); return; }
        bConfirmed=false; Confirmation={}; Preview={}; DisplayName.Empty(); AppearanceIds.Empty(); QuestionAnswers.Empty(); Refresh();
    }
    if (S!=ActiveScreen && (ActiveScreen==ELHUIScreen::Dialogue || ActiveScreen==ELHUIScreen::Services || S==ELHUIScreen::Dialogue)) Message.Empty();
    ActiveScreen = S;
    switch (S)
    {
    case ELHUIScreen::Frontend: Controls = {"New", "Continue", "Characters", "Settings", "Quit"}; break;
    case ELHUIScreen::Characters: Controls = {"Profiles", "Details", "Recovery", "Continue", "Back"}; break;
    case ELHUIScreen::Creation: Controls = {"Name", "Body", "Hair", "Skin", "Outfit", "Question1", "Question2", "Question3", "Question4", "Roll", "Reroll", "Review", "Confirm", "Back"}; break;
    case ELHUIScreen::CharacterSheet: Controls = {"CharacterTab", "InventoryTab", "Strength", "Endurance", "Agility", "Intelligence", "Wisdom", "Details", "Reset", "Confirm", "Back"}; break;
    case ELHUIScreen::Inventory: Controls = {"CharacterTab", "InventoryTab", "Items", "Head", "Torso", "MainHand", "OffHand", "Legs", "Feet", "Accessory", "Quiver", "Details", "Equip", "Unequip", "Use", "AssignItem", "Confirm", "Back"}; break;
    case ELHUIScreen::Hud:
        Controls.Empty(); for(int32 I=0; I<Read.AbilityCatalog().Num(); ++I) Controls.Add(FName(*FString::Printf(TEXT("Spell%d"),I)));
        Controls.Append({"Ability1","Ability2","Ability3","Ability4","Ability5","Ability6","AssignAbility","UseHotbarItem","Back"}); break;
    case ELHUIScreen::Dialogue:
        Controls.Empty(); for(int32 I=0; I<Read.DialogueTopics(InteractionTarget).Num(); ++I) Controls.Add(FName(*FString::Printf(TEXT("Topic%d"),I))); Controls.Add("Back"); break;
    case ELHUIScreen::Services:
        Controls.Empty(); for(int32 I=0; I<Read.ServiceOffers(InteractionTarget).Num(); ++I) Controls.Add(FName(*FString::Printf(TEXT("Offer%d"),I))); Controls.Add("Back"); break;
    case ELHUIScreen::Loot:
        Controls.Empty(); for(int32 I=0; I<Read.CorpseContents(InteractionTarget).Num(); ++I) Controls.Add(FName(*FString::Printf(TEXT("Loot%d"),I))); Controls.Add("Back"); break;
    case ELHUIScreen::Death: Controls={"Respawn","Back"}; break;
    case ELHUIScreen::Pause: Controls={"Resume","Settings","Back"}; break;
    case ELHUIScreen::Settings: Controls = {"Volume", "Controls", "Apply", "Revert", "Back"}; break;
    }
    Controls.AddUnique(TEXT("RetryCommand"));
    Controls.AddUnique(TEXT("Quit"));
    Controls.Add(TEXT("RetrySave"));
    Focus = 0;
    if (const FName* Prior = RetainedFocus.Find(S))
    {
        const int32 Index = Controls.IndexOfByKey(*Prior);
        if (Index != INDEX_NONE) Focus = Index;
    }
}
void FLHUIPresenter::MoveFocus(int32 Delta)
{
    const int32 Direction=Delta<0?-1:1;
    Focus=FMath::Clamp(Focus+Delta,0,Controls.Num()-1);
    while (Controls.IsValidIndex(Focus) && !IsControlEnabled(Controls[Focus]))
    {
        const int32 Next=Focus+Direction;
        if (!Controls.IsValidIndex(Next)) break;
        Focus=Next;
    }
}
bool FLHUIPresenter::IsControlEnabled(FName Id) const
{
    if ((Id!="Continue" && Id!="Recovery") || ActiveScreen!=ELHUIScreen::Characters) return true;
    const auto* Profile=ProfileView.FindByPredicate([this](const auto& V){ return V.Id.Value==SelectedProfile.Value; });
    if (Id=="Recovery") return Profile && Profile->bCanContinue && Profile->bRequiresRecoveryAcknowledgment;
    return !Profile || Profile->bCanContinue;
}
FName FLHUIPresenter::FocusedControl() const { return Controls.IsValidIndex(Focus) ? Controls[Focus] : NAME_None; }
void FLHUIPresenter::Refresh()
{
    View = Read.Snapshot(); ProfileView = Read.Profiles();
    if (FeedbackEntrance.Area.Content.Value!=View.Character.ActiveEntrance.Area.Content.Value ||
        FeedbackEntrance.LocalId!=View.Character.ActiveEntrance.LocalId) Message.Empty();
    FeedbackEntrance=View.Character.ActiveEntrance;
}
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
    bMessageError=true;
    Preview = Read.Preview(DisplayName, AppearanceIds, QuestionAnswers, bReroll);
    bPending = false;
    Message = Preview.bLegal ? FString() : TEXT("Unavailable: creation requires authoritative legal review.");
    if (!Preview.bLegal) for (const auto& E:Preview.FieldErrors) Message += TEXT("\n")+E.Key.ToString()+TEXT(": ")+E.Value;
}
FLHCommandResult FLHUIPresenter::Unavailable(ELHCommandReason R) const
{
    FLHCommandResult Result; Result.Reason = R; return Result;
}
void FLHUIPresenter::Apply(const FLHCommandResult& R)
{
    bPending = false;
    bMessageError = R.Disposition != ELHCommandDisposition::Accepted;
    Message = R.Disposition == ELHCommandDisposition::Accepted ? FString() : Reason(R.Reason);
    Refresh();
    if(ActiveScreen==ELHUIScreen::Dialogue || ActiveScreen==ELHUIScreen::Services || ActiveScreen==ELHUIScreen::Loot) Open(ActiveScreen);
    // Refresh only the view; rejected forms and preview remain staged.
}
FLHCommandResult FLHUIPresenter::ConfirmCreation()
{
    if (bConfirmed) return Confirmation;
    if (bPending) return Unavailable(ELHCommandReason::Busy);
    if (!Preview.bLegal || !Preview.Token.IsValid() || !View.Session.RequestEpoch.IsValid())
    {
        bMessageError=true; Message = Reason(ELHCommandReason::UnresolvedRules);
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
void FLHUIPresenter::SelectProfile(FLHCharacterId Id) { if (!bPending) { SelectedProfile = Id; ClearSelectionError(); MoveFocus(0); } }
bool FLHUIPresenter::Continue(bool bAcknowledgeRecovery)
{
    if (bPending) return false;
    bMessageError=true;
    Refresh();
    const auto* Profile = ProfileView.FindByPredicate([this](const auto& P) { return P.Id.Value == SelectedProfile.Value; });
    if (!SelectedProfile.Value.IsValid() || !Profile || !Profile->bCanContinue) { Message = Profile && !Profile->bCanContinue ? TEXT("Unreadable: ")+Profile->Status : TEXT("Unavailable: select a validated character."); return false; }
    if (Profile->bRequiresRecoveryAcknowledgment && !bAcknowledgeRecovery)
    { Message = TEXT("Recovery: acknowledge loading the earlier save before continuing."); return false; }
    bPending = true; Message = Session.Continue(SelectedProfile, bAcknowledgeRecovery); bPending = false;
    return Message.IsEmpty();
}
bool FLHUIPresenter::Quit()
{
    if (bPending) return false;
    bMessageError=true; bPending = true; Message = Session.RequestExit(); bPending = false; return Message.IsEmpty();
}
FString FLHUIPresenter::Format(const FLHInteger& V)
{
    if (V.Resolution != ELHValueResolution::Resolved) return TEXT("— (not available in this prototype)");
    FString S = LexToString(V.Value);
    if (V.Provenance.Status == ELHProvenanceStatus::Prototype) S += TEXT(" Prototype");
    if (V.Provenance.Status == ELHProvenanceStatus::Disputed) S += TEXT(" Disputed");
    return S;
}
FString FLHUIPresenter::Format(const FLHNumber& V)
{
    if (V.Resolution != ELHValueResolution::Resolved) return TEXT("— (not available in this prototype)");
    FString S = FString::SanitizeFloat(V.Value);
    if (V.Provenance.Status == ELHProvenanceStatus::Prototype) S += TEXT(" Prototype");
    if (V.Provenance.Status == ELHProvenanceStatus::Disputed) S += TEXT(" Disputed");
    return S;
}
FString FLHUIPresenter::Reason(ELHCommandReason V)
{
    switch (V)
    {
    case ELHCommandReason::InventoryFull: return TEXT("Inventory full. Make room and try again.");
    case ELHCommandReason::InsufficientMana: return TEXT("Not enough mana.");
    case ELHCommandReason::InsufficientGold: return TEXT("Not enough gold.");
    case ELHCommandReason::Cooldown: return TEXT("Ability is cooling down.");
    case ELHCommandReason::OutOfRange: return TEXT("Move closer.");
    case ELHCommandReason::Obstructed: return TEXT("Line of sight is blocked.");
    case ELHCommandReason::InvalidLifeState: return TEXT("You must be alive to do this.");
    case ELHCommandReason::NoEffect: return TEXT("Already full; item was not consumed.");
    case ELHCommandReason::NotUsable: return TEXT("This item cannot be used.");
    case ELHCommandReason::UnresolvedRules: return TEXT("Unavailable: required rules data is unknown.");
    case ELHCommandReason::InsufficientPoints: return TEXT("Not enough points.");
    case ELHCommandReason::InvalidEquipment: return TEXT("Equipment does not support this action.");
    case ELHCommandReason::Ineligible: return TEXT("Requirements not met.");
    case ELHCommandReason::Busy: return TEXT("Another action is finishing.");
    case ELHCommandReason::SaveRequired: return TEXT("Progress must be saved first.");
    default: return TEXT("This action could not be validated. Your changes were not applied.");
    }
}

FLHRequestId FLHUIPresenter::FreshRequest() const { FLHRequestId R; R.Epoch=Read.Snapshot().Session.RequestEpoch; R.Value=FGuid::NewGuid(); return R; }
FLHCommandResult FLHUIPresenter::Submit(TFunction<FLHCommandResult()> C)
{
    if(bPending) return Unavailable(ELHCommandReason::Busy);
    Retry=MoveTemp(C); bPending=true; auto R=Retry(); Apply(R); return R;
}
FLHCommandResult FLHUIPresenter::RetryCommand() { if(!Retry || bPending) return Unavailable(ELHCommandReason::InvalidRequest); bPending=true; auto R=Retry(); Apply(R); return R; }
void FLHUIPresenter::OpenTarget(ELHUIScreen S,const FLHEntityId& Target) { Message.Empty(); InteractionTarget=Target; Open(S); }
void FLHUIPresenter::SelectAbility(int32 Slot) { if(Slot>=0 && Slot<6) AbilitySlot=Slot; }
FLHUIAbility FLHUIPresenter::SelectedAbilityView() const
{
    auto H=Hud(); if(const auto* Binding=AbilityBindings.Find(AbilitySlot)) { auto Catalog=Read.AbilityCatalog(); if(const auto* A=Catalog.FindByPredicate([&](const auto& Row){return Row.Id.Value==Binding->Value;})) return *A; return {}; }
    return H.Abilities.IsValidIndex(AbilitySlot)?H.Abilities[AbilitySlot]:FLHUIAbility{};
}
FLHContentId FLHUIPresenter::SelectedAbility() const { return SelectedAbilityView().Id; }
FLHCommandResult FLHUIPresenter::UseAbility(const FLHEntityId& Target) { FLHUseAbilityRequest Q; Q.Request=FreshRequest(); Q.Ability=SelectedAbility(); const auto H=Hud(); Q.Target=SelectedAbilityView().bTargetsSelf?H.Player:Target; return Submit([this,Q](){return Commands.Execute(Q);}); }
FLHCommandResult FLHUIPresenter::UseItem(const FLHEntityId& Item) { FLHUseItemRequest Q; Q.Request=FreshRequest(); Q.Item=Item; return Submit([this,Q](){return Commands.Execute(Q);}); }
FLHCommandResult FLHUIPresenter::UseHotbarItem() { return UseItem(HotbarItem); }
FString FLHUIPresenter::GameplaySummary() const
{
    const auto H=Hud(); FString S=TEXT("Health ")+Format(H.Health)+TEXT(" / ")+Format(H.MaxHealth)+TEXT(" | Mana ")+Format(H.Mana)+TEXT(" / ")+Format(H.MaxMana);
    if (ActiveScreen==ELHUIScreen::Dialogue || ActiveScreen==ELHUIScreen::Services)
    {
        const auto Name=Read.DialogueName(InteractionTarget);
        S= (Name.IsEmpty()?TEXT("Dialogue"):Name)+TEXT("\n")+S;
    }
    S+=TEXT("\n")+Error();
    S+=TEXT("\n")+H.TargetName+TEXT(" ")+Format(H.TargetHealth)+TEXT(" / ")+Format(H.TargetMaxHealth)+TEXT("\n")+H.Objective+TEXT("\n")+H.SaveStatus+TEXT("\n")+H.CompletionNotice;
    if(ActiveScreen==ELHUIScreen::Death) S+=TEXT("\nYou have fallen. Return to the church.");
    if(ActiveScreen==ELHUIScreen::Pause) S+=TEXT("\nPaused");
    return S;
}
FString FLHUIPresenter::GameplayLabel(FName Control) const
{
    const FString S=Control.ToString();
    if(S.StartsWith(TEXT("Ability"))) { const int32 I=FCString::Atoi(*S.Mid(7))-1; auto H=Hud(); FLHUIAbility A; if(H.Abilities.IsValidIndex(I)) A=H.Abilities[I]; if(const auto* Binding=AbilityBindings.Find(I)) { auto C=Read.AbilityCatalog(); if(const auto* V=C.FindByPredicate([&](const auto& Row){return Row.Id.Value==Binding->Value;})) A=*V; } return (I==AbilitySlot?TEXT("> "):TEXT(""))+FString::FromInt(I+1)+TEXT(" ")+(A.Id.Value.IsNone()?TEXT("Empty"):A.Label+TEXT(" | ")+A.Feedback); }
    if(S.StartsWith(TEXT("Spell"))) { auto C=Read.AbilityCatalog(); int32 I=FCString::Atoi(*S.Mid(5)); if(C.IsValidIndex(I)) return C[I].Label+TEXT(" | ")+C[I].Feedback; }
    if(S.StartsWith(TEXT("Topic"))) { auto T=Read.DialogueTopics(InteractionTarget); const int32 I=FCString::Atoi(*S.Mid(5)); if(T.IsValidIndex(I)) return T[I].Label+TEXT(" | ")+T[I].Feedback; }
    if(S.StartsWith(TEXT("Offer"))) { auto T=Read.ServiceOffers(InteractionTarget); const int32 I=FCString::Atoi(*S.Mid(5)); if(T.IsValidIndex(I)) return DisplayItemNames(T[I].Label)+TEXT(" | ")+T[I].Price+TEXT(" | ")+T[I].Feedback; }
    if(S.StartsWith(TEXT("Loot"))) { auto T=Read.CorpseContents(InteractionTarget); const int32 I=FCString::Atoi(*S.Mid(4)); if(T.IsValidIndex(I)) return DisplayItemNames(T[I].Label)+TEXT(" x ")+Format(T[I].Quantity); }
    return S;
}
bool FLHUIPresenter::ActivateGameplay(FName Control)
{
    const FString S=Control.ToString();
    if(Control=="AssignAbility") { const auto C=Read.AbilityCatalog(); const auto* A=C.FindByPredicate([&](const auto& Row){return Row.Id.Value==InspectedAbility.Value;}); if(A && A->bAvailable) { AbilityBindings.Add(AbilitySlot,A->Id); bMessageError=false; Message=TEXT("Ability assigned to selected slot for this session."); } else { bMessageError=true; Message=TEXT("Select an available learned ability first."); } return true; }
    if(S.StartsWith(TEXT("Spell"))) { auto C=Read.AbilityCatalog(); int32 I=FCString::Atoi(*S.Mid(5)); if(C.IsValidIndex(I)) { InspectedAbility=C[I].Id; bMessageError=false; Message=C[I].Label+TEXT(" | ")+C[I].Feedback; } return true; }
    if(Control=="RetryCommand") { RetryCommand(); return true; }
    if(Control=="Respawn") { Respawn(); return true; }
    if(Control=="Resume") { SetPaused(false); ResumeGameplay(); return true; }
    if(Control=="UseHotbarItem") { UseHotbarItem(); return true; }
    if(S.StartsWith(TEXT("Ability"))) { SelectAbility(FCString::Atoi(*S.Mid(7))-1); return true; }
    if(S.StartsWith(TEXT("Topic")))
    {
        const auto T=Read.DialogueTopics(InteractionTarget); const int32 I=FCString::Atoi(*S.Mid(5)); if(!T.IsValidIndex(I)) return true; if(!T[I].bEnabled) { bMessageError=true; Message=T[I].Feedback; return true; }
        FLHInteractRequest Q; Q.Request=FreshRequest(); Q.Target=InteractionTarget; Q.Topic=T[I].Id;
        auto R=Submit([this,Q](){return Commands.Execute(Q);});
        if(R.Disposition==ELHCommandDisposition::Rejected && R.Reason==ELHCommandReason::NoEffect && Q.Topic.Value==TEXT("Topic.Heal")) Message=TEXT("You are already at full health.");
        if(R.Disposition==ELHCommandDisposition::Accepted) { bMessageError=false; Message=T[I].Text; if(Q.Topic.Value==TEXT("Topic.Services")) Open(ELHUIScreen::Services); }
        return true;
    }
    if(S.StartsWith(TEXT("Offer")))
    {
        auto T=Read.ServiceOffers(InteractionTarget); const int32 I=FCString::Atoi(*S.Mid(5)); if(!T.IsValidIndex(I)) return true; if(!T[I].bEnabled) { bMessageError=true; Message=T[I].Feedback; return true; } const auto O=T[I]; const auto ID=FreshRequest();
        switch(O.Kind)
        {
        case ELHUIOfferKind::Train: { FLHTrainSkillRequest Q; Q.Request=ID; Q.Trainer=InteractionTarget; Q.Skill=O.Id; Q.Points=O.Quantity; Submit([this,Q](){return Commands.Execute(Q);}); break; }
        case ELHUIOfferKind::Learn: { FLHLearnSpellRequest Q; Q.Request=ID; Q.Trainer=InteractionTarget; Q.Spell=O.Id; Submit([this,Q](){return Commands.Execute(Q);}); break; }
        case ELHUIOfferKind::Buy: { FLHBuyItemRequest Q; Q.Request=ID; Q.Vendor=InteractionTarget; Q.Offer=O.Id; Q.Quantity=O.Quantity; Submit([this,Q](){return Commands.Execute(Q);}); break; }
        case ELHUIOfferKind::Sell: { FLHSellItemRequest Q; Q.Request=ID; Q.Vendor=InteractionTarget; Q.Item=O.Item; Q.Quantity=O.Quantity; Submit([this,Q](){return Commands.Execute(Q);}); break; }
        } return true;
    }
    if(S.StartsWith(TEXT("Loot")))
    {
        auto T=Read.CorpseContents(InteractionTarget); const int32 I=FCString::Atoi(*S.Mid(4)); if(!T.IsValidIndex(I)) return true;
        FLHTakeLootRequest Q; Q.Request=FreshRequest(); Q.Container=InteractionTarget; Q.Kind=T[I].Kind; Q.Item=T[I].Item; Q.Quantity=T[I].Quantity;
        Submit([this,Q](){return Commands.Execute(Q);}); return true;
    }
    return false;
}

FString FLHUIPresenter::ItemName(const FLHContentId& Id)
{
    // Presentation labels keyed by the native item catalog, which has no display-name field yet.
    static const TMap<FName,FString> Names={
        {"Item.RustedDirk",TEXT("Rusted Dirk")},{"Item.AshwoodFlatbow",TEXT("Ashwood Flatbow")},
        {"Item.WoodenArrows",TEXT("Wooden Arrows")},{"Item.PotionOfMana",TEXT("Potion of Mana")},
        {"Item.ClothVest",TEXT("Cloth Vest")},{"Item.ClothPants",TEXT("Cloth Pants")},
        {"Item.Torch",TEXT("Torch")},{"Item.LightHeal",TEXT("Light Heal")},
        {"Item.DecayingBatWings",TEXT("Decaying Bat Wings")},{"Item.GoblinLeatherArmor",TEXT("Goblin Leather Armor")},
        {"Item.IronRing",TEXT("Iron Ring")},{"Item.GoblinBlade",TEXT("Goblin Blade")},
        {"Item.IronKey",TEXT("Iron Key")},{"Item.FlowingBlackRobe",TEXT("Flowing Black Robe")}};
    if (LHItemData::Find(Id)) if (const auto* Name=Names.Find(Id.Value)) return *Name;
    return TEXT("Unknown item");
}
FString FLHUIPresenter::EquippedItemName(const FLHEntityId& Id) const
{
    const auto* Item=View.Character.Inventory.FindByPredicate([&](const auto& I){
        return I.Id.RunId==Id.RunId && I.Id.Area.Content.Value==Id.Area.Content.Value && I.Id.InstanceId==Id.InstanceId;
    });
    return Item?ItemName(Item->Definition):TEXT("Unknown item");
}

FString FLHUIPresenter::DisplayItemNames(FString Text)
{
    for (const auto& Row:LHItemData::Catalog()) Text.ReplaceInline(*Row.Id.Value.ToString(),*ItemName(Row.Id),ESearchCase::CaseSensitive);
    return Text;
}
