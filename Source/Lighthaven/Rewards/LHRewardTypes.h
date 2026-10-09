#pragma once
#include "Core/LHCommands.h"

enum class ELHRespawnPolicy : uint8 { OrdinaryRepeat, PermanentDefeat };
struct LIGHTHAVEN_API FLHKillRewardSpec
{
    FLHInteger Experience, GoldMin, GoldMax;
    FLHNumber ItemDropChance;
    TArray<FLHLootEntry> Loot;
    FLHNumber RespawnSeconds, SafetyDistanceCm;
    ELHRespawnPolicy RespawnPolicy = ELHRespawnPolicy::OrdinaryRepeat;
    bool bBoss = false;
};
struct LIGHTHAVEN_API FLHKillFacts
{
    FLHAreaId Area;
    FLHSpawnLifeId Life;
    FLHContentId Enemy;
    bool bKillerIsPlayer = false;
};
// Observers modify only InOut, never publish external effects. Failure rolls back the entire kill.
class LIGHTHAVEN_API ILHKillObserver
{
public:
    virtual ~ILHKillObserver() = default;
    virtual ELHCommandReason OnSettledKill(const FLHKillFacts&, FLHSaveSnapshot& InOut) = 0;
};
