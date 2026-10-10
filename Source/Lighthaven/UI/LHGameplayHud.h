#pragma once
#include "Widgets/SCompoundWidget.h"
#include "UI/LHUIPresenter.h"
#include "UI/LHPresentation.h"
#include "Abilities/LHCombatComponent.h"
// Read-only HUD: bars are hidden when unresolved; never a guessed resource fraction.
class LIGHTHAVEN_API SLHGameplayHud : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLHGameplayHud) {} SLATE_ARGUMENT(FLHUIPresenter*, Presenter) SLATE_END_ARGS()
    void Construct(const FArguments&);
    virtual void Tick(const FGeometry&, double, float) override;
private:
    FLHPresentationCues Cues;
    TSet<FGuid> LocalActions;
    TSet<FLHHitIdentity> SeenImpacts;
    void Committed(const FLHAttackEvent&);
    TSet<TWeakObjectPtr<ULHCombatComponent>> Bound;
    void Impact(const FLHHitIdentity&, const LH::Rules::FCombatResult&);
    FLHUIPresenter* P=nullptr;
};
