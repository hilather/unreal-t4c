#pragma once
#include "Abilities/LHCombatComponent.h"

// Catalog-owned values; none of these fields supplies historical or fallback tuning.
struct LIGHTHAVEN_API FLHEnemyRuntimeSpec
{
    FLHContentId ContentId, PresentationId;
    FLHNumber CapsuleRadiusCm, CapsuleHalfHeightCm, MaxHealth;
    FLHBasicAttackConfig Attack;
    LH::Rules::FRequirementInput Requirements;
    FLHNumber Accuracy, Avoidance, Armor, Resistance, DamageBonus, MaxMana;
    bool bBoss = false;
    FLHNumber AggroRadiusCm, LeashRadiusCm, MoveSpeedCmPerSec;
    FLHNumber PathRetryLimit, MaxActivePursuers, DecisionIntervalSeconds, HomeToleranceCm, SpawnSafetyDistanceCm;
};
namespace LHAI
{
struct LIGHTHAVEN_API FParameterKey { FName Name; const TCHAR* Unit; const TCHAR* Meaning; };
LIGHTHAVEN_API const TArray<FParameterKey>& AllowedParameterKeys();
// Atomic: rejected inputs leave InOut unchanged. All registered keys are required.
LIGHTHAVEN_API bool ApplyParameters(const TArray<FLHMechanicalField>& AIParameters, FLHEnemyRuntimeSpec& InOut, FString& Error);
LIGHTHAVEN_API bool ValidateSpec(const FLHEnemyRuntimeSpec& Spec, FString& Error);
LIGHTHAVEN_API bool SameLife(const FLHSpawnLifeId& A, const FLHSpawnLifeId& B);
}
