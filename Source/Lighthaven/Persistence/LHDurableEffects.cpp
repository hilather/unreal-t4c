#include "Persistence/LHDurableEffects.h"
namespace LHDurableEffectsPrivate
{
const TCHAR* Keys[]={TEXT("EpochA"),TEXT("EpochB"),TEXT("EpochC"),TEXT("EpochD")};
FLHNumber Number(double V)
{
    FLHNumber N; N.Resolution=ELHValueResolution::Resolved; N.Value=V;
    N.Provenance.Status=ELHProvenanceStatus::Prototype;
    N.Provenance.Notes=TEXT("Runtime active-seconds remainder / epoch metadata; no offline catch-up"); return N;
}
}
bool LHSave::ValidateDurableLight(const FLHDurableEffectRecord& R,const FGuid& Epoch)
{
    const auto& N=R.RemainingSeconds;
    if (!Epoch.IsValid() || R.Effect.Value.ToString()!=TEXT("Effect.SpellLight") ||
        !R.Owner.RunId.IsValid() || !R.Owner.InstanceId.IsValid() || R.Owner.Area.Content.Value.IsNone() ||
        R.Source.RunId!=R.Owner.RunId || R.Source.InstanceId!=R.Owner.InstanceId || R.Source.Area.Content.Value.ToString()!=R.Owner.Area.Content.Value.ToString() ||
        N.Resolution!=ELHValueResolution::Resolved || !FMath::IsFinite(N.Value) || N.Value<=0 || N.Value>600 ||
        R.Stacks.Resolution!=ELHValueResolution::Resolved || R.Stacks.Value!=1 || R.CanonicalInputs.Num()!=4) return false;
    const uint32 Words[]={Epoch.A,Epoch.B,Epoch.C,Epoch.D};
    for(int32 I=0;I<4;++I) {
        const auto& F=R.CanonicalInputs[I];
        if(F.Key.ToString()!=LHDurableEffectsPrivate::Keys[I] || F.Value.Resolution!=ELHValueResolution::Resolved || F.Value.Value!=double(Words[I])) return false;
    }
    return true;
}
FLHDurableEffectRecord LHSave::MakeDurableLight(const FLHEntityId& Owner,const FGuid& Epoch,double Remaining)
{
    FLHDurableEffectRecord R; R.Owner=R.Source=Owner; R.Effect.Value=TEXT("Effect.SpellLight");
    R.RemainingSeconds=LHDurableEffectsPrivate::Number(Remaining);
    R.Stacks.Resolution=ELHValueResolution::Resolved; R.Stacks.Value=1; R.Stacks.Provenance.Status=ELHProvenanceStatus::Prototype;
    const uint32 Words[]={Epoch.A,Epoch.B,Epoch.C,Epoch.D};
    for(int32 I=0;I<4;++I) { FLHMechanicalField F; F.Key=LHDurableEffectsPrivate::Keys[I]; F.Value=LHDurableEffectsPrivate::Number(Words[I]); R.CanonicalInputs.Add(F); }
    return R;
}
