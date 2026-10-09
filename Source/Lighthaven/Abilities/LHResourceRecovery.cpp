#include "Abilities/LHResourceRecovery.h"
#include "Abilities/LHAbilityCatalog.h"
namespace LHResourceRecoveryPrivate
{
bool Same(const FLHEntityId& A,const FLHEntityId& B) {return A.RunId==B.RunId && A.InstanceId==B.InstanceId && A.Area.Content.Value==B.Area.Content.Value;}
}
namespace LHAbilities
{
bool AdvanceManaRegen(FLHSaveSnapshot& S,double Seconds,const LH::Rules::FManaParameters& P)
{
    for(const auto* N:{&S.Character.CurrentMana,&S.Character.EarnedBaseMana,&S.Character.CurrentHealth,&S.Session.ManaRegenFractionalSeconds}) if(N->Resolution!=ELHValueResolution::Resolved) return false;
    LH::Rules::FManaInput I; I.Current=S.Character.CurrentMana.Value; I.Maximum=S.Character.EarnedBaseMana.Value; I.FractionalSeconds=S.Session.ManaRegenFractionalSeconds.Value; I.ActiveSeconds=Seconds; I.bAlive=S.Character.CurrentHealth.Value>0;
    const auto R=LH::Rules::RegenerateMana(P,I); if(!R.Diagnostic.IsAccepted()) return false;
    S.Character.CurrentMana.Value=R.Value.Current; S.Session.ManaRegenFractionalSeconds.Value=R.Value.FractionalSeconds; return true;
}
void CaptureCooldowns(const ULHCombatComponent& C,const FLHEntityId& Owner,TArray<FLHCooldownRecord>& Out)
{
    Out.RemoveAll([&](const auto& R){return LHResourceRecoveryPrivate::Same(R.Owner,Owner);});
    for(const auto& E:C.GetCooldownMap()) {FLHCooldownRecord R; R.Owner=Owner; R.Ability.Value=E.Key; R.RemainingSeconds=PrototypeNumber(E.Value,TEXT("Active simulation cooldown remainder; no offline catch-up")); Out.Add(R);}
}
bool RestoreCooldowns(ULHCombatComponent& C,const FLHEntityId& Owner,const TArray<FLHCooldownRecord>& In)
{
    TMap<FName,double> R;
    for(const auto& E:In) if(LHResourceRecoveryPrivate::Same(E.Owner,Owner)) {
        if(!Find(E.Ability) || E.RemainingSeconds.Resolution!=ELHValueResolution::Resolved || R.Contains(E.Ability.Value) || !FMath::IsFinite(E.RemainingSeconds.Value) || E.RemainingSeconds.Value<0) return false;
        R.Add(E.Ability.Value,E.RemainingSeconds.Value);
    }
    return C.RestoreCooldownMap(R);
}
}
