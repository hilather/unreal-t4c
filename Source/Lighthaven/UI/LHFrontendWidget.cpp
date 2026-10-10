#include "UI/LHFrontendWidget.h"
#include "UI/LHPresentation.h"
#include "Input/LHInputGlyphs.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Brushes/SlateColorBrush.h"
namespace LHFrontendWidgetPrivate
{
FLHInteger* Attribute(FLHAttributeBlock& A, FName Id)
{
    if (Id == "Strength") return &A.Strength;
    if (Id == "Endurance") return &A.Endurance;
    if (Id == "Agility") return &A.Agility;
    if (Id == "Intelligence") return &A.Intelligence;
    if (Id == "Wisdom") return &A.Wisdom;
    return nullptr;
}
FSlateFontInfo Font(float Size) { return FCoreStyle::GetDefaultFontStyle("Regular", Size); }
}
void SLHFrontendWidget::Construct(const FArguments& Args)
{
    P = Args._Presenter; check(P); FLHInputGlyphs::Initialize();
    ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
    ButtonStyle.SetNormal(FSlateColorBrush(Style.Raised)).SetHovered(FSlateColorBrush(Style.Raised))
        .SetPressed(FSlateColorBrush(Style.Panel)).SetDisabled(FSlateColorBrush(Style.Raised));
    ButtonStyle.SetNormalPadding(FMargin(Style.ControlInset)).SetPressedPadding(FMargin(Style.ControlInset));
    EditStyle = FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox");
    EditStyle.SetBackgroundImageNormal(FSlateColorBrush(Style.Raised)).SetBackgroundImageHovered(FSlateColorBrush(Style.Raised))
        .SetBackgroundImageFocused(FSlateColorBrush(Style.Raised)).SetForegroundColor(Style.TextPrimary);
    Appearance = P->AppearanceInput(); Answers = P->AnswerInput();
    for (FName Id : {"Strength", "Endurance", "Agility", "Intelligence", "Wisdom"})
    {
        auto* A = LHFrontendWidgetPrivate::Attribute(Allocation, Id); A->Resolution = ELHValueResolution::Resolved;
        A->Provenance.Status = ELHProvenanceStatus::Prototype;
        A->Provenance.Notes = TEXT("UI staged command delta, not a canonical stat");
    }
    Build();
}
void SLHFrontendWidget::Open(ELHUIScreen Screen)
{
    if (HasAllocation() && (P->Screen() == ELHUIScreen::CharacterSheet || P->Screen() == ELHUIScreen::Inventory) && Screen != P->Screen())
    { NextScreen = Screen; Modal("Discard"); return; }
    const auto PreviousScreen=P->Screen();
    P->Open(Screen);
    if (PreviousScreen!=P->Screen()) { SubmittedAction=NAME_None; Appearance=P->AppearanceInput(); Answers=P->AnswerInput(); }
    LocalMessage.Empty(); ModalAction = NAME_None; Build(); Focus();
}
FText SLHFrontendWidget::Summary() const
{
    const auto& C = P->Snapshot().Character;
    FString S;
    switch (P->Screen())
    {
    case ELHUIScreen::Frontend: S = TEXT("Lighthaven | Single-player\nLocal characters: ") + FString::FromInt(P->Profiles().Num()); break;
    case ELHUIScreen::Characters:
        S = TEXT("Characters\nSelect a character, then Continue. Your save files are kept on failure.");
        if (P->Profiles().IsValidIndex(ProfileIndex))
        {
            const auto& V = P->Profiles()[ProfileIndex]; S += TEXT("\n") + V.Name + TEXT(" | ") + V.Status;
            if (V.bRequiresRecoveryAcknowledgment) S += TEXT("\nRecovery: An earlier valid save is available. Progress after this save may be lost.");
        }
        break;
    case ELHUIScreen::Creation:
        S = TEXT("New Character\nAppearance and questions are owner supplied. Roll, review, then Confirm.\nInitial resources / starter kit / learned abilities: — (not available before owner confirmation).");
        if (P->CreationPreview().Token.IsValid())
        {
            auto A = P->CreationPreview().Record.AcceptedAttributes;
            for (FName Id : {"Strength", "Endurance", "Agility", "Intelligence", "Wisdom"}) S += TEXT("\n") + Id.ToString() + TEXT(": ") + FLHUIPresenter::Format(*LHFrontendWidgetPrivate::Attribute(A,Id));
            for (const auto& E : P->CreationPreview().FieldErrors) S += TEXT("\nError: ") + E.Key.ToString() + TEXT(": ") + E.Value;
        }
        break;
    case ELHUIScreen::CharacterSheet:
        S = TEXT("Character | ") + C.DisplayName + (P->HasUnsavedChanges() ? TEXT(" (unsaved)") : TEXT("")) + TEXT("\nLevel ") + FLHUIPresenter::Format(C.EarnedLevel)
            + TEXT(" | XP ") + FLHUIPresenter::Format(C.ExperienceBalance) + TEXT(" | Debt ") + FLHUIPresenter::Format(C.ExperienceDebt)
            + TEXT("\nAttribute points ") + FLHUIPresenter::Format(C.UnspentAttributePoints)
            + TEXT(" | Skill points ") + FLHUIPresenter::Format(C.UnspentSkillPoints)
            + TEXT("\nHealth ") + FLHUIPresenter::Format(C.CurrentHealth) + TEXT(" / Earned maximum ") + FLHUIPresenter::Format(C.EarnedBaseHealth)
            + TEXT(" | Mana ") + FLHUIPresenter::Format(C.CurrentMana) + TEXT(" / Earned maximum ") + FLHUIPresenter::Format(C.EarnedBaseMana)
            + TEXT("\n") + P->DerivedSummary() + TEXT("\nGrowth awards: ") + FString::FromInt(C.GrowthAwards.Num());
        for (const auto& G : C.GrowthAwards)
            S += TEXT("\nLevel ") + FLHUIPresenter::Format(G.FromLevel) + TEXT(" → ") + FLHUIPresenter::Format(G.ToLevel)
                + TEXT(" | HP +") + FLHUIPresenter::Format(G.HealthIncrement) + TEXT(" | MP +") + FLHUIPresenter::Format(G.ManaIncrement)
                + TEXT(" | Attribute points +") + FLHUIPresenter::Format(G.AttributePoints) + TEXT(" | Skill points +") + FLHUIPresenter::Format(G.SkillPoints);
        S += TEXT("\nNext-level preview: — (not available in this prototype)"); break;
    case ELHUIScreen::Inventory:
        S = FString(P->HasUnsavedChanges() ? TEXT("Inventory (unsaved) | Gold ") : TEXT("Inventory | Gold ")) + FLHUIPresenter::Format(C.Gold) + TEXT("\nOwned items: ") + FString::FromInt(C.Inventory.Num())
            + TEXT("\nRequirements: — (not available in this prototype). Equip is validated by the owner."); break;
    case ELHUIScreen::Hud: case ELHUIScreen::Dialogue: case ELHUIScreen::Services: case ELHUIScreen::Loot: case ELHUIScreen::Death: case ELHUIScreen::Pause: S=P->GameplaySummary(); break;
    default: S = TEXT("Settings\nLeft/right changes values. Apply stores user preferences; Revert restores them.\nAudio seams: UI.Click and Combat.Impact (silent; no approved runtime sound)." ); break;
    }
    return FText::FromString(S);
}
FText SLHFrontendWidget::Label(FName Id) const
{
    FString S = P->GameplayLabel(Id);
    const auto& V=FLHPresentationSettings::Get();
    if(Id=="Master") S=FString::Printf(TEXT("Master volume: %.0f%%"),V.Master*100);
    if(Id=="Effects") S=FString::Printf(TEXT("Effects volume: %.0f%%"),V.Effects*100);
    if(Id=="UI") S=FString::Printf(TEXT("UI volume: %.0f%%"),V.UI*100);
    if(Id=="TextScale") S=FString::Printf(TEXT("Text scale: %.0f%%"),V.TextScale*100);
    if(Id=="HighContrast") S=FString(TEXT("High contrast: "))+(V.HighContrast?TEXT("On"):TEXT("Off"));
    if (Id == "Profiles") S = P->Profiles().IsValidIndex(ProfileIndex) ? P->Profiles()[ProfileIndex].Name + TEXT(" | Selected") : TEXT("Select character: no selection (left/right)");
    if (Id == "Recovery" && !P->IsControlEnabled(Id)) S = TEXT("Recovery unavailable: no earlier readable generation requires acknowledgment.");
    else if (Id == "Recovery") S = bRecovery ? TEXT("Recovery acknowledged | Selected") : TEXT("Acknowledge earlier save recovery");
    if (Id=="Body" || Id=="Hair" || Id=="Skin" || Id=="Outfit")
    {
        const FString Prefix=TEXT("Presentation.Player.")+Id.ToString()+TEXT(".");
        const auto* Choice=Appearance.FindByPredicate([&](const FLHContentId& A){return A.Value.ToString().StartsWith(Prefix);});
        S=Id.ToString()+TEXT(": ")+(Choice?Choice->Value.ToString().RightChop(Prefix.Len()):TEXT("Missing — select (left/right)"));
    }
    if (Id=="Continue" && !P->IsControlEnabled(Id)) S=TEXT("Continue unavailable: selected character has no readable save generation.");
    if (Id.ToString().StartsWith(TEXT("Question")))
    {
        int32 I = FCString::Atoi(*Id.ToString().Right(1)) - 1;
        const auto Q = P->QuestionCatalog();
        S = Q.IsValidIndex(I) ? Q[I].Prompt : Id.ToString() + TEXT(": Unavailable, question catalog missing");
        if (Answers.IsValidIndex(I)) S += TEXT(" | ") + Answers[I].Answer.Value.ToString();
    }
    auto Draft = Allocation; auto Base = P->Snapshot().Character.BaseAttributes;
    if (auto* A = LHFrontendWidgetPrivate::Attribute(Draft,Id)) S += TEXT(" | Base ") + FLHUIPresenter::Format(*LHFrontendWidgetPrivate::Attribute(Base,Id)) + TEXT(" | Pending +") + FString::Printf(TEXT("%lld"), A->Value);
    if (Id == "Items")
    {
        const auto& Items = P->Snapshot().Character.Inventory;
        S = Items.IsValidIndex(ItemIndex) ? FLHUIPresenter::ItemName(Items[ItemIndex].Definition) + TEXT(" | Quantity ") + FLHUIPresenter::Format(Items[ItemIndex].Quantity) : TEXT("Items: no selection (left/right)");
    }
    if (Id == "Reset") S = TEXT("Reset allocation draft");
    if (P->Screen() == ELHUIScreen::Inventory)
    {
        static const TMap<FName,ELHEquipmentSlot> SlotLabels = {{"Head",ELHEquipmentSlot::Head},{"Torso",ELHEquipmentSlot::Torso},{"MainHand",ELHEquipmentSlot::MainHand},{"OffHand",ELHEquipmentSlot::OffHand},{"Legs",ELHEquipmentSlot::Legs},{"Feet",ELHEquipmentSlot::Feet},{"Accessory",ELHEquipmentSlot::Accessory},{"Quiver",ELHEquipmentSlot::Quiver}};
        if (const auto* TargetSlot = SlotLabels.Find(Id))
        {
            const auto* Binding = P->Snapshot().Character.Equipment.FindByPredicate([TargetSlot](const auto& B) { return B.Slot == *TargetSlot; });
            S += Binding ? TEXT(" | Equipped ") + P->EquippedItemName(Binding->Item) : TEXT(" | Empty");
        }
    }
    if (Id == "Confirm") S = TEXT("Confirm — review before applying");
    if (P->FocusedControl() == Id && ModalAction.IsNone()) S = TEXT("> ") + S;
    return FText::FromString(S);
}
void SLHFrontendWidget::AddControl(FName Id)
{
    TSharedPtr<SWidget> W;
    if (Id == "Name")
    {
        SAssignNew(NameField, SEditableTextBox).Style(&EditStyle).Text(FText::FromString(P->NameInput()))
            .HintText(FText::FromString(TEXT("Character name"))).Font(LHFrontendWidgetPrivate::Font(Style.Control))
            .OnTextChanged_Lambda([this](const FText& T) { EditName(T.ToString()); });
        W = NameField;
    }
    else
    {
        W = SNew(SButton).ButtonStyle(&ButtonStyle).IsFocusable(true).IsEnabled_Lambda([this,Id]() { return P->IsControlEnabled(Id); })
            .OnClicked_Lambda([this,Id]() { Activate(Id); return FReply::Handled(); })
            [ SNew(STextBlock).Text_Lambda([this,Id]() { return Label(Id); })
                .ColorAndOpacity(Style.TextPrimary).Font(LHFrontendWidgetPrivate::Font(Style.Control)).AutoWrapText(true) ];
    }
    TSharedPtr<SHorizontalBox> Group;
    SAssignNew(Group,SHorizontalBox);
    if (Id == "Name") Group->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(Style.ControlInset)
        [SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(P->FocusedControl() == "Name" ? TEXT("> Name") : TEXT("Name")); }).Font(LHFrontendWidgetPrivate::Font(Style.Control)).ColorAndOpacity(Style.TextPrimary)];
    Group->AddSlot().FillWidth(1)[W.ToSharedRef()];
    if (Id == "Profiles" || (Id=="Body" || Id=="Hair" || Id=="Skin" || Id=="Outfit") || Id == "Items" || Id.ToString().StartsWith(TEXT("Question")) || LHFrontendWidgetPrivate::Attribute(Allocation,Id))
    {
        for (int32 D : {-1,1}) Group->AddSlot().AutoWidth().Padding(Style.RowGap,0)
            [SNew(SButton).ButtonStyle(&ButtonStyle).IsFocusable(false).OnClicked_Lambda([this,Id,D]() { Adjust(Id,D); return FReply::Handled(); })
                [SNew(STextBlock).Text(FText::FromString(D<0 ? TEXT("Previous / Minus") : TEXT("Next / Plus"))).Font(LHFrontendWidgetPrivate::Font(Style.Control)).ColorAndOpacity(Style.TextPrimary)]];
    }
    Targets.Add(Id,W);
    Rows->AddSlot().AutoHeight().Padding(Style.FocusGap, Style.RowGap/2)
    [ SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor_Lambda([this,Id]() { return ModalAction.IsNone() && P->FocusedControl() == Id ? Style.FocusRing : Style.Edge; })
        .Padding(Style.FocusStroke)
        [ SNew(SBox).MinDesiredHeight(Style.TargetMin)[Group.ToSharedRef()] ] ];
}
void SLHFrontendWidget::Build()
{
    const auto& V=FLHPresentationSettings::Get();
    Style=FLHUIStyle(); Style.Body*=V.TextScale; Style.Control*=V.TextScale; Style.Metadata*=V.TextScale;
    if(V.HighContrast) { Style.Panel=FLinearColor::Black; Style.Background=FLinearColor::Black; Style.TextPrimary=FLinearColor::White; Style.TextSecondary=FLinearColor::White; }
    Targets.Empty(); NameField.Reset();
    TSharedPtr<SVerticalBox> Layout;
    ChildSlot
    [ SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Style.Background)
        [ SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
            [ SNew(SBox).WidthOverride(1920).HeightOverride(1080)
                [ SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Style.Panel).Padding(FMargin(72,54))
                    [ SAssignNew(Layout,SVerticalBox) ] ] ] ] ];
    Layout->AddSlot().FillHeight(1)
    [ SAssignNew(Scroll,SScrollBox) + SScrollBox::Slot()[SAssignNew(Rows,SVerticalBox)] ];
    // Detailed gear/growth records share the scroll area so they cannot crowd out controls.
    Rows->AddSlot().AutoHeight().Padding(0,0,0,Style.Gutter)
    [ SNew(STextBlock).Text_Lambda([this]() { return Summary(); }).Font(LHFrontendWidgetPrivate::Font(Style.Body)).ColorAndOpacity(Style.TextPrimary).AutoWrapText(true) ];
    if (ModalAction.IsNone()) for (FName Id : P->FocusOrder()) AddControl(Id);
    else if (ModalAction == "Keyboard")
    {
        Rows->AddSlot().AutoHeight()[SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(TEXT("Name: ") + P->NameInput() + TEXT(" | Letter: ") + FString::Chr(TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 -'")[Letter])); }).Font(LHFrontendWidgetPrivate::Font(Style.Body)).ColorAndOpacity(Style.TextPrimary)];
        for (FName Id : {"PreviousLetter", "NextLetter", "AppendLetter", "EraseLetter", "Done"})
        {
            auto B = SNew(SButton).ButtonStyle(&ButtonStyle).OnClicked_Lambda([this,Id]() { Keyboard(Id); return FReply::Handled(); })
                [SNew(STextBlock).Text_Lambda([this,Id]() { return FText::FromString((KeyboardFocus == Id ? TEXT("> ") : TEXT("")) + Id.ToString()); }).Font(LHFrontendWidgetPrivate::Font(Style.Control)).ColorAndOpacity(Style.TextPrimary)];
            Targets.Add(Id,B); Rows->AddSlot().AutoHeight().Padding(Style.FocusGap,Style.RowGap)
                [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                    .BorderBackgroundColor_Lambda([this,Id]() { const bool Focused = ModalAction == "Keyboard" ? KeyboardFocus == Id : (bModalConfirm ? Id == "ModalConfirm" : Id == "ModalCancel"); return Focused ? Style.FocusRing : Style.Edge; })
                    .Padding(Style.FocusStroke)[SNew(SBox).MinDesiredHeight(Style.TargetMin)[B]]];
        }
    }
    else
    {
        Rows->AddSlot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("Review: ") + ModalAction.ToString() + TEXT("? Cancel is the default."))).Font(LHFrontendWidgetPrivate::Font(Style.Body)).ColorAndOpacity(Style.TextPrimary)];
        for (bool Accept : {false,true})
        {
            const FName Id = Accept ? FName("ModalConfirm") : FName("ModalCancel");
            auto B = SNew(SButton).ButtonStyle(&ButtonStyle).OnClicked_Lambda([this,Accept]() { bModalConfirm = Accept; Submit(); return FReply::Handled(); })
            [ SNew(STextBlock).Text_Lambda([this,Accept]() { return FText::FromString(FString(bModalConfirm == Accept ? TEXT("> ") : TEXT("")) + (Accept ? TEXT("Apply") : TEXT("Cancel"))); }).Font(LHFrontendWidgetPrivate::Font(Style.Control)).ColorAndOpacity(Style.TextPrimary) ];
            Targets.Add(Id,B); Rows->AddSlot().AutoHeight().Padding(Style.FocusGap,Style.RowGap)
                [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                    .BorderBackgroundColor_Lambda([this,Id]() { const bool Focused = ModalAction == "Keyboard" ? KeyboardFocus == Id : (bModalConfirm ? Id == "ModalConfirm" : Id == "ModalCancel"); return Focused ? Style.FocusRing : Style.Edge; })
                    .Padding(Style.FocusStroke)[SNew(SBox).MinDesiredHeight(Style.TargetMin)[B]]];
        }
    }
    Layout->AddSlot().AutoHeight().Padding(0,Style.Gutter)
    [ SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(P->Error().IsEmpty() ? LocalMessage : P->FeedbackText()); }).Font(LHFrontendWidgetPrivate::Font(Style.Body)).ColorAndOpacity_Lambda([this]() { return P->HasError()?Style.Error:Style.Info; }).AutoWrapText(true) ];
    Layout->AddSlot().AutoHeight()[SNew(STextBlock).Text_Lambda([](){ return FText::FromString(FLHInputGlyphs::MenuHints()); }).Font(LHFrontendWidgetPrivate::Font(Style.Metadata)).ColorAndOpacity(Style.TextSecondary).AutoWrapText(true)];
}
void SLHFrontendWidget::Focus()
{
    const FName Id = ModalAction == "Keyboard" ? KeyboardFocus : ModalAction.IsNone() ? P->FocusedControl() : bModalConfirm ? FName("ModalConfirm") : FName("ModalCancel");
    if (const auto* W = Targets.Find(Id))
    {
        if (FSlateApplication::IsInitialized()) FSlateApplication::Get().SetKeyboardFocus(*W, EFocusCause::Navigation);
        if (Scroll.IsValid()) Scroll->ScrollDescendantIntoView(*W, false);
    }
}
void SLHFrontendWidget::Navigate(int32 Delta)
{
    if (ModalAction == "Keyboard")
    {
        const TArray<FName> Keys = {"PreviousLetter", "NextLetter", "AppendLetter", "EraseLetter", "Done"};
        KeyboardFocus = Keys[FMath::Clamp(Keys.IndexOfByKey(KeyboardFocus)+Delta,0,Keys.Num()-1)];
    }
    else if (IsModal()) bModalConfirm = Delta > 0;
    else P->MoveFocus(Delta);
    Focus();
}
void SLHFrontendWidget::EditName(const FString& Name)
{
    P->EditCreation(Name,Appearance,Answers); SubmittedAction = NAME_None;
    if (NameField && NameField->GetText().ToString() != P->NameInput()) NameField->SetText(FText::FromString(P->NameInput()));
}
FString SLHFrontendWidget::FieldName() const { return NameField.IsValid() ? NameField->GetText().ToString() : P->NameInput(); }
void SLHFrontendWidget::StageCreation() { P->EditCreation(P->NameInput(),Appearance,Answers); SubmittedAction = NAME_None; }
void SLHFrontendWidget::Adjust(FName Id, int32 D)
{
    if(Id=="Master" || Id=="Effects" || Id=="UI" || Id=="TextScale" || Id=="HighContrast")
    {
        auto& V=FLHPresentationSettings::Get();
        if(Id=="Master") V.Master+=D*.1f; else if(Id=="Effects") V.Effects+=D*.1f; else if(Id=="UI") V.UI+=D*.1f; else if(Id=="TextScale") V.TextScale+=D*.1f; else V.HighContrast=!V.HighContrast;
        V.Normalize(); Build(); Focus(); return;
    }
    if (Id == "Profiles")
    {
        const auto& V = P->Profiles(); if (V.IsEmpty()) return;
        ProfileIndex = FMath::Clamp(ProfileIndex+D,0,V.Num()-1); P->SelectProfile(V[ProfileIndex].Id); LocalMessage.Empty(); bRecovery = false; SubmittedAction = NAME_None;
    }
    else if ((Id=="Body" || Id=="Hair" || Id=="Skin" || Id=="Outfit"))
    {
        TArray<FLHContentId> C;
        const FString Prefix=TEXT("Presentation.Player.")+Id.ToString()+TEXT(".");
        for (const auto& Option:P->AppearanceCatalog()) if (Option.Value.ToString().StartsWith(Prefix)) C.Add(Option);
        if (C.IsEmpty()) { LocalMessage=TEXT("Unavailable: ")+Id.ToString()+TEXT(" catalog missing."); return; }
        int32 Index=C.IndexOfByPredicate([this](const FLHContentId& Option){return Appearance.ContainsByPredicate([&](const FLHContentId& A){return A.Value==Option.Value;});});
        Index=FMath::Clamp(Index+D,0,C.Num()-1);
        Appearance.RemoveAll([&](const FLHContentId& A){return A.Value.ToString().StartsWith(Prefix);});
        Appearance.Add(C[Index]); StageCreation();
    }
    else if (Id.ToString().StartsWith(TEXT("Question")))
    {
        const int32 I = FCString::Atoi(*Id.ToString().Right(1))-1; const auto Q = P->QuestionCatalog();
        if (!Q.IsValidIndex(I) || Q[I].Answers.IsEmpty()) return;
        while (Answers.Num() <= I) Answers.AddDefaulted();
        int32 Index = Q[I].Answers.IndexOfByPredicate([this,I](const FLHContentId& A) { return A.Value == Answers[I].Answer.Value; });
        Index = FMath::Clamp(Index+D,0,Q[I].Answers.Num()-1); Answers[I].Question = Q[I].Id; Answers[I].Answer = Q[I].Answers[Index]; StageCreation();
    }
    else if (auto* A = LHFrontendWidgetPrivate::Attribute(Allocation,Id))
    {
        if (D > 0 && A->Value < TNumericLimits<int64>::Max()) ++A->Value;
        else if (D < 0 && A->Value > 0) --A->Value;
        SubmittedAction = NAME_None;
    }
    else if (Id == "Items")
    {
        const auto& Items = P->Snapshot().Character.Inventory; if (Items.IsEmpty()) return;
        ItemIndex = FMath::Clamp(ItemIndex+D,0,Items.Num()-1); SelectedItem = Items[ItemIndex].Id; bUnequip = false; SubmittedAction = NAME_None;
    }
}
void SLHFrontendWidget::Modal(FName Action) { ModalAction = Action; bModalConfirm = false; Build(); Focus(); }
void SLHFrontendWidget::Submit()
{
    const FName Action = ModalAction; ModalAction = NAME_None;
    if (bModalConfirm && (Action == "Quit" || Action == "Discard" || SubmittedAction != Action))
    {
        if (Action == "Confirm")
        {
            FLHCommandResult R;
            if (P->Screen() == ELHUIScreen::Creation) R = P->ConfirmCreation();
            else if (P->Screen() == ELHUIScreen::CharacterSheet) R = P->Allocate(Allocation);
            else R = P->Equip(SelectedItem,Slot,bUnequip);
            SubmittedAction = R.Disposition == ELHCommandDisposition::Accepted ? Action : NAME_None;
            if (SubmittedAction == Action)
            {
                LocalMessage.Empty(); // Owner status reports pending save, failure, or completed transition.
                if (P->Screen() == ELHUIScreen::CharacterSheet)
                    for (FName Id : {"Strength", "Endurance", "Agility", "Intelligence", "Wisdom"}) LHFrontendWidgetPrivate::Attribute(Allocation,Id)->Value = 0;
            }
        }
        else if (Action == "Continue") SubmittedAction = P->Continue(bRecovery) ? Action : NAME_None;
        else if (Action == "Quit") P->Quit();
        else if (Action == "Discard")
        {
            for (FName Id : {"Strength", "Endurance", "Agility", "Intelligence", "Wisdom"}) LHFrontendWidgetPrivate::Attribute(Allocation,Id)->Value = 0;
            if (P->Screen() == ELHUIScreen::Creation)
            { Appearance.Empty(); Answers.Empty(); P->EditCreation(TEXT(""),{},{}); }
            SubmittedAction = NAME_None;
            const ELHUIScreen Destination = NextScreen.Get(ELHUIScreen::Frontend); NextScreen.Reset();
            Open(Destination); return;
        }
    }
    NextScreen.Reset(); Build(); Focus();
}
void SLHFrontendWidget::Activate(FName Id)
{
    if (!P->IsControlEnabled(Id)) return;
    FLHPresentationAudio::Cue("UI.Click",true);
    if(P->Screen()==ELHUIScreen::Settings)
    {
        if(Id=="Apply") { const bool Saved=FLHPresentationSettings::Get().Save(FLHPresentationSettings::UserFile()); LocalMessage=Saved?TEXT("Settings saved."):TEXT("Error: Settings could not be saved. Session preview retained."); return; }
        if(Id=="Revert") { FLHPresentationSettings::Get().Load(FLHPresentationSettings::UserFile()); Build(); Focus(); return; }
        if(Id=="Master" || Id=="Effects" || Id=="UI" || Id=="TextScale" || Id=="HighContrast") { Adjust(Id,1); return; }
    }
    if (ModalAction == "Keyboard") { Keyboard(KeyboardFocus); return; }
    if (IsModal()) { Submit(); return; }
    const int32 I = P->FocusOrder().IndexOfByKey(Id); if (I != INDEX_NONE) P->MoveFocus(I - P->FocusOrder().IndexOfByKey(P->FocusedControl()));
    if (P->ActivateGameplay(Id)) { Build(); }
    else if (Id == "Use" || Id == "AssignItem") { const auto Items=P->Snapshot().Character.Inventory; if(Items.IsValidIndex(ItemIndex)) { if(Id=="Use") P->UseItem(Items[ItemIndex].Id); else P->AssignHotbarItem(Items[ItemIndex].Id); } }
    else if (Id == "RetrySave") { P->RetryPersistence(); }
    else if (Id == "Name") { Modal("Keyboard"); }
    else if (Id == "New") Open(ELHUIScreen::Creation);
    else if (Id == "Characters" || (Id == "Continue" && P->Screen() == ELHUIScreen::Frontend)) Open(ELHUIScreen::Characters);
    else if (Id == "Settings") Open(ELHUIScreen::Settings);
    else if (Id == "CharacterTab") Open(ELHUIScreen::CharacterSheet);
    else if (Id == "InventoryTab") Open(ELHUIScreen::Inventory);
    else if (Id == "Back") Back();
    else if (Id == "Recovery") { bRecovery = !bRecovery; SubmittedAction = NAME_None; P->ClearSelectionError(); LocalMessage.Empty(); }
    else if (Id == "Roll" || Id == "Reroll" || Id == "Review") { P->Roll(Id == "Reroll"); }
    else if (Id == "Confirm")
    {
        if (P->Screen()==ELHUIScreen::Creation && P->IsConfirmed()) { P->RetryPersistence(); return; }
        FLHUIIntentReview Review;
        if (P->Screen() == ELHUIScreen::CharacterSheet) {
            Review = P->ReviewAllocation(Allocation);
            auto Draft = Allocation;
            for (FName A : {"Strength", "Endurance", "Agility", "Intelligence", "Wisdom"})
                Review.Summary += TEXT("\n") + A.ToString() + TEXT(" pending +") + FString::Printf(TEXT("%lld"),LHFrontendWidgetPrivate::Attribute(Draft,A)->Value);
        }
        else if (P->Screen() == ELHUIScreen::Inventory) Review = P->ReviewEquipment(SelectedItem,Slot,bUnequip);
        else { Review.bLegal = P->CreationPreview().bLegal; Review.Summary = TEXT("Create ") + P->NameInput() + TEXT(" with the reviewed attributes?"); }
        LocalMessage = Review.Summary;
        if (P->Screen() == ELHUIScreen::CharacterSheet && !HasAllocation())
        { Review.bLegal = false; LocalMessage = TEXT("Unavailable: add at least one pending point."); }
        if (Review.bLegal && SubmittedAction != Id) Modal(Id);
    }
    else if (Id == "Continue" || Id == "Quit") Modal(Id);
    else if (Id == "Profiles" || (Id=="Body" || Id=="Hair" || Id=="Skin" || Id=="Outfit") || Id == "Items" || Id.ToString().StartsWith(TEXT("Question")) || LHFrontendWidgetPrivate::Attribute(Allocation,Id)) Adjust(Id,1);
    else if (Id == "Equip")
    {
        const auto& Items = P->Snapshot().Character.Inventory;
        if (Items.IsValidIndex(ItemIndex)) { SelectedItem = Items[ItemIndex].Id; bUnequip = false; SubmittedAction = NAME_None; }
        LocalMessage = TEXT("Equip selected inventory item into selected slot. Confirm to review.");
    }
    else if (Id == "Unequip")
    {
        const auto* Binding = P->Snapshot().Character.Equipment.FindByPredicate([this](const auto& B) { return B.Slot == Slot; });
        if (Binding) { SelectedItem = Binding->Item; bUnequip = true; SubmittedAction = NAME_None; }
        else { SelectedItem = {}; LocalMessage = TEXT("Unavailable: selected equipment slot is empty."); }
    }
    else if (Id == "Reset")
    {
        for (FName A : {"Strength", "Endurance", "Agility", "Intelligence", "Wisdom"}) LHFrontendWidgetPrivate::Attribute(Allocation,A)->Value = 0;
        SubmittedAction = NAME_None; LocalMessage.Empty(); P->ClearSelectionError();
    }
    else if (Id == "Details") LocalMessage = TEXT("Details: owner-provided rules and provenance only. Missing values: — (not available in this prototype).");
    else
    {
        static const TMap<FName,ELHEquipmentSlot> Slots = {{"Head",ELHEquipmentSlot::Head},{"Torso",ELHEquipmentSlot::Torso},{"MainHand",ELHEquipmentSlot::MainHand},{"OffHand",ELHEquipmentSlot::OffHand},{"Legs",ELHEquipmentSlot::Legs},{"Feet",ELHEquipmentSlot::Feet},{"Accessory",ELHEquipmentSlot::Accessory},{"Quiver",ELHEquipmentSlot::Quiver}};
        if (const auto* S = Slots.Find(Id))
        {
            Slot = *S; SubmittedAction = NAME_None;
            if (ItemIndex == INDEX_NONE)
            {
                const auto* Binding = P->Snapshot().Character.Equipment.FindByPredicate([this](const auto& B) { return B.Slot == Slot; });
                if (Binding) { SelectedItem = Binding->Item; bUnequip = true; }
                else SelectedItem = {};
            }
            LocalMessage = TEXT("Selected slot: ") + Id.ToString() + (bUnequip ? TEXT(" | Unequip") : TEXT(" | Equip selected item"));
        }
    }
    Focus();
}
void SLHFrontendWidget::Back()
{
    if (ModalAction == "Keyboard") { Keyboard("Done"); return; }
    if (IsModal()) { bModalConfirm = false; NextScreen.Reset(); Submit(); return; }
    if (P->Screen() == ELHUIScreen::Creation && P->IsConfirmed())
    { LocalMessage = TEXT("Creation accepted. Waiting for the owner to finish saving and transition."); return; }
    if (!HasAllocation() && P->ResumeGameplay()) return;
    if (P->Screen() == ELHUIScreen::Frontend) { Modal("Quit"); return; }
    if (P->Screen() == ELHUIScreen::Creation || P->Screen() == ELHUIScreen::CharacterSheet) { Modal("Discard"); return; }
    Open(ELHUIScreen::Frontend);
}
FReply SLHFrontendWidget::OnPreviewKeyDown(const FGeometry&, const FKeyEvent& E)
{
    if (!IsModal())
        for (const auto& Target : Targets)
            if (Target.Value->HasKeyboardFocus() || Target.Value->HasFocusedDescendants())
            {
                const int32 Index = P->FocusOrder().IndexOfByKey(Target.Key);
                if (Index != INDEX_NONE) P->MoveFocus(Index-P->FocusOrder().IndexOfByKey(P->FocusedControl()));
                break;
            }
    const FKey K = E.GetKey(); FLHInputGlyphs::Observe(K);
    const bool Confirm = K == EKeys::Enter || K == EKeys::SpaceBar || K == EKeys::Gamepad_FaceButton_Bottom;
    if (E.IsRepeat() && (Confirm || K == EKeys::C || K == EKeys::I || K == EKeys::Escape || K == EKeys::Gamepad_FaceButton_Right)) return FReply::Handled();
    if (K == EKeys::Up || K == EKeys::Gamepad_DPad_Up) Navigate(-1);
    else if (K == EKeys::Down || K == EKeys::Gamepad_DPad_Down || K == EKeys::Tab) Navigate(K == EKeys::Tab && E.IsShiftDown() ? -1 : 1);
    else if (K == EKeys::Escape || K == EKeys::Gamepad_FaceButton_Right) Back();
    else if (Confirm && (P->FocusedControl() != "Name" || IsModal() || K == EKeys::Gamepad_FaceButton_Bottom)) { if (ModalAction == "Keyboard") Keyboard(KeyboardFocus); else if (IsModal()) Submit(); else Activate(P->FocusedControl()); }
    else if ((K == EKeys::Left || K == EKeys::Right || K == EKeys::Gamepad_DPad_Left || K == EKeys::Gamepad_DPad_Right) && (P->FocusedControl() != "Name" || IsModal()))
    {
        const int32 D = K == EKeys::Left || K == EKeys::Gamepad_DPad_Left ? -1 : 1;
        if (IsModal()) Navigate(D); else Adjust(P->FocusedControl(),D);
    }
    else if ((K == EKeys::C || K == EKeys::I) && !IsModal()
        && (P->Screen() == ELHUIScreen::CharacterSheet || P->Screen() == ELHUIScreen::Inventory))
    {
        const auto Destination = K == EKeys::C ? ELHUIScreen::CharacterSheet : ELHUIScreen::Inventory;
        if (P->Screen() == Destination) Back(); else Open(Destination);
    }
    else if ((K == EKeys::Gamepad_LeftShoulder || K == EKeys::Gamepad_RightShoulder) && !IsModal()
        && (P->Screen() == ELHUIScreen::CharacterSheet || P->Screen() == ELHUIScreen::Inventory))
        Open(K == EKeys::Gamepad_LeftShoulder ? ELHUIScreen::CharacterSheet : ELHUIScreen::Inventory);
    else return FReply::Unhandled();
    return FReply::Handled();
}
FReply SLHFrontendWidget::OnKeyDown(const FGeometry& G, const FKeyEvent& E) { return OnPreviewKeyDown(G,E); }
FReply SLHFrontendWidget::OnAnalogValueChanged(const FGeometry&, const FAnalogInputEvent& E)
{
    if(FMath::Abs(E.GetAnalogValue())>.25f) FLHInputGlyphs::Observe(E.GetKey());
    if (E.GetKey() != EKeys::Gamepad_LeftY && E.GetKey() != EKeys::Gamepad_LeftX) return FReply::Unhandled();
    if (E.GetKey() == EKeys::Gamepad_LeftY) Stick.Y = E.GetAnalogValue(); else Stick.X = E.GetAnalogValue();
    if (FMath::Max(FMath::Abs(Stick.X),FMath::Abs(Stick.Y)) < .25f) { bStickNeutral = true; return FReply::Handled(); }
    if (bStickNeutral && FMath::Max(FMath::Abs(Stick.X),FMath::Abs(Stick.Y)) > .6f)
    {
        bStickNeutral = false;
        if (FMath::Abs(Stick.Y) >= FMath::Abs(Stick.X)) Navigate(Stick.Y > 0 ? -1 : 1);
        else if (IsModal()) Navigate(Stick.X > 0 ? 1 : -1);
        else Adjust(P->FocusedControl(), Stick.X > 0 ? 1 : -1);
    }
    return FReply::Handled();
}

bool SLHFrontendWidget::HasAllocation() const
{
    return Allocation.Strength.Value || Allocation.Endurance.Value || Allocation.Agility.Value || Allocation.Intelligence.Value || Allocation.Wisdom.Value;
}
void SLHFrontendWidget::Keyboard(FName Id)
{
    const FString Letters = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 -'");
    if (Id == "PreviousLetter") Letter = (Letter+Letters.Len()-1)%Letters.Len();
    else if (Id == "NextLetter") Letter = (Letter+1)%Letters.Len();
    else if (Id == "AppendLetter") EditName(P->NameInput()+FString::Chr(Letters[Letter]));
    else if (Id == "EraseLetter") EditName(P->NameInput().LeftChop(1));
    else if (Id == "Done") { ModalAction = NAME_None; Build(); }
    Focus();
}
