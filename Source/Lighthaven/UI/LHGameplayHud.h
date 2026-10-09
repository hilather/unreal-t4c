#pragma once
#include "Widgets/SCompoundWidget.h"
#include "UI/LHUIPresenter.h"
// Read-only HUD: bars are hidden when unresolved; never a guessed resource fraction.
class LIGHTHAVEN_API SLHGameplayHud : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLHGameplayHud) {} SLATE_ARGUMENT(FLHUIPresenter*, Presenter) SLATE_END_ARGS()
    void Construct(const FArguments&);
private:
    FLHUIPresenter* P=nullptr;
};
