// DRAFT schema rev 1 — NOT COMPILED (no Unreal Engine installed); integrator review pending
#pragma once

#include "CoreMinimal.h"
#include "LHIdentity.h"
#include "LHValues.generated.h"

UENUM(BlueprintType)
enum class ELHProvenanceStatus : uint8
{
    Missing, Confirmed, VerifiedT4C, Disputed, Modernized, Prototype
};

UENUM(BlueprintType)
enum class ELHValueResolution : uint8
{
    Unresolved, Resolved
};

UENUM(BlueprintType)
enum class ELHMigrationPolicy : uint8
{
    Reject, RequireExplicitMigration, NewCharacterRequired
};

// Source URL empty for authored tuning; notes identify designer choice. Date is ISO date; record alternatives separately.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHFieldProvenance
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName FieldPath = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString SourceUrl;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString SourceBaseline;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString RetrievedDate;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHProvenanceStatus Status = ELHProvenanceStatus::Missing;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString ValueAsRecorded;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString Notes;
};

// Value is storage only when unresolved. Consumers must check Resolution; zero is never an implicit resolved value.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHInteger
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHValueResolution Resolution = ELHValueResolution::Unresolved;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) int64 Value = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHFieldProvenance Provenance;
};

// Value is storage only when unresolved. Consumers must check Resolution; zero is never an implicit resolved value.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHNumber
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHValueResolution Resolution = ELHValueResolution::Unresolved;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) double Value = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHFieldProvenance Provenance;
};

// Base or accepted creation values only; no positional arrays, no GAS aggregates.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHAttributeBlock
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Strength;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Endurance;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Agility;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Intelligence;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Wisdom;
};

// Revision zero invalid. Hash algorithm and canonicalization frozen by integrator.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHRulesetRef
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Id;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) int32 Revision = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString ContentHash;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHMigrationPolicy MigrationPolicy = ELHMigrationPolicy::Reject;
};

// Opaque value bytes, bounded by loader; independent gameplay streams, no cosmetic RNG.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHRngState
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName StreamId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName Algorithm = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) int32 AlgorithmRevision = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<uint8> State;
};

// One committed award per newly earned level; recovering debt does not append awards.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHGrowthAward
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRewardId AwardId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger FromLevel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger ToLevel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHAttributeBlock GrowthInputs;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber HealthIncrement;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber ManaIncrement;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger AttributePoints;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger SkillPoints;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRulesetRef Ruleset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHRngState> RollInputs;
};

