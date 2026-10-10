#include "Abilities/LHLightEffect.h"
#include "Persistence/LHDurableEffects.h"
namespace LHAbilities
{
bool CaptureLightEffect(const ULHCombatComponent& C,const FLHEntityId& Owner,FLHSessionRecord& S)
{
    if(C.IsActionPending() || C.IsPublishingActionEvents() || S.EffectPolicy!=ELHEffectSavePolicy::CompletedActionBoundaryOnly) return false;
    const double Remaining=C.GetLightRemainingSeconds();
    auto R=LHSave::MakeDurableLight(Owner,S.RequestEpoch,Remaining);
    // Validate owner and epoch even for empty capture.
    auto Check=R; Check.RemainingSeconds.Value=1;
    if(!LHSave::ValidateDurableLight(Check,S.RequestEpoch)) return false;
    S.DurableEffects.Reset(); if(Remaining>0) S.DurableEffects.Add(R); return true;
}
bool RestoreLightEffect(ULHCombatComponent& C,const FLHEntityId& Owner,const FLHSessionRecord& S)
{
    if(S.EffectPolicy!=ELHEffectSavePolicy::CompletedActionBoundaryOnly || S.DurableEffects.Num()>1) return false;
    double Remaining=0;
    for(const auto& R:S.DurableEffects) {
        if(!LHSave::ValidateDurableLight(R,S.RequestEpoch) || R.Owner.RunId!=Owner.RunId || R.Owner.InstanceId!=Owner.InstanceId || R.Owner.Area.Content.Value.ToString()!=Owner.Area.Content.Value.ToString()) return false;
        Remaining=R.RemainingSeconds.Value;
    }
    return C.RestoreLightRemainingSeconds(Remaining);
}
}
