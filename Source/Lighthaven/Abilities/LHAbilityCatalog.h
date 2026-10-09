#pragma once
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHEquipmentCombat.h"
enum class ELHAbilityEffect : uint8 { PhysicalDamage, SpellDamage, Heal, Light };
enum class ELHAbilityTarget : uint8 { Hostile, Self, Friendly };
struct LIGHTHAVEN_API FLHAbilityCatalogRow
{
    FLHContentId Id;
    FLHEligibility Eligibility;
    FLHInteger LearningSkillPoints, LearningGold;
    FLHNumber ManaCost, RangeCm, CooldownSeconds, ImpactSeconds, MinimumMagnitude, MaximumMagnitude, DurationSeconds;
    ELHAbilityEffect Effects=ELHAbilityEffect::PhysicalDamage;
    ELHAbilityTarget TargetPolicy=ELHAbilityTarget::Hostile;
    FName ClassificationTag;
    FName RefundPolicy=TEXT("NoRefundAfterCommit");
};
namespace LHAbilities
{
LIGHTHAVEN_API const TArray<FLHAbilityCatalogRow>& Catalog();
LIGHTHAVEN_API const FLHAbilityCatalogRow* Find(const FLHContentId&);
LIGHTHAVEN_API FLHNumber PrototypeNumber(double Value, const TCHAR* Note);
LIGHTHAVEN_API bool BuildAttackConfig(const FLHSaveSnapshot&, const FLHContentId&, const FLHCombatItemLookup&, const LH::Rules::FCombatParameters&, FLHBasicAttackConfig&, FString&);
struct FLHUseAbilityContext
{
    ULHCombatComponent* Source=nullptr;
    ULHCombatComponent* Target=nullptr;
    FLHSaveSnapshot Snapshot;
    FLHCombatItemLookup ItemLookup;
    LH::Rules::FCombatParameters Combat;
    FLHEntityId SourceId, TargetId;
    bool bTargetFriendly=false;
};
LIGHTHAVEN_API ELHCommandReason ExecuteUseAbility(FLHUseAbilityContext&, const FLHUseAbilityRequest&);
}
