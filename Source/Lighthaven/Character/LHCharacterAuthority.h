#pragma once
#include "Core/LHCommands.h"
#include "Rules/LHRules.h"
#include "Components/ActorComponent.h"
#include "LHCharacterAuthority.generated.h"

// Fully registered, complete mechanical values. No inferred zero modifiers or requirements.
struct LIGHTHAVEN_API FLHCharacterItemDefinition
{
    FLHContentId Id;
    ELHEquipmentSlot Slot = ELHEquipmentSlot::Unspecified;
    FLHEligibility Eligibility;
    LH::Rules::FModifier Modifier;
    FLHInteger StackLimit;
    FLHNumber Weight;
    bool bBow = false;
    TArray<FLHContentId> CompatibleQuivers;
};
struct LIGHTHAVEN_API FLHCharacterProfile
{
    LH::Rules::FRuleset Rules;
    FLHRulesetRef Reference;
    FLHContentId CreationPolicy;
    FLHInteger CreationRevision;
    FLHInteger InitialSkillPoints, InitialGold, InventorySlots;
    FLHNumber InitialHealth, InitialMana;
    LH::Rules::EAttributeBasis GrowthBasis = LH::Rules::EAttributeBasis::Unresolved;
    TArray<FLHCharacterItemDefinition> Items;
    TArray<FLHContentId> StarterItems; // Explicitly authored empty list means no free equipment.
    // W2-01 supplies D04 canonical request digests and D06 reward IDs. No local alternate codec.
    TFunction<FString(FName, const UScriptStruct *, const void *)> RequestDigest;
    TFunction<FLHRewardId(const FGuid &, const FLHCharacterId &, int64)> GrowthId;
};
struct LIGHTHAVEN_API FLHCharacterPreview
{
    FGuid Token;
    FLHCreationRecord Creation;
    int64 UnspentPoints = 0;
};

// Trusted local authority API. UI receives copies; there are no mutable record setters.
class LIGHTHAVEN_API FLHCharacterAuthority
{
  public:
    bool Initialize(const FLHCharacterProfile &Profile, const FGuid &BootstrapEpoch, int32 Seed);
    ELHCommandReason Preview(const TArray<FLHQuestionAnswer> &Answers, FLHCharacterPreview &Out);
    FLHCommandResult Execute(const FLHCreateCharacterRequest &Request);
    FLHCommandResult Execute(const FLHAllocateAttributePointsRequest &Request);
    FLHCommandResult Execute(const FLHEquipItemRequest &Request);
    ELHCommandReason GrantExperience(int64 Gain);
    ELHCommandReason AddItem(const FLHItemInstance &Item);
    ELHCommandReason RemoveItem(const FLHEntityId &Item, int64 Quantity);
    ELHCommandReason AddExperienceDebt(int64 Amount);
    LH::Rules::TResult<LH::Rules::FStatsResult> Stats() const;
    const FLHCharacterRecord &Record() const
    {
        return State.Character;
    }
    void Export(FLHSaveSnapshot &Snapshot) const;
    ELHCommandReason Import(const FLHSaveSnapshot &Snapshot);

  private:
    FLHCharacterProfile Profile;
    FLHSaveSnapshot State;
    FLHCharacterPreview Pending;
    FLHRngState PendingRng;
    int32 Seed = 0;
    bool bInitialized = false;
    const FLHCharacterItemDefinition *Definition(FName Id) const;
    ELHCommandReason Validate(const FLHSaveSnapshot &Candidate) const;
    LH::Rules::TResult<LH::Rules::FStatsResult> Stats(const FLHCharacterRecord &Record) const;
    bool Begin(const FLHRequestId &, FName, const UScriptStruct *, const void *, FLHCommandResult &, FString &) const;
    FLHCommandResult Commit(const FLHRequestId &, const FString &, FLHSaveSnapshot &&);
};

// Integrator attaches once to PlayerState; authority survives avatar death/travel.
UCLASS()
class LIGHTHAVEN_API ULHCharacterAuthorityComponent : public UActorComponent
{
    GENERATED_BODY()
  public:
    static ULHCharacterAuthorityComponent *Attach(class APlayerState *Owner);
    FLHCharacterAuthority &Authority()
    {
        return Service;
    }

  private:
    FLHCharacterAuthority Service;
};
