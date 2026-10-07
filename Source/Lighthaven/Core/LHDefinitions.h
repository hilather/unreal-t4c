// DRAFT schema rev 1 — NOT COMPILED (no Unreal Engine installed); integrator review pending
#pragma once

#include "CoreMinimal.h"
#include "LHValues.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "LHDefinitions.generated.h"

class UWorld;

UENUM(BlueprintType)
enum class ELHEquipmentSlot : uint8
{
    Unspecified, MainHand, OffHand, Quiver, Head, Torso, Legs, Feet, Accessory
};

// Shell for unresolved formula parameters; named units and semantics must be frozen before consumption.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHMechanicalField
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName Key = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber Value;
};

// Named policy choice, not an executable expression or second rules engine.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHPolicyField
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName Key = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHValueResolution Resolution = ELHValueResolution::Unresolved;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName Value = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHFieldProvenance Provenance;
};

// Absent constraint is an explicit policy; unresolved minima cannot silently mean unrestricted.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHEligibility
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHAttributeBlock MinimumAttributes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger MinimumLevel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHContentId> RequiredSkills;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHContentId> RequiredSpells;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHMechanicalField> SkillMinimums;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHPolicyField> Policies;
};

USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHLootEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Item;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger MinimumQuantity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger MaximumQuantity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber Weight;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHFieldProvenance Provenance;
};

// Untraced transform is unresolved, never a spawn at world origin.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHEntranceDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntranceId Id;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHValueResolution TransformResolution = ELHValueResolution::Unresolved;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FTransform SafeTransform;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHFieldProvenance Provenance;
};

// Destination must resolve; reverse portal separately authored.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHPortalDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Portal;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntranceId Source;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntranceId Destination;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHFieldProvenance Provenance;
};

UCLASS(Abstract, BlueprintType)
class LIGHTHAVEN_API ULHDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHContentId Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHFieldProvenance> FieldProvenance;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UObject> Visual;
    virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId(PrimaryType(), Id.Value); }
protected:
    virtual FName PrimaryType() const PURE_VIRTUAL(ULHDefinition::PrimaryType, return NAME_None;);
};

UCLASS(BlueprintType)
class LIGHTHAVEN_API ULHRulesetDefinition : public ULHDefinition
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHRulesetRef Reference;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHMechanicalField> Parameters;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHPolicyField> Policies;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHInteger> ExperienceThresholds;
protected:
    virtual FName PrimaryType() const override { return FName(TEXT("Ruleset")); }
};

UCLASS(BlueprintType)
class LIGHTHAVEN_API ULHItemDefinition : public ULHDefinition
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag Category;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHEligibility Eligibility;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ELHEquipmentSlot Slot = ELHEquipmentSlot::Unspecified;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHInteger StackLimit;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHNumber Weight;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHInteger BuyGold;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHInteger SellGold;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHMechanicalField> PermanentModifiers;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHPolicyField> Policies;
protected:
    virtual FName PrimaryType() const override { return FName(TEXT("Item")); }
};

UCLASS(BlueprintType)
class LIGHTHAVEN_API ULHAbilityDefinition : public ULHDefinition
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHEligibility Eligibility;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftClassPtr<UObject> ExecutionClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHInteger LearningSkillPoints;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHInteger LearningGold;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHNumber ManaCost;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHNumber RangeCm;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHNumber CooldownSeconds;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHMechanicalField> Effects;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHPolicyField> Policies;
protected:
    virtual FName PrimaryType() const override { return FName(TEXT("Ability")); }
};

UCLASS(BlueprintType)
class LIGHTHAVEN_API ULHEnemyDefinition : public ULHDefinition
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHAttributeBlock Attributes;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHNumber Health;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHInteger ExperienceReward;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHInteger GoldReward;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHContentId> Attacks;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ELHValueResolution LootResolution = ELHValueResolution::Unresolved;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHLootEntry> Loot;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHMechanicalField> AIParameters;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHPolicyField> Policies;
protected:
    virtual FName PrimaryType() const override { return FName(TEXT("Enemy")); }
};

UCLASS(BlueprintType)
class LIGHTHAVEN_API ULHEncounterDefinition : public ULHDefinition
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHAreaId Area;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGuid SpawnSlot;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHContentId Enemy;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ELHValueResolution PlacementResolution = ELHValueResolution::Unresolved;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FTransform Anchor;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHNumber RespawnSeconds;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHNumber SafetyDistanceCm;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bBoss = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bElite = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHPolicyField> Policies;
protected:
    virtual FName PrimaryType() const override { return FName(TEXT("Encounter")); }
};

UCLASS(BlueprintType)
class LIGHTHAVEN_API ULHAreaDefinition : public ULHDefinition
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UWorld> Map;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHEntranceDefinition> Entrances;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHPortalDefinition> Portals;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLHEntranceId SafeFallback;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FLHContentId> Encounters;
protected:
    virtual FName PrimaryType() const override { return FName(TEXT("Area")); }
};
