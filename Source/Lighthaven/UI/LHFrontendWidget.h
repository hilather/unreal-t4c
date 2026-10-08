#pragma once
#include "Widgets/SCompoundWidget.h"
#include "Styling/SlateTypes.h"
#include "UI/LHUIPresenter.h"
#include "UI/LHUIStyle.h"
class SVerticalBox;
class SEditableTextBox;
class SScrollBox;

// Native screen host. Presenter and adapters must outlive this widget.
class LIGHTHAVEN_API SLHFrontendWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLHFrontendWidget) {} SLATE_ARGUMENT(FLHUIPresenter*, Presenter) SLATE_END_ARGS()
    void Construct(const FArguments& Args);
    void Open(ELHUIScreen Screen);
    void Navigate(int32 Delta);
    void Activate(FName Control);
    void Back();
    void EditName(const FString& Name);
    FString FieldName() const;
    bool HasControl(FName Id) const { return Targets.Contains(Id); }
    bool IsModal() const { return !ModalAction.IsNone(); }
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnPreviewKeyDown(const FGeometry&, const FKeyEvent&) override;
    virtual FReply OnKeyDown(const FGeometry&, const FKeyEvent&) override;
    virtual FReply OnAnalogValueChanged(const FGeometry&, const FAnalogInputEvent&) override;
private:
    void Build();
    void Focus();
    void AddControl(FName Id);
    void Adjust(FName Id, int32 Direction);
    void Submit();
    void Keyboard(FName Id);
    bool HasAllocation() const;
    void Modal(FName Action);
    FText Label(FName Id) const;
    FText Summary() const;
    void StageCreation();
    FLHUIPresenter* P = nullptr;
    FLHUIStyle Style;
    FButtonStyle ButtonStyle;
    FEditableTextBoxStyle EditStyle;
    TSharedPtr<SVerticalBox> Rows;
    TSharedPtr<SScrollBox> Scroll;
    TSharedPtr<SEditableTextBox> NameField;
    TMap<FName, TSharedPtr<SWidget>> Targets;
    TArray<FLHContentId> Appearance;
    TArray<FLHQuestionAnswer> Answers;
    FLHAttributeBlock Allocation;
    int32 ProfileIndex = INDEX_NONE, ItemIndex = INDEX_NONE;
    ELHEquipmentSlot Slot = ELHEquipmentSlot::Unspecified;
    FLHEntityId SelectedItem;
    bool bUnequip = false, bRecovery = false, bModalConfirm = false;
    bool bStickNeutral = true;
    FVector2D Stick = FVector2D::ZeroVector;
    FName ModalAction;
    FName SubmittedAction;
    FString LocalMessage;
    int32 Letter = 0;
    FName KeyboardFocus = "AppendLetter";
    TOptional<ELHUIScreen> NextScreen;
};
