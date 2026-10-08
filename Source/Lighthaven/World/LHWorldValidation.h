#pragma once
#include "LHAreaRegistry.h"
enum class ELHPlacedIdKind : uint8 { Spawn, Interactable, Portal };
struct LIGHTHAVEN_API FLHPlacedIdentity
{
    FLHAreaId Area;
    FGuid Id;
    ELHPlacedIdKind Kind=ELHPlacedIdKind::Spawn;
    FLHContentId Definition;
    FLHEntranceId Source, Destination;
    bool bReturn=false;
    FString Context;
};
struct LIGHTHAVEN_API FLHPlacedEntrance
{
    FLHEntranceId Id;
    FTransform Transform;
    bool bSafetyReviewed=false;
    FString Context;
};
namespace LHWorld
{
    // Synthetic-data-capable validator shared by editor scan and native tests.
    LIGHTHAVEN_API bool ValidatePlacements(const TArray<FLHAreaId>& Maps,
        const TArray<FLHPlacedIdentity>& Identities, const TArray<FLHPlacedEntrance>& Entrances,
        TArray<FString>& Errors);
}
