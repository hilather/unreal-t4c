#include "Rules/LHRules.h"
#include <cmath>
namespace LH::Rules
{
FCombatParameters MakeStage1PrototypeCombat()
{
    FCombatParameters P; P.Model=ECombatModel::Stage1Ratio;
    auto N=[](double V) { FLHNumber N; N.Value=V; N.Resolution=ELHValueResolution::Resolved; N.Provenance.Status=ELHProvenanceStatus::Prototype; N.Provenance.SourceBaseline=TEXT("rules-ledger Physical resolution; replace after combat fixtures/balance review"); N.Provenance.SourceUrl=TEXT("https://www.t4cbible.com/skills"); N.Provenance.RetrievedDate=TEXT("2026-10-07"); return N; };
    P.HitBase=N(0); P.AccuracyScale=N(0); P.AvoidanceScale=N(0); P.MinimumChance=N(.10); P.MaximumChance=N(.95);
    P.ArmorScale=N(1); P.ResistanceScale=N(0); P.MinimumDamage=N(1); P.DamageQuantum=N(1); return P;
}
double PhysicalAttributeBonus(double Strength, double Agility, bool bBow) { return std::floor(bBow ? (Strength+Agility)/10 : Strength/5); }
}
