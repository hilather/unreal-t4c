#pragma once
#include "GameFramework/Actor.h"
#include "Core/LHSaveSnapshot.h"
#include "LHWorldMarkers.generated.h"

UENUM(BlueprintType)
enum class ELHPortalDirection : uint8 { Descent, Return };

// Explicit interaction only: no overlap automatically starts travel.
UCLASS(BlueprintType)
class LIGHTHAVEN_API ALHPortal : public AActor
{
    GENERATED_BODY()
public:
    ALHPortal();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FGuid PortalId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FLHEntranceId Source;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FLHEntranceId Destination;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") ELHPortalDirection Direction = ELHPortalDirection::Descent;
    FLHEntityId Materialize(const FGuid& Run) const;
};
UCLASS(BlueprintType)
class LIGHTHAVEN_API ALHEntranceMarker : public AActor
{
    GENERATED_BODY()
public:
    ALHEntranceMarker();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FLHEntranceId EntranceId;
    // Absolute ground-contact pivot; spawning must add actual pawn capsule half-height.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FTransform SafeArrivalTransform;
    // Set only after geometry, enemy reach and arrival clearance have been checked.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") bool bSafetyReviewed = false;
};
UCLASS(BlueprintType)
class LIGHTHAVEN_API ALHSpawnMarker : public AActor
{
    GENERATED_BODY()
public:
    ALHSpawnMarker();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FLHAreaId Area;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FGuid SpawnId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FLHContentId EnemyDefinitionId;
    // Hydration chooses the persisted high-water generation, never resets it on map load.
    bool ResolveLife(const FLHAreaRecord& Record, FLHSpawnLifeId& Out) const;
};
UCLASS(BlueprintType)
class LIGHTHAVEN_API ALHInteractableMarker : public AActor
{
    GENERATED_BODY()
public:
    ALHInteractableMarker();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FLHAreaId Area;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FGuid InstanceId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighthaven") FLHContentId DefinitionId;
    FLHEntityId Materialize(const FGuid& Run) const;
};
