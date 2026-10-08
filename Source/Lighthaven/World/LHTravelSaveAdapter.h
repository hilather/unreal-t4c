#pragma once
#include "LHTravelCoordinator.h"
#include "Persistence/LHSaveStore.h"

// Concrete persistence bridge; session/map hooks remain integrator-owned.
// Destruct only when no travel is pending, before destroying the bound hooks.
class LIGHTHAVEN_API FLHTravelSaveAdapter : public ILHTravelHost
{
public:
    struct FHooks
    {
        TFunction<void(bool)> Freeze;
        TFunction<bool(FLHSaveSnapshot&,FString&)> Capture;
        TFunction<void(const FLHSaveSnapshot&)> Durable;
        TFunction<void(const FLHAreaDefinition&,uint64)> Load;
        TFunction<bool(const FLHSaveSnapshot&,const FLHEntranceDefinition&,FString&)> Install;
        TFunction<void(const FLHSaveSnapshot&,const FLHEntranceDefinition&,uint64)> Restore;
    };
    FLHTravelSaveAdapter(TSharedRef<FLHSaveStore> InStore, const FLHSaveCompatibility& InCompatibility, FHooks InHooks);
    virtual ~FLHTravelSaveAdapter();
    FLHTravelCoordinator& Travel() { return Coordinator; }
    virtual void FreezeInteraction(bool bFreeze) override;
    virtual bool SettleAndCapture(FLHSaveSnapshot& Out,FString& Error) override;
    virtual void SaveCheckpoint(const FLHSaveSnapshot& Snapshot,uint64 Token) override;
    virtual void CheckpointDurable(const FLHSaveSnapshot& Snapshot) override;
    virtual void LoadDestination(const FLHAreaDefinition& Area,uint64 Token) override;
    virtual bool ValidateAndInstallArrival(const FLHSaveSnapshot& Snapshot,const FLHEntranceDefinition& Entrance,FString& Error) override;
    virtual void RestoreSource(const FLHSaveSnapshot& Snapshot,const FLHEntranceDefinition& Entrance,uint64 Token) override;
private:
    TSharedRef<FLHSaveStore> Store;
    FLHSaveCompatibility Compatibility;
    FHooks Hooks;
    FLHTravelCoordinator Coordinator;
    FDelegateHandle SaveHandle;
    FGuid Character;
    uint64 SaveToken=0;
    bool bAwaitingSave=false;
    void OnSave(const FLHSaveEvent& Event);
};
