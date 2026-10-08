#include "Framework/LHDevCombatFixture.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Framework/LHCharacter.h"
#include "Engine/World.h"
namespace
{
FLHNumber Number(double V)
{
    FLHNumber N; N.Resolution = ELHValueResolution::Resolved; N.Value = V;
    N.Provenance.Status = ELHProvenanceStatus::Prototype;
    N.Provenance.SourceBaseline = TEXT("W1-INT generated dev-map fixture / Prototype / 2026-10-08; no historical source");
    return N;
}
FLHInteger Integer(int64 V)
{
    FLHInteger I; I.Resolution = ELHValueResolution::Resolved; I.Value = V;
    I.Provenance = Number(0).Provenance; return I;
}
}
void LHDevCombat::InitializeForMap(ULHCombatComponent* Combat, AActor* Avatar)
{
    if (!Combat || !Avatar || !Avatar->GetWorld()) return;
    const FString Map = Avatar->GetWorld()->GetMapName();
    // PIE prefixes are allowed; unrelated maps cannot receive debug resources.
    if (!Map.EndsWith(TEXT("Dev_Combat")) && !Map.EndsWith(TEXT("Dev_Movement"))) return;
    if (!Avatar->IsA<ALHCharacter>() && !Avatar->ActorHasTag(TEXT("LH.Dev.CombatFixture"))) return;
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetMaxHealthAttribute(), 100);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), 100);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetMaxManaAttribute(), 100);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(), 100);
    FLHBasicAttackConfig Config;
    Config.Combat.HitBase = Number(1); Config.Combat.AccuracyScale = Number(0); Config.Combat.AvoidanceScale = Number(0);
    Config.Combat.MinimumChance = Number(1); Config.Combat.MaximumChance = Number(1);
    Config.Combat.ArmorScale = Number(0); Config.Combat.ResistanceScale = Number(0);
    Config.Combat.MinimumDamage = Number(0); Config.Combat.DamageQuantum = Number(1);
    Config.RequirementPolicy.Basis = LH::Rules::EAttributeBasis::Base;
    Config.RequirementPolicy.BasisProvenance = Number(0).Provenance;
    Config.Eligibility.MinimumLevel = Integer(0);
    auto& A = Config.Eligibility.MinimumAttributes;
    A.Strength = Integer(0); A.Endurance = Integer(0); A.Agility = Integer(0); A.Intelligence = Integer(0); A.Wisdom = Integer(0);
    Config.ManaCost = Number(2); Config.CooldownSeconds = Number(3); Config.ImpactSeconds = Number(1);
    Config.RangeCm = Number(200); Config.WeaponMinimum = Number(10); Config.WeaponMaximum = Number(10);
    Combat->ConfigureAttack(Config, {});
    Combat->SetCombatRandomState(FRandomStream(123));
}
