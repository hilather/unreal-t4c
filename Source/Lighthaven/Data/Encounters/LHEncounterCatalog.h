#pragma once
#include "World/LHAreaRegistry.h"
#include "Rewards/LHRewardTypes.h"
struct LIGHTHAVEN_API FLHEncounterCatalogRow
{
    FGuid SpawnId;
    FLHAreaId Area;
    FLHContentId Enemy;
    FLHNumber RespawnSeconds, SafetyDistanceCm;
    ELHRespawnPolicy RespawnPolicy=ELHRespawnPolicy::OrdinaryRepeat;
    bool bBoss=false;
    ELHValueResolution PlacementResolution=ELHValueResolution::Unresolved;
    FLHFieldProvenance PlacementProvenance;
    TArray<FLHPolicyField> Policies;
};
namespace LHEncounterData
{
LIGHTHAVEN_API const TArray<FLHEncounterCatalogRow>& Catalog();
LIGHTHAVEN_API const FLHEncounterCatalogRow* ForSpawn(const FGuid&);
}
