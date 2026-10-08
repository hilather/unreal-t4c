#include "UI/LHUIWidgetHarness.h"
#include "UI/LHFrontendWidget.h"
#include "InputCoreTypes.h"
namespace
{
class FHarness final : public ILHUIWidgetHarness
{
public:
    explicit FHarness(FLHUIPresenter& P) : Widget(SNew(SLHFrontendWidget).Presenter(&P)) {}
    void Open(ELHUIScreen S) override { Widget->Open(S); }
    void Activate(FName Id) override { Widget->Activate(Id); }
    void EditName(const FString& N) override { Widget->EditName(N); }
    FString FieldName() const override { return Widget->FieldName(); }
    bool HasControl(FName Id) const override { return Widget->HasControl(Id); }
    bool IsModal() const override { return Widget->IsModal(); }
    void Key(ELHUITestKey K, bool Repeat) override
    {
        const FKey Keys[] = {EKeys::Gamepad_DPad_Down,EKeys::Gamepad_DPad_Up,EKeys::Gamepad_FaceButton_Bottom,
            EKeys::Gamepad_FaceButton_Right,EKeys::Gamepad_RightShoulder,EKeys::Gamepad_DPad_Right,EKeys::Gamepad_DPad_Left};
        const FKeyEvent E(Keys[static_cast<uint8>(K)],FModifierKeysState(),0,Repeat,0,0);
        Widget->OnPreviewKeyDown(FGeometry(),E);
    }
private:
    TSharedRef<SLHFrontendWidget> Widget;
};
}
TUniquePtr<ILHUIWidgetHarness> ILHUIWidgetHarness::Create(FLHUIPresenter& P) { return MakeUnique<FHarness>(P); }
