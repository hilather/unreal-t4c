#pragma once

#include "Core/LHSaveSnapshot.h"

// Stable reasons for presentation adapters; persistence never substitutes a new character.
enum class ELHSaveReason : uint8
{
    None, NotFound, Busy, IoFailure, Oversize, Malformed, BadChecksum,
    UnknownAlgorithm, UnknownCodec, FutureSchema, UnsupportedSchema,
    IncompatibleRuleset, IncompatibleContent, InvalidSnapshot, EpochMismatch,
    SequenceExhausted, BoundaryRequired
};

struct LIGHTHAVEN_API FLHSaveError
{
    ELHSaveReason Reason = ELHSaveReason::None;
    FString Detail;
};

struct LIGHTHAVEN_API FLHSaveCompatibility
{
    FLHRulesetRef Ruleset;
    FString ContentRevision;
    int32 MaxGrowthAwards = 4096; // Set to the selected ruleset level-transition count (bounded by D05).
    TFunction<bool(const FLHSaveSnapshot&, FLHSaveError&)> ValidateReferences;
};

struct LIGHTHAVEN_API FLHSaveDecodeStats
{
    bool bPayloadAllocationStarted = false;
};

namespace LHSave
{
    constexpr int32 MaxFileBytes = 16 * 1024 * 1024;
    constexpr int32 MaxHeaderBytes = 64 * 1024;
    constexpr int32 MaxPayloadBytes = 15 * 1024 * 1024;
    // Explicit v1 identity migration dispatch; unsupported/ambiguous versions never reinterpret.
    LIGHTHAVEN_API bool SupportsVersion(int32 Version);
    LIGHTHAVEN_API FString Sha256(TConstArrayView<uint8> Bytes);
    LIGHTHAVEN_API bool Validate(const FLHSaveSnapshot& Snapshot, FLHSaveError& Error);
    LIGHTHAVEN_API bool Encode(const FLHSaveSnapshot& Snapshot, TArray<uint8>& Bytes, FLHSaveError& Error);
    LIGHTHAVEN_API bool Decode(TConstArrayView<uint8> Bytes, const FLHCharacterId& Character,
        const FLHSaveCompatibility& Compatibility, FLHSaveSnapshot& Snapshot, FLHSaveError& Error, FLHSaveDecodeStats* Stats=nullptr);
}

// Command type metadata is used only for exact allowlist dispatch, never reflection serialization.
namespace LHSave
{
    LIGHTHAVEN_API bool EncodeCanonicalRequest(FName Command, const UScriptStruct* Type, const void* Request,
        TArray<uint8>& Bytes, FLHSaveError& Error);
    LIGHTHAVEN_API bool CanonicalRequestDigest(FName Command, const UScriptStruct* Type, const void* Request,
        FString& Digest, FLHSaveError& Error);
    // Directly compatible with FLHCharacterProfile::RequestDigest; empty on rejection.
    LIGHTHAVEN_API FString RequestDigest(FName Command, const UScriptStruct* Type, const void* Request);
    LIGHTHAVEN_API bool ValidateCanonicalRequestBytes(FName Command, TConstArrayView<uint8> Bytes, FLHSaveError& Error);
    LIGHTHAVEN_API bool GrowthRewardId(const FGuid& Run, const FLHCharacterId& Character, int64 ToLevel,
        FLHRewardId& Reward, FLHSaveError& Error);
    LIGHTHAVEN_API bool EnemyLifeRewardId(const FGuid& Run, const FLHSpawnLifeId& Life,
        FLHRewardId& Reward, FLHSaveError& Error);
    // Directly compatible with FLHCharacterProfile::GrowthId; invalid on rejection.
    LIGHTHAVEN_API FLHRewardId GrowthId(const FGuid& Run, const FLHCharacterId& Character, int64 ToLevel);
    // Mapping only. Authority must reject an existing ID associated with a different source tuple.
    LIGHTHAVEN_API bool RewardIdFromDigest(const FString& Digest, FLHRewardId& Reward, FLHSaveError& Error);
}
