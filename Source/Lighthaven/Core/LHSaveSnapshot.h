// DRAFT schema rev 1 — NOT COMPILED (no Unreal Engine installed); integrator review pending
#pragma once

#include "CoreMinimal.h"
#include "LHDefinitions.h"
#include "LHSaveSnapshot.generated.h"

UENUM(BlueprintType)
enum class ELHEncounterLifeState : uint8
{
    Unresolved, Alive, Dead, RespawnPending, PermanentlyDefeated
};

UENUM(BlueprintType)
enum class ELHEffectSavePolicy : uint8
{
    Unresolved, CompletedActionBoundaryOnly
};

USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHQuestionAnswer
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Question;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Answer;
};

// Policy distinguishes question/roll creation from declared prototype allocation; never use current stats as accepted roll.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHCreationRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHQuestionAnswer> QuestionAnswers;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHAttributeBlock AcceptedAttributes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger GenerationRevision;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId GenerationPolicy;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHRngState> AcceptedRollInputs;
};

// Only permanent instance values; no equipment/GAS temporary totals.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHItemInstance
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Id;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Definition;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Quantity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHMechanicalField> PermanentRolledValues;
};

// Unique slot and inventory instance; empty binding omitted, quiver first-class.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHEquipmentBinding
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHEquipmentSlot Slot = ELHEquipmentSlot::Unspecified;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Item;
};

USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHLearnedSkill
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Skill;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger TrainedValue;
};

// Validated safe location, not an arbitrary serialized actor transform.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHCheckpoint
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntranceId Entrance;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHValueResolution TransformResolution = ELHValueResolution::Unresolved;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FTransform SafeTransform;
};

// No modifiers baked into base values. Stage 1 reserves rebirth count zero only.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHCharacterRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHContentId> AppearanceIds;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHCreationRecord Creation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHAttributeBlock BaseAttributes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger EarnedLevel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger ExperienceBalance;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger ExperienceDebt;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger UnspentAttributePoints;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger UnspentSkillPoints;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber EarnedBaseHealth;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber EarnedBaseMana;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber CurrentHealth;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber CurrentMana;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHLearnedSkill> LearnedSkills;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHContentId> LearnedSpells;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHItemInstance> Inventory;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHEquipmentBinding> Equipment;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Gold;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHGrowthAward> GrowthAwards;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntranceId ActiveEntrance;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) int32 RebirthCount = 0;
};

// Death, reward and finalized corpse loot are one snapshot transaction.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHEncounterRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHSpawnLifeId Life;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Definition;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHEncounterLifeState State = ELHEncounterLifeState::Unresolved;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber CurrentHealth;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber RespawnRemainingSeconds;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRewardId KillReward;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bRewardCommitted = false;
};

// Committed rolls preserved; remaining contents change only after atomic pickup.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHCorpseLootRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Container;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHSpawnLifeId SourceLife;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRewardId Reward;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHItemInstance> RemainingItems;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger RemainingGold;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bFinalized = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bClaimed = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber CleanupRemainingSeconds;
};

// Placed doors/chests/pickups, with identity independent of actor labels.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHObjectRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Id;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Definition;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bOpened = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bCollected = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bUnlocked = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHItemInstance> RemainingItems;
};

// Acceptance, completion and reward are separate; eligible kills independent of corpse cleanup.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHQuestRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Quest;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName Stage = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bAccepted = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bCompleted = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bRewarded = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger EligibleKillCount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHContentId> Flags;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRewardId TurnInClaim;
};

// Defeat, mark, dialogue and quest state must not collapse into one boolean.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHBossRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Boss;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bDefeated = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHContentId> Marks;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHContentId> DialogueFlags;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRewardId UniqueClaim;
};

USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHAreaRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHAreaId Area;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHEncounterRecord> Encounters;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHCorpseLootRecord> Corpses;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHObjectRecord> Objects;
};

// A coherent world for one local campaign; arrays serialized as unique keyed sets after validation.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHWorldRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FGuid RunId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHAreaRecord> Areas;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHQuestRecord> Quests;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHBossRecord> Bosses;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHRewardId> ClaimedUniqueRewards;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHEntityId> UnlockedPortals;
};

USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHCooldownRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Ability;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber RemainingSeconds;
};

// Approved durable effect inputs only; rebuild modifiers, never serialize GAS aggregates.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHDurableEffectRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Effect;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Source;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber RemainingSeconds;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Stacks;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHMechanicalField> CanonicalInputs;
};

// Successful completed requests only. Reject reused ID with different payload; bounded retention needs explicit policy.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHRequestReceipt
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString PayloadDigest;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) int64 TransactionSequence = 0;
};

// Save at completed action boundaries; timers stop while paused/closed, respawns also stop off-floor.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHSessionRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHRngState> GameplayRng;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHNumber ManaRegenFractionalSeconds;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHEffectSavePolicy EffectPolicy = ELHEffectSavePolicy::Unresolved;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHCooldownRecord> Cooldowns;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHDurableEffectRecord> DurableEffects;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHCheckpoint SafeRespawn;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHRequestReceipt> RecentRequests;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FString> Diagnostics;
};

// Envelope length/checksum are serializer outputs, not mechanics; no checksum algorithm silently selected.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHSaveHeader
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString Magic = TEXT("LHSave");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) int32 SchemaVersion = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) int64 TransactionSequence = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString BuildId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRulesetRef Ruleset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString ContentRevision;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHCharacterId CharacterId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) int64 PayloadLengthBytes = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName ChecksumAlgorithm = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString PayloadChecksum;
};

// Values only. This DTO is not a serializer or USaveGame implementation.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHSaveSnapshot
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHSaveHeader Header;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHCharacterRecord Character;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHWorldRecord World;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHSessionRecord Session;
};
