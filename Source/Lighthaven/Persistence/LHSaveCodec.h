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
