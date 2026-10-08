#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "LHSaveStore.h"
#include "LHSaveSubsystem.generated.h"

UCLASS()
class LIGHTHAVEN_API ULHSaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // Configure with the authoritative mechanical closure/catalog hashes before enumerate/load/save.
    void Configure(const FLHSaveCompatibility& InCompatibility);
    TArray<FLHSaveProfile> EnumerateCharacters();
    bool LoadCharacter(const FLHCharacterId& Character, FLHSaveSnapshot& Snapshot, FLHSaveError& Error);
    bool SaveSnapshot(const FLHSaveSnapshot& Snapshot, bool bCompletedActionBoundary, FLHSaveError& Error);
    bool RetrySave(const FLHCharacterId& Character, FLHSaveError& Error);
    bool IsDirty(const FLHCharacterId& Character) const;
    bool IsWriting(const FLHCharacterId& Character) const;
    TSharedPtr<FLHSaveStore> GetStore() const { return Store; } // Game-thread session adapter owns event lifetime.
    FLHSaveEvents Events;
private:
    TSharedPtr<FLHSaveStore> Store;
    FLHSaveCompatibility Compatibility;
    bool bConfigured = false;
    bool Ready(FLHSaveError& Error) const;
    void ForwardEvent(const FLHSaveEvent& Event);
};
