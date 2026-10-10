#pragma once
#include "Subsystems/GameInstanceSubsystem.h"
#include "LHTravelSaveAdapter.h"
#include "Engine/EngineBaseTypes.h"
#include "LHWorldTravelSubsystem.generated.h"

struct LIGHTHAVEN_API FLHWorldTravelBindings
{
    // These are session authority adapters, not Blueprint teleport commands.
    TFunction<void(bool)> FreezeInteractionAndAutosaves;
    TFunction<bool(FLHSaveSnapshot&,FString&)> SettleAndCapture;
    TFunction<void(const FLHSaveSnapshot&)> CheckpointDurable;
    // Rebuild canonical session/avatar, check enemy safety/collision, and place capsule at ground pivot.
    // Must not grant rewards, advance clocks or alter the provided canonical snapshot.
    TFunction<bool(FLHSaveSnapshot&,FString&)> PrepareArrival;
    TFunction<bool(const FLHSaveSnapshot&,UWorld*,const FLHEntranceDefinition&,FString&)> InstallCheckpoint;
};
UCLASS()
class LIGHTHAVEN_API ULHWorldTravelSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    bool Configure(TSharedRef<FLHSaveStore> Store,const FLHSaveCompatibility& Compatibility,FLHWorldTravelBindings Bindings,FString& Error);
    bool RequestTravel(const FLHRequestTravelRequest& Request,FString& Error);
    FLHTravelCoordinator* Travel() const;
private:
    TUniquePtr<FLHTravelSaveAdapter> Adapter;
    FLHWorldTravelBindings SessionBindings;
    FDelegateHandle MapLoadedHandle, TravelFailureHandle;
    uint64 MapToken=0;
    bool bRestoring=false;
    FLHSaveSnapshot Recovery;
    FLHEntranceId ExpectedEntrance;
    void OpenMap(const FLHAreaDefinition& Area,const FLHEntranceId& Entrance,uint64 Token,bool bRecovery);
    void OnMapLoaded(UWorld* World);
    void OnTravelFailure(UWorld* World,ETravelFailure::Type Type,const FString& Error);
    bool Install(const FLHSaveSnapshot& Snapshot,UWorld* World,const FLHEntranceDefinition& Entrance,FString& Error);
    void FailLoad(const FString& Error);
};
