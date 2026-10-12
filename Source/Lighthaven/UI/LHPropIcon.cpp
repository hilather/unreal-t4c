#include "UI/LHPropIcon.h"
#include "UI/LHUIStyle.h"
#include "Visual/Props/LHPropsCatalog.h"
#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/CoreStyle.h"
void SLHPropIcon::Construct(const FArguments& Args)
{
    Identity=Args._Identity; const FLHUIStyle Style;
    Brush.ImageSize=FVector2D(Style.Icon,Style.Icon); Brush.DrawAs=ESlateBrushDrawType::Image;
    SetVisibility(TAttribute<EVisibility>::CreateLambda([this](){return CurrentBrush()?EVisibility::HitTestInvisible:EVisibility::Collapsed;}));
    ChildSlot[SNew(SBox).WidthOverride(Style.Icon).HeightOverride(Style.Icon)
        [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Style.Raised).Padding(0)
        [SNew(SImage).Image_Lambda([this](){return CurrentBrush();})]]];
}
const FSlateBrush* SLHPropIcon::CurrentBrush() const
{
    auto* Texture=ULHPropsCatalog::Icon(Identity.Get()); Brush.SetResourceObject(Texture); return Texture?&Brush:nullptr;
}
