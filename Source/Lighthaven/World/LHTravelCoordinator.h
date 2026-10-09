#pragma once
#include "Core/LHCommands.h"
#include "Core/LHSaveSnapshot.h"
#include "LHAreaRegistry.h"

enum class ELHTravelPhase : uint8 { Idle, SavingSource, SourceSaveFailed, LoadingDestination, SavingArrival, ArrivalSaveFailed, RestoringSource, RecoveryFailed };
// Host owns actual session/GAS/world loading. Completion notifications carry an operation token.
// All methods and completions are game-thread-only; completions may run inline.
class LIGHTHAVEN_API ILHTravelHost
{
public:
    virtual ~ILHTravelHost() = default;
    virtual void FreezeInteraction(bool bFreeze) = 0;
    // Cancel/finish action by session policy, capture canonical world/character between transactions.
    virtual bool SettleAndCapture(FLHSaveSnapshot& Out, FString& Error) = 0;
    // Preserve snapshot until durability completion. Suppress all unrelated autosaves while frozen.
    virtual void SaveCheckpoint(const FLHSaveSnapshot& Snapshot, uint64 Token) = 0;
    // Publish the actual store high-water sequence to the session before continuing.
    virtual void CheckpointDurable(const FLHSaveSnapshot& Snapshot) {}
    virtual void LoadDestination(const FLHAreaDefinition& Area, uint64 Token) = 0;
    // Validate actual loaded map, entrance marker, collision/enemy safety; install before arrival save.
    virtual bool ValidateAndInstallArrival(const FLHSaveSnapshot& Snapshot, const FLHEntranceDefinition& Entrance, FString& Error) = 0;
    // Reload/rebuild canonical source and place at its source entrance; notify restoration completion.
    virtual void RestoreSource(const FLHSaveSnapshot& Snapshot, const FLHEntranceDefinition& Entrance, uint64 Token) = 0;
};

class LIGHTHAVEN_API FLHTravelCoordinator
{
public:
    explicit FLHTravelCoordinator(ILHTravelHost& InHost) : Host(InHost) {}
    bool Begin(const FLHRequestTravelRequest& Request, FString& Error);
    void OnSaveCompleted(uint64 Token, bool bSuccess, int64 DurableSequence, const FString& Error);
    // LoadedEntrance must be the actual marker's identity. Host performs geometry safety checks too.
    void OnDestinationLoaded(uint64 Token, bool bSuccess, const FLHEntranceId& LoadedEntrance, const FString& Error);
    void OnSourceRestored(uint64 Token, bool bSuccess, const FString& Error);
    bool Retry(FString& Error);
    bool Cancel(FString& Error);
    ELHTravelPhase GetPhase() const { return Phase; }
    bool IsFrozen() const { return Phase!=ELHTravelPhase::Idle; }
    uint64 GetToken() const { return Operation; }
    const FString& GetError() const { return LastError; }
    const FLHSaveSnapshot& SourceCheckpoint() const { return Source; }
private:
    ILHTravelHost& Host;
    ELHTravelPhase Phase=ELHTravelPhase::Idle;
    uint64 Operation=0;
    FLHSaveSnapshot Source, Arrival;
    FLHPortalDefinition Edge;
    FString LastError;
    bool bDispatching=false;
    void Recover(const FString& Error);
    void Finish();
};
