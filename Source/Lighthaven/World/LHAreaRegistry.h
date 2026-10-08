#pragma once
#include "Core/LHDefinitions.h"

// Runtime value view of the frozen Core DTOs. Candidate transforms are ground pivots.
struct LIGHTHAVEN_API FLHSpawnAuthoring
{
    FName Alias;
    FGuid SpawnId;
    FLHContentId Enemy;
    FTransform Anchor;
};
struct LIGHTHAVEN_API FLHAreaDefinition
{
    FLHAreaId Id;
    FSoftObjectPath Map;
    TArray<FLHEntranceDefinition> Entrances;
    FLHEntranceId SafeFallback;
    TArray<FLHPortalDefinition> Portals;
    TArray<FLHSpawnAuthoring> Spawns;
};
namespace LHWorld
{
    LIGHTHAVEN_API const TArray<FLHAreaDefinition>& Registry();
    LIGHTHAVEN_API const FLHAreaDefinition* FindArea(const FLHAreaId& Id);
    LIGHTHAVEN_API const FLHEntranceDefinition* FindEntrance(const FLHEntranceId& Id);
    LIGHTHAVEN_API const FLHPortalDefinition* FindPortal(const FLHEntityId& Id);
    LIGHTHAVEN_API bool SameArea(const FLHAreaId& A, const FLHAreaId& B);
    LIGHTHAVEN_API bool SameEntrance(const FLHEntranceId& A, const FLHEntranceId& B);
    LIGHTHAVEN_API bool SafeTransform(const FTransform& Transform);
    LIGHTHAVEN_API bool ValidateRegistry(TArray<FString>& Errors);
}
