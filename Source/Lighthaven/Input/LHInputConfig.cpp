#include "Input/LHInputConfig.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputTriggers.h"
#include "InputCoreTypes.h"
const TArray<FName>& ULHInputConfig::GameplayActions()
{
    static const TArray<FName> Names = {"Move", "Look", "Zoom", "ToggleRun", "Interact", "Attack", "TargetNext", "TargetPrev", "CancelTarget", "OpenCharacter", "OpenInventory", "Pause", "Hotbar1", "Hotbar2", "Hotbar3", "Hotbar4", "Hotbar5", "Hotbar6", "HotbarItem", "ShoulderSelect", "OpenAbilities"};
    return Names;
}
UInputAction* ULHInputConfig::Action(FName Name) const { return Actions.FindRef(Name); }
UInputMappingContext* ULHInputConfig::Context(ELHInputContext Mode) const { return Contexts.IsValidIndex(int32(Mode)) ? Contexts[int32(Mode)] : nullptr; }
void ULHInputConfig::Initialize()
{
    if (!Contexts.IsEmpty()) return;
    for (int32 I=0; I<3; ++I) Contexts.Add(NewObject<UInputMappingContext>(this));
    for (FName Name : GameplayActions())
    {
        UInputAction* A = NewObject<UInputAction>(this);
        A->ValueType = (Name == "Move" || Name == "Look") ? EInputActionValueType::Axis2D : Name == "Zoom" ? EInputActionValueType::Axis1D : EInputActionValueType::Boolean;
        Actions.Add(Name, A);
    }
    auto Map = [this](FName Name, FKey Key, bool Negate=false, bool Y=false)
    {
        auto& M = Contexts[0]->MapKey(Action(Name), Key);
        if (Negate) M.Modifiers.Add(NewObject<UInputModifierNegate>(this));
        if (Y) { auto* S=NewObject<UInputModifierSwizzleAxis>(this); S->Order=EInputAxisSwizzle::YXZ; M.Modifiers.Add(S); }
    };
    auto* Select=NewObject<UInputAction>(this); Select->ValueType=EInputActionValueType::Boolean; Actions.Add("SelectMouse",Select); Map("SelectMouse",EKeys::LeftMouseButton);
    Map("Move", EKeys::W, false, true); Map("Move", EKeys::S, true, true);
    Map("Move", EKeys::D); Map("Move", EKeys::A, true); Map("Move", EKeys::Gamepad_Left2D);
    Map("Look", EKeys::Mouse2D); Map("Look", EKeys::Gamepad_Right2D);
    Map("Zoom", EKeys::MouseWheelAxis); Map("Zoom", EKeys::Gamepad_DPad_Up); Map("Zoom", EKeys::Gamepad_DPad_Down, true);
    Map("ToggleRun", EKeys::LeftShift); Map("ToggleRun", EKeys::Gamepad_LeftThumbstick);
    Map("Interact", EKeys::E); Map("Interact", EKeys::Gamepad_FaceButton_Bottom);
    Map("Attack", EKeys::RightMouseButton); Map("Attack", EKeys::Gamepad_RightTrigger);
    Map("TargetNext", EKeys::Tab); Map("TargetNext", EKeys::Gamepad_RightShoulder);
    Map("TargetPrev", EKeys::Q); Map("TargetPrev", EKeys::Gamepad_LeftShoulder);
    Map("CancelTarget", EKeys::F); Map("CancelTarget", EKeys::Gamepad_RightThumbstick);
    Map("OpenCharacter", EKeys::C); Map("OpenCharacter", EKeys::Gamepad_FaceButton_Top);
    Map("OpenInventory", EKeys::I); Map("OpenInventory", EKeys::Gamepad_Special_Left);
    Map("Pause", EKeys::Escape); Map("Pause", EKeys::Gamepad_Special_Right);
    Map("OpenAbilities",EKeys::K); Map("OpenAbilities",EKeys::Gamepad_FaceButton_Left);
    Map("ShoulderSelect",EKeys::LeftAlt); Map("ShoulderSelect",EKeys::Gamepad_LeftTrigger);
    const FKey KeyboardSlots[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six};
    const FKey PadSlots[]={EKeys::Gamepad_FaceButton_Bottom,EKeys::Gamepad_FaceButton_Right,EKeys::Gamepad_FaceButton_Left,EKeys::Gamepad_FaceButton_Top,EKeys::Gamepad_DPad_Left,EKeys::Gamepad_DPad_Right};
    for(int32 I=0; I<6; ++I)
    {
        const FName Name(*FString::Printf(TEXT("Hotbar%d"),I+1)); Map(Name,KeyboardSlots[I]);
        auto& M=Contexts[0]->MapKey(Action(Name),PadSlots[I]);
        auto* Chord=NewObject<UInputTriggerChordAction>(this); Chord->ChordAction=Action("ShoulderSelect"); M.Triggers.Add(Chord);
    }
    Map("HotbarItem",EKeys::Seven);
    auto& ItemMap=Contexts[0]->MapKey(Action("HotbarItem"),EKeys::Gamepad_DPad_Down);
    auto* ItemChord=NewObject<UInputTriggerChordAction>(this); ItemChord->ChordAction=Action("ShoulderSelect"); ItemMap.Triggers.Add(ItemChord);
    // UI/Creation deliberately contain navigation only; never gameplay movement/attack.
    for (int32 I=1; I<3; ++I)
    {
        for (FName Name : {FName("Navigate"), FName("Confirm"), FName("Back")})
        {
            if (!Actions.Contains(Name)) { auto* A=NewObject<UInputAction>(this); A->ValueType=Name=="Navigate" ? EInputActionValueType::Axis2D : EInputActionValueType::Boolean; Actions.Add(Name,A); }
        }
        Contexts[I]->MapKey(Action("Navigate"), EKeys::Gamepad_Left2D);
        for (auto Pair : {TPair<FName,FKey>("Confirm",EKeys::Enter), {"Confirm",EKeys::Gamepad_FaceButton_Bottom}, {"Back",EKeys::Escape}, {"Back",EKeys::Gamepad_FaceButton_Right}}) Contexts[I]->MapKey(Action(Pair.Key),Pair.Value);
        for (auto Pair : {TPair<FKey,int32>(EKeys::Up,1), {EKeys::Down,-1}, {EKeys::Right,2}, {EKeys::Left,-2}, {EKeys::Gamepad_DPad_Up,1}, {EKeys::Gamepad_DPad_Down,-1}, {EKeys::Gamepad_DPad_Right,2}, {EKeys::Gamepad_DPad_Left,-2}})
        {
            auto& M=Contexts[I]->MapKey(Action("Navigate"),Pair.Key);
            if (Pair.Value<0) M.Modifiers.Add(NewObject<UInputModifierNegate>(this));
            if (FMath::Abs(Pair.Value)==1) { auto* S=NewObject<UInputModifierSwizzleAxis>(this); S->Order=EInputAxisSwizzle::YXZ; M.Modifiers.Add(S); }
        }
    }
}
bool ULHInputConfig::HasDeviceParity(FName Name) const
{
    bool Keyboard=false, Pad=false;
    if (Contexts.IsEmpty()) return false;
    for (const auto& M : Contexts[0]->GetMappings()) if (M.Action==Action(Name)) { Pad |= M.Key.IsGamepadKey(); Keyboard |= !M.Key.IsGamepadKey(); }
    return Keyboard && Pad;
}
