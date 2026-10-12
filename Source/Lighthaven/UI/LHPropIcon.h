#pragma once
#include "Widgets/SCompoundWidget.h"
#include "Styling/SlateBrush.h"
// The brush belongs to the widget, never a temporary. Catalogue owns texture lifetime.
class LIGHTHAVEN_API SLHPropIcon : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLHPropIcon) {} SLATE_ATTRIBUTE(FName, Identity) SLATE_END_ARGS()
    void Construct(const FArguments& Args);
    const FSlateBrush* CurrentBrush() const;
private:
    TAttribute<FName> Identity;
    mutable FSlateBrush Brush;
};
