// DRAFT schema rev 1 — NOT COMPILED (no Unreal Engine installed); integrator review pending
#pragma once

#include "CoreMinimal.h"
#include "LHSaveSnapshot.h"
#include "LHCommands.generated.h"

UENUM(BlueprintType)
enum class ELHCommandDisposition : uint8
{
    Rejected, Accepted
};

UENUM(BlueprintType)
enum class ELHCommandReason : uint8
{
    None, InvalidRequest, ReusedRequestId, UnresolvedRules, InvalidLifeState, NotFound, OutOfRange, Obstructed, Ineligible, InsufficientPoints, InsufficientGold, InsufficientMana, InventoryFull, InvalidEquipment, Cooldown, ActiveAction, InvalidDestination, SaveRequired, Busy
};

// Rejected means no partial mutation; accepted replay returns original outcome/sequence.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHCommandResult
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHCommandDisposition Disposition = ELHCommandDisposition::Rejected;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHCommandReason Reason = ELHCommandReason::InvalidRequest;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) int64 CommittedSequence = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bReplay = false;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHCreateCharacterRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FString DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) TArray<FLHContentId> AppearanceIds;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHCreationRecord Creation;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHAllocateAttributePointsRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHAttributeBlock Points;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHTrainSkillRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Trainer;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Skill;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Points;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHLearnSpellRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Trainer;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Spell;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHBuyItemRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Vendor;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Offer;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Quantity;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHSellItemRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Vendor;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Item;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Quantity;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHEquipItemRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Item;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) ELHEquipmentSlot Slot = ELHEquipmentSlot::Unspecified;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool bUnequip = false;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHUseAbilityRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Ability;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Target;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHInteractRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Target;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHContentId Topic;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHTakeLootRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Container;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Item;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHInteger Quantity;
};

// Owner resolved from interface context, never from caller identity. Inputs are intents, not trusted state.
USTRUCT(BlueprintType)
struct LIGHTHAVEN_API FLHRequestTravelRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHRequestId Request;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntityId Portal;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FLHEntranceId Destination;
};

// Native synchronous game-thread seam; one implementation owned by the local authority.
// No bus, transport, actor lookup, mutation implementation or Blueprint bypass is supplied here.
class LIGHTHAVEN_API ILHCommandHandler
{
public:
    virtual ~ILHCommandHandler() = default;
    virtual FLHCommandResult Execute(const FLHCreateCharacterRequest& Request) = 0;
    virtual FLHCommandResult Execute(const FLHAllocateAttributePointsRequest& Request) = 0;
    virtual FLHCommandResult Execute(const FLHTrainSkillRequest& Request) = 0;
    virtual FLHCommandResult Execute(const FLHLearnSpellRequest& Request) = 0;
    virtual FLHCommandResult Execute(const FLHBuyItemRequest& Request) = 0;
    virtual FLHCommandResult Execute(const FLHSellItemRequest& Request) = 0;
    virtual FLHCommandResult Execute(const FLHEquipItemRequest& Request) = 0;
    virtual FLHCommandResult Execute(const FLHUseAbilityRequest& Request) = 0;
    virtual FLHCommandResult Execute(const FLHInteractRequest& Request) = 0;
    virtual FLHCommandResult Execute(const FLHTakeLootRequest& Request) = 0;
    virtual FLHCommandResult Execute(const FLHRequestTravelRequest& Request) = 0;
};
