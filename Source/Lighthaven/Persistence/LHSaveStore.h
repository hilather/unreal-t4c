#pragma once

#include "LHSaveCodec.h"

// Only local generation storage; no live actors. All callbacks and coordinator calls are game-thread-only.
class LIGHTHAVEN_API ILHSaveStorage
{
public:
    virtual ~ILHSaveStorage() = default;
    virtual TArray<FGuid> Enumerate() = 0;
    virtual bool Read(const FGuid& Character, int32 Slot, TArray<uint8>& Bytes, FLHSaveError& Error) = 0;
    // Own Bytes until completion; never reference caller memory. Completion occurs on the game thread.
    virtual void Write(const FGuid& Character, int32 Slot, TArray<uint8> Bytes, TFunction<void(bool)> Complete) = 0;
};

// Size is checked on the open handle before buffer allocation. Default path: Saved/SaveGames/Lighthaven/.
LIGHTHAVEN_API TSharedRef<ILHSaveStorage> LHCreateLocalSaveStorage(const FString& Directory);

enum class ELHSaveEventKind : uint8 { Succeeded, Failed, Recovered, Unreadable };
struct LIGHTHAVEN_API FLHSaveEvent
{
    ELHSaveEventKind Kind = ELHSaveEventKind::Unreadable;
    FLHCharacterId Character;
    int64 Sequence = 0;
    FLHSaveError Error;
    // Each rejected generation remains available on disk; UI can show concrete details.
    FLHSaveError Slots[2];
};
DECLARE_MULTICAST_DELEGATE_OneParam(FLHSaveEvents, const FLHSaveEvent&);

struct LIGHTHAVEN_API FLHSaveProfile
{
    FLHCharacterId Character;
    bool bReadable = false;
    bool bRecovered = false;
    int64 Sequence = 0;
    FString DisplayName;
    FLHSaveError Error;
};

class LIGHTHAVEN_API FLHSaveStore : public TSharedFromThis<FLHSaveStore>
{
public:
    explicit FLHSaveStore(TSharedRef<ILHSaveStorage> InStorage) : Storage(InStorage) {}
    FLHSaveEvents Events;
    TArray<FLHSaveProfile> Enumerate(const FLHSaveCompatibility& Compatibility);
    bool Load(const FLHCharacterId& Character, const FLHSaveCompatibility& Compatibility,
        FLHSaveSnapshot& Out, FLHSaveError& Error);
    // Caller certifies no pending action, travel transition or publication stack. Snapshot is copied once.
    bool RequestSave(const FLHSaveSnapshot& Snapshot, const FLHSaveCompatibility& Compatibility,
        bool bCompletedActionBoundary, FLHSaveError& Error);
    bool Retry(const FLHCharacterId& Character, FLHSaveError& Error);
    bool IsDirty(const FLHCharacterId& Character) const;
    bool IsWriting(const FLHCharacterId& Character) const;
private:
    struct FState
    {
        bool bWriting = false;
        TSharedPtr<const FLHSaveSnapshot> Pending;
        FLHSaveCompatibility Compatibility;
        uint64 Serial = 0;
    };
    struct FPair
    {
        bool Valid[2] = {false,false};
        FLHSaveSnapshot Saves[2];
        FLHSaveError Errors[2];
        int32 Newest = INDEX_NONE;
    };
    TSharedRef<ILHSaveStorage> Storage;
    TMap<FGuid,FState> States;
    FPair ReadPair(const FLHCharacterId& Character, const FLHSaveCompatibility& Compatibility);
    bool Start(const FLHCharacterId& Character, FLHSaveError& Error);
    void Emit(ELHSaveEventKind Kind, const FLHCharacterId& Character, int64 Sequence, const FLHSaveError& Error, const FPair* Pair=nullptr);
};
