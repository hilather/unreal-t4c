#pragma once
#include "Subsystems/WorldSubsystem.h"
#include "Core/LHSaveSnapshot.h"
#include "AI/LHEnemyRuntime.h"
#include "LHEncounterDirector.generated.h"
class ALHEnemyCharacter;
class ALHEnemyAIController;
UCLASS()
class LIGHTHAVEN_API ULHEncounterDirector : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    TFunction<const FLHEnemyRuntimeSpec*(const FLHContentId&)> ResolveSpec;
    TFunction<bool(const FLHSpawnLifeId&, const FLHHitIdentity&, AActor*)> SettleKill;
    TFunction<void(float)> AdvanceRespawns;
    // Integration owns run identity, live player and freeze. No session/rewards dependency.
    FGuid RunId;
    TWeakObjectPtr<AActor> Player;
    void SetSimulationFrozen(bool Frozen);
    bool IsSimulationEnabled() const;
    void Populate(const FLHAreaRecord& Area);
    void CaptureLive(FLHAreaRecord& Area) const;
    void SpawnLife(const FLHSpawnLifeId& Life);
    void Despawn(const FLHSpawnLifeId& Life);
    void TickActiveSimulation(float Seconds);
    bool IsSpawnSafe(const FGuid& SpawnId) const;
    ALHEnemyCharacter* FindByLife(const FLHSpawnLifeId& Life) const;
    ALHEnemyCharacter* FindByEntity(const FLHEntityId& Entity) const;
    bool CanPursue(const ALHEnemyAIController* Controller, int32 Cap) const;
    bool IsSafeSegment(const FVector& Start, const FVector& End, float Radius = 0, float HalfHeight = 0) const;
    bool HasSight(const AActor* Source, const AActor* Target) const;
    virtual void Deinitialize() override;
private:
    void SpawnRecord(const FLHEncounterRecord& Record);
    void HandleDeath(ALHEnemyCharacter* Enemy, const FLHHitIdentity& Hit);
    UPROPERTY() TArray<TObjectPtr<ALHEnemyCharacter>> Enemies;
    // Settlement latch survives actor despawn/repopulation for this loaded floor.
    TArray<FLHSpawnLifeId> PublishedDeaths;
    FLHAreaId ActiveArea;
    TArray<FBox> SafetyBounds;
    bool bLoaded = false, bFrozen = false;
};
