#include "UI/LHGameplayHud.h"
#include "UI/LHUIStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Styling/CoreStyle.h"
namespace LHGameplayHudPrivate
{
TOptional<float> Fraction(const FLHNumber& V,const FLHNumber& M)
{
    if(V.Resolution!=ELHValueResolution::Resolved || M.Resolution!=ELHValueResolution::Resolved || !FMath::IsFinite(V.Value) || !FMath::IsFinite(M.Value) || M.Value<=0) return 0.f;
    return float(FMath::Clamp(V.Value/M.Value,0.,1.));
}
}
void SLHGameplayHud::Construct(const FArguments& A)
{
    P=A._Presenter; const FLHUIStyle S;
    TSharedPtr<SOverlay> Layout;
    ChildSlot[SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SNew(SBox).WidthOverride(1920).HeightOverride(1080)[SAssignNew(Layout,SOverlay)]]];
    auto Text=[&](TAttribute<FText> T){return SNew(STextBlock).Text(T).Font(FCoreStyle::GetDefaultFontStyle("Regular",30)).ColorAndOpacity(S.TextPrimary).AutoWrapText(true);};
    Layout->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(72,54)
    [SNew(SBox).WidthOverride(426)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(S.Panel).Padding(24)
    [SNew(SVerticalBox)
    +SVerticalBox::Slot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([this](){auto H=P->Hud();return FText::FromString(TEXT("Health ")+P->Format(H.Health)+TEXT(" / ")+P->Format(H.MaxHealth));}))]
    +SVerticalBox::Slot().AutoHeight().Padding(0,12)[SNew(SProgressBar).Percent_Lambda([this](){auto H=P->Hud();return LHGameplayHudPrivate::Fraction(H.Health,H.MaxHealth);}).FillColorAndOpacity(S.BarHealth)]
    +SVerticalBox::Slot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([this](){auto H=P->Hud();return FText::FromString(TEXT("Mana ")+P->Format(H.Mana)+TEXT(" / ")+P->Format(H.MaxMana));}))]
    +SVerticalBox::Slot().AutoHeight().Padding(0,12)[SNew(SProgressBar).Percent_Lambda([this](){auto H=P->Hud();return LHGameplayHudPrivate::Fraction(H.Mana,H.MaxMana);}).FillColorAndOpacity(S.BarMana)]
    ]]];
    Layout->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0,54)
    [SNew(SBox).WidthOverride(576)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(S.Panel).Padding(24)
    [Text(TAttribute<FText>::CreateLambda([this](){auto H=P->Hud(); return FText::FromString(H.TargetName+TEXT("\nHealth ")+P->Format(H.TargetHealth)+TEXT(" / ")+P->Format(H.TargetMaxHealth));}))]]];
    Layout->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(72,54)
    [SNew(SBox).WidthOverride(426)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(S.Panel).Padding(24)
    [Text(TAttribute<FText>::CreateLambda([this](){auto H=P->Hud();return FText::FromString(H.SaveStatus+TEXT("\n")+H.Objective+TEXT("\n")+H.CompletionNotice);}))]]];
    TSharedPtr<SHorizontalBox> Slots;
    Layout->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(72,54)
    [SNew(SVerticalBox)+SVerticalBox::Slot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([this](){return FText::FromString(P->Error());}))]
    +SVerticalBox::Slot().AutoHeight()[SAssignNew(Slots,SHorizontalBox)]
    +SVerticalBox::Slot().AutoHeight()[Text(FText::FromString(TEXT("1–6 select | Attack | E / South interact | K / West abilities | 7 item | Menu pause")))]];
    for(int32 I=0; I<6; ++I) Slots->AddSlot().AutoWidth().Padding(12,12)
    [SNew(SBox).WidthOverride(180)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(S.Panel).Padding(12)
    [Text(TAttribute<FText>::CreateLambda([this,I](){return FText::FromString(P->GameplayLabel(FName(*FString::Printf(TEXT("Ability%d"),I+1))));}))]]];
}
