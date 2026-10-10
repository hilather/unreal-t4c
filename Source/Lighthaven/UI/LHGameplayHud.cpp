#include "UI/LHGameplayHud.h"
#include "Input/LHInputGlyphs.h"
#include "Engine/Engine.h"
#include "UObject/UObjectIterator.h"
#include "UI/LHUIStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Styling/CoreStyle.h"
void SLHGameplayHud::Construct(const FArguments& A)
{
    P=A._Presenter; FLHInputGlyphs::Initialize(); const FLHUIStyle S;
    TSharedPtr<SOverlay> Layout;
    ChildSlot[SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SNew(SBox).WidthOverride(1920).HeightOverride(1080)[SAssignNew(Layout,SOverlay)]]];
    auto Text=[&](TAttribute<FText> T){return SNew(STextBlock).Text(T).Font_Lambda([](){return FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(30*FLHPresentationSettings::Get().TextScale));}).ColorAndOpacity_Lambda([](){return FLHPresentationSettings::Get().HighContrast?FLinearColor::White:FLHUIStyle().TextPrimary;}).AutoWrapText(true);};
    Layout->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(72,54)
    [SNew(SBox).WidthOverride(426)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor_Lambda([](){return FLHPresentationSettings::Get().HighContrast?FLinearColor::Black:FLHUIStyle().Panel;}).Padding(24)
    [SNew(SVerticalBox)
    +SVerticalBox::Slot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([this](){auto H=P->Hud();return FText::FromString(TEXT("Health ")+P->Format(H.Health)+TEXT(" / ")+P->Format(H.MaxHealth));}))]
    +SVerticalBox::Slot().AutoHeight().Padding(0,12)[SNew(SProgressBar).Visibility_Lambda([this](){auto H=P->Hud(); return H.Health.Resolution==ELHValueResolution::Resolved && H.MaxHealth.Resolution==ELHValueResolution::Resolved && H.MaxHealth.Value>0?EVisibility::Visible:EVisibility::Collapsed;}).Percent_Lambda([this](){return TOptional<float>(Cues.Health);}).FillColorAndOpacity(S.BarHealth)]
    +SVerticalBox::Slot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([this](){auto H=P->Hud();return FText::FromString(TEXT("Mana ")+P->Format(H.Mana)+TEXT(" / ")+P->Format(H.MaxMana));}))]
    +SVerticalBox::Slot().AutoHeight().Padding(0,12)[SNew(SProgressBar).Visibility_Lambda([this](){auto H=P->Hud(); return H.Mana.Resolution==ELHValueResolution::Resolved && H.MaxMana.Resolution==ELHValueResolution::Resolved && H.MaxMana.Value>0?EVisibility::Visible:EVisibility::Collapsed;}).Percent_Lambda([this](){return TOptional<float>(Cues.Mana);}).FillColorAndOpacity(S.BarMana)]
    ]]];
    Layout->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0,54)
    [SNew(SBox).WidthOverride(576)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor_Lambda([](){return FLHPresentationSettings::Get().HighContrast?FLinearColor::Black:FLHUIStyle().Panel;}).Padding(24)
    [Text(TAttribute<FText>::CreateLambda([this](){auto H=P->Hud(); return FText::FromString((H.TargetName.IsEmpty()?TEXT("No target"):TEXT("> Selected: ")+H.TargetName)+TEXT("\nHealth ")+P->Format(H.TargetHealth)+TEXT(" / ")+P->Format(H.TargetMaxHealth));}))]]];
    Layout->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(72,54)
    [SNew(SBox).WidthOverride(426)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor_Lambda([](){return FLHPresentationSettings::Get().HighContrast?FLinearColor::Black:FLHUIStyle().Panel;}).Padding(24)
    [Text(TAttribute<FText>::CreateLambda([this](){auto H=P->Hud();return FText::FromString(H.SaveStatus+TEXT("\n")+H.Objective+TEXT("\n")+H.CompletionNotice);}))]]];
    TSharedPtr<SHorizontalBox> Slots;
    Layout->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(72,54)
    [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor_Lambda([](){return FLHPresentationSettings::Get().HighContrast?FLinearColor::Black:FLHUIStyle().Panel;}).Padding(12)[SNew(SVerticalBox)+SVerticalBox::Slot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([this](){return FText::FromString(Cues.Notice+TEXT("\n")+P->Error());}))]
    +SVerticalBox::Slot().AutoHeight()[SAssignNew(Slots,SHorizontalBox)]
    +SVerticalBox::Slot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([](){return FText::FromString(FLHInputGlyphs::GameplayHints());}))]]];
    for(int32 I=0; I<6; ++I) Slots->AddSlot().AutoWidth().Padding(12,12)
    [SNew(SBox).WidthOverride(180)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor_Lambda([](){return FLHPresentationSettings::Get().HighContrast?FLinearColor::Black:FLHUIStyle().Panel;}).Padding(12)
    [Text(TAttribute<FText>::CreateLambda([this,I](){return FText::FromString(P->GameplayLabel(FName(*FString::Printf(TEXT("Ability%d"),I+1))));}))]]];
}

void SLHGameplayHud::Tick(const FGeometry& G,double Time,float Delta)
{
    SCompoundWidget::Tick(G,Time,Delta);
    Cues.Observe(P->Hud(),P->Snapshot().Character.EarnedLevel,Delta);
    UWorld* World=GEngine?GEngine->GetCurrentPlayWorld():nullptr;
    if(!World) return;
    // Weak Slate subscriptions cannot extend actor/widget lifetime or mutate combat.
    for(TObjectIterator<ULHCombatComponent> It;It;++It)
        if(It->GetWorld()==World && !Bound.Contains(*It))
        { Bound.Add(*It); It->OnAttackCommitted.AddSP(this,&SLHGameplayHud::Committed); It->OnImpact.AddSP(this,&SLHGameplayHud::Impact); }
    for(auto It=Bound.CreateIterator();It;++It) if(!It->IsValid()) It.RemoveCurrent();
}
void SLHGameplayHud::Committed(const FLHAttackEvent& Event)
{
    const auto Player=P->Hud().Player;
    if(Event.Source.InstanceId==Player.InstanceId || Event.Target.InstanceId==Player.InstanceId)
    { if(LocalActions.Num()>=32) LocalActions.Empty(); LocalActions.Add(Event.Identity.ActivationId); }
}
void SLHGameplayHud::Impact(const FLHHitIdentity& Id,const LH::Rules::FCombatResult& Result)
{
    if(!LocalActions.Contains(Id.ActivationId) || SeenImpacts.Contains(Id)) return;
    if(SeenImpacts.Num()>=32) SeenImpacts.Empty(); SeenImpacts.Add(Id);
    Cues.Hit(Result.bHit,Result.Damage);
}
