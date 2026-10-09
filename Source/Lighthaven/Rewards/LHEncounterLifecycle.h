#pragma once
#include "LHRewardTypes.h"
#include "Character/LHCharacterAuthority.h"
#include "World/LHAreaRegistry.h"
namespace LHRewards
{
    LIGHTHAVEN_API ELHCommandReason PopulateArea(FLHAreaRecord&, const FLHAreaDefinition&, TFunctionRef<FLHNumber(const FLHContentId&)> MaxHealth);
    LIGHTHAVEN_API FLHEntityId CorpseContainerFor(const FGuid& Run, const FLHSpawnLifeId&);
    LIGHTHAVEN_API ELHCommandReason SettleKill(FLHSaveSnapshot&, const FLHKillFacts&, const FLHKillRewardSpec&, const FLHCharacterProfile&, TArrayView<ILHKillObserver* const>);
    // Caller supplies elapsed loaded, unpaused simulation ONLY; resolves full HP from catalog before spawning.
    LIGHTHAVEN_API ELHCommandReason AdvanceRespawns(FLHAreaRecord&, double ActiveSeconds, TFunctionRef<bool(const FGuid&)> IsSpawnSafe, TArray<FLHSpawnLifeId>& NewLives);
    // Complete atomic variant for integration: restores max HP and pre-mints next life reward.
    LIGHTHAVEN_API ELHCommandReason AdvanceRespawns(FLHAreaRecord&, const FGuid& Run, double ActiveSeconds, TFunctionRef<bool(const FGuid&)> IsSpawnSafe, TFunctionRef<FLHNumber(const FLHContentId&)> MaxHealth, TArray<FLHSpawnLifeId>& NewLives);
    LIGHTHAVEN_API ELHCommandReason ExecuteTakeLoot(FLHSaveSnapshot&, const FLHCharacterProfile&, const FLHTakeLootRequest&);
    // Requires persisted HP==0; changes to positive HP are the idempotency boundary before avatar respawn.
    LIGHTHAVEN_API ELHCommandReason SettlePlayerDeath(FLHSaveSnapshot&, const FLHCharacterProfile&, const FLHCheckpoint& ReviewedRecovery, bool bRecoverySafetyReviewed);
    // In-flight container references must be excluded by caller; unique/quest item predicate prevents expiry.
    LIGHTHAVEN_API ELHCommandReason AdvanceLootCleanup(FLHAreaRecord&, double ActiveSeconds, TFunctionRef<bool(const FLHContentId&)> IsProtectedItem, TFunctionRef<bool(const FLHEntityId&)> IsReferenced);
}
