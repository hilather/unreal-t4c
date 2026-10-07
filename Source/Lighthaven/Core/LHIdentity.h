// DRAFT schema rev 1 — NOT COMPILED (no Unreal Engine installed); integrator review pending
#pragma once

#include "CoreMinimal.h"
#include "LHIdentity.generated.h"

// Invalid until explicitly assigned. Local identity; never an account or display name.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHCharacterId
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FGuid Value;
};

// Invalid until explicitly assigned. Local identity; never an account or display name.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHRequestId
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FGuid Value;
};

// Invalid until explicitly assigned. Local identity; never an account or display name.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHRewardId
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FGuid Value;
};

// Canonical namespaced ID, e.g. Item.RustedDirk. Never derived from asset filename.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHContentId
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName Value = NAME_None;
};

// Area definition identity, independent of map filename.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHAreaId
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Content;
};

// Entrance identity is area-qualified.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHEntranceId
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHAreaId Area;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName LocalId = NAME_None;
};

// Stable placed or runtime instance in one campaign; not an actor GUID.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHEntityId
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FGuid RunId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHAreaId Area;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FGuid InstanceId;
};

// Slot is authored, generation increments only on committed respawn; zero is initial life, not an unresolved mechanic.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHSpawnLifeId
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHAreaId Area;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FGuid SpawnSlot;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) int64 LifeGeneration = 0;
};
