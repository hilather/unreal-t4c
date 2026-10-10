#include "Input/LHInputGlyphs.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
namespace LHInputGlyphsPrivate
{
bool Pad=false;
class FObserver : public IInputProcessor
{
public:
    void Tick(const float, FSlateApplication&, TSharedRef<ICursor>) override {}
    bool HandleKeyDownEvent(FSlateApplication&,const FKeyEvent& E) override { FLHInputGlyphs::Observe(E.GetKey()); return false; }
    bool HandleAnalogInputEvent(FSlateApplication&,const FAnalogInputEvent& E) override { if(FMath::Abs(E.GetAnalogValue())>.25f) FLHInputGlyphs::Observe(E.GetKey()); return false; }
    bool HandleMouseButtonDownEvent(FSlateApplication&,const FPointerEvent&) override { Pad=false; return false; }
};
}
void FLHInputGlyphs::Initialize()
{
    static TSharedPtr<LHInputGlyphsPrivate::FObserver> Observer;
    if(!Observer && FSlateApplication::IsInitialized()) { Observer=MakeShared<LHInputGlyphsPrivate::FObserver>(); FSlateApplication::Get().RegisterInputPreProcessor(Observer); }
}
void FLHInputGlyphs::Observe(FKey Key) { LHInputGlyphsPrivate::Pad=Key.IsGamepadKey(); }
bool FLHInputGlyphs::Gamepad() { return LHInputGlyphsPrivate::Pad; }
FString FLHInputGlyphs::MenuHints() { return Gamepad()?TEXT("[D-pad] Focus / Change | [South] Confirm | [East] Back | [L/R shoulder] Tabs"):TEXT("[Arrows] Focus / Change | [Enter] Confirm | [Esc] Back | [C/I] Tabs"); }
FString FLHInputGlyphs::GameplayHints() { return Gamepad()?TEXT("[Right trigger] Attack | [South] Interact | [West] Abilities | [Menu] Pause"):TEXT("[Right mouse] Attack | [E] Interact | [K] Abilities | [7] Item | [Esc] Pause"); }
