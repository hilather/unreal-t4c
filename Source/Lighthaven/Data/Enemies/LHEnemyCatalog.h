#pragma once
#include "AI/LHEnemyRuntime.h"
#include "Rewards/LHRewardTypes.h"
struct LIGHTHAVEN_API FLHEnemyCatalogRow
{
    FLHContentId Id, PresentationId;
    FLHInteger Level, ExperienceColumns[3], GoldMinimum, GoldMaximum;
    FLHNumber Health, MinimumDamage, MaximumDamage;
    FLHFieldProvenance LootMembership;
    TArray<FLHMechanicalField> AIParameters;
    FLHEnemyRuntimeSpec Runtime;
    FLHKillRewardSpec Reward;
};
namespace LHEnemyData
{
LIGHTHAVEN_API const TArray<FLHEnemyCatalogRow>& Catalog();
LIGHTHAVEN_API const FLHEnemyCatalogRow* Find(const FLHContentId&);
}
