#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Core/LHCommands.h"
#include "Rules/LHRules.h"
#include "LHCombatComponent.generated.h"
class ULHAttributeSet;

// Runtime adapter inputs must come from validated definitions, never UI supplied numbers.
struct LIGHTHAVEN_API FLHBasicAttackConfig
{
    FLHContentId Ability{FName(TEXT("Attack.Melee.Basic"))};
    bool bSpell=false, bHeal=false, bLight=false, bBow=false, bUseLearnedSkills=false;
    double QuiverBonus=0, LightDuration=0;
    LH::Rules::FCombatParameters Combat;
    LH::Rules::FRequirementPolicy RequirementPolicy;
    FLHEligibility Eligibility;
    FLHNumber ManaCost, CooldownSeconds, ImpactSeconds, RangeCm, WeaponMinimum, WeaponMaximum;
};
struct LIGHTHAVEN_API FLHHitIdentity
{
    FGuid ActivationId;
    int32 ImpactIndex = 0;
    bool operator==(const FLHHitIdentity& Other) const { return ActivationId == Other.ActivationId && ImpactIndex == Other.ImpactIndex; }
    friend uint32 GetTypeHash(const FLHHitIdentity& Id) { return HashCombine(GetTypeHash(Id.ActivationId), GetTypeHash(Id.ImpactIndex)); }
};
DECLARE_MULTICAST_DELEGATE_OneParam(FLHDeathEvent, const FLHHitIdentity&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FLHImpactEvent, const FLHHitIdentity&, const LH::Rules::FCombatResult&);

enum class ELHAttackCancelReason : uint8 { Explicit, SourceDeath, AvatarCleared, Travel };
enum class ELHAttackOutcome : uint8 { ResolvedHit, ResolvedMiss, ImpactInvalidated, Cancelled };
struct FLHAttackEvent
{
    FLHHitIdentity Identity;
    FLHEntityId Source, Target;
    FLHContentId Ability;
    uint64 SourceLife=0, TargetLife=0;
    double CommitSimulationTime=0, ImpactSeconds=0;
};
DECLARE_MULTICAST_DELEGATE_OneParam(FLHAttackCommittedEvent, const FLHAttackEvent&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FLHAttackCancelledEvent, const FLHAttackEvent&, ELHAttackCancelReason);
DECLARE_MULTICAST_DELEGATE_TwoParams(FLHAttackFinishedEvent, const FLHAttackEvent&, ELHAttackOutcome);
UCLASS()
class LIGHTHAVEN_API ULHCombatComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()
public:
    ULHCombatComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void ConfigureManaRegen(const LH::Rules::FManaParameters& Parameters) { ManaParameters=Parameters; bManaTickerEnabled=true; }
    void SetRecoveryMenuPaused(bool Paused) { bRecoveryMenuPaused=Paused; }
    // Export GAS forwarding calls through Lighthaven for consumers in other modules.
    FOnGameplayAttributeValueChange& GetGameplayAttributeValueChangeDelegate(FGameplayAttribute Attribute);
    void SetNumericAttributeBase(const FGameplayAttribute& Attribute, float NewBaseValue);
    void CancelAllAbilities(UGameplayAbility* Ignore = nullptr);
    // Possession/restore must call after canonical resources and derived values are installed.
    void InitializeCombatActorInfo(AActor* Owner, AActor* Avatar);
    void ClearCombatAvatar();
    void ConfigureAttack(const FLHBasicAttackConfig& Config, const LH::Rules::FRequirementInput& Requirements);
    void SetCombatRandomState(const FRandomStream& State) { CombatRandom = State; bRandomReady = true; }
    FRandomStream GetCombatRandomState() const { return CombatRandom; }
    ELHCommandReason RequestBasicAttack(ULHCombatComponent* Target);
    ELHCommandReason ValidateAttack(ULHCombatComponent* Target) const;
    // Only the active GAS ability commits/resolves; identity is checked against this component's pending action.
    ELHCommandReason CommitAttack();
    bool ResolveImpact(const FLHHitIdentity& Id);
    void FinishAttack();
    bool IsAlive() const;
    bool IsActionPending() const { return bPending; }
    bool IsPublishingActionEvents() const { return PublicationDepth>0; }
    FLHHitIdentity GetPendingIdentity() const { return PendingIdentity; }
    float GetImpactDelay() const { return static_cast<float>(Attack.ImpactSeconds.Value); }
    ULHAttributeSet* GetCombatAttributes() const { return Attributes; }
    double GetRemainingCooldown() const;
    bool RestoreRemainingCooldown(double Seconds);
    void SetStableEntity(const FLHEntityId& Id) { StableEntity=Id; }
    void CancelAttack(ELHAttackCancelReason Reason);
    double GetRemainingCooldown(const FLHContentId& Ability) const;
    bool RestoreCooldownMap(const TMap<FName,double>& Remaining);
    TMap<FName,double> GetCooldownMap() const;
    bool AdvanceManaRegen(double ActiveSeconds, bool bPaused, const LH::Rules::FManaParameters& Parameters);
    double ManaRegenFractionalSeconds=0;
    double GetLightRemainingSeconds() const;
    bool RestoreLightRemainingSeconds(double Seconds);
    // Session is the sole clock owner; never also advanced by TickComponent.
    bool AdvanceLightEffect(double ActiveSeconds, bool bGamePaused, bool bAIPaused, bool bTravel);
    FLHAttackCommittedEvent OnAttackCommitted;
    FLHAttackCancelledEvent OnAttackCancelled;
    FLHAttackFinishedEvent OnAttackFinished;
    FLHDeathEvent OnDeath;
    FLHImpactEvent OnImpact;
private:
    bool InRangeAndSight(const ULHCombatComponent* Target) const;
    bool ApplyResolvedDamage(const FLHHitIdentity& Id, double Damage);
    UPROPERTY() TObjectPtr<ULHAttributeSet> Attributes;
    FLHBasicAttackConfig Attack;
    LH::Rules::FRequirementInput RequirementInput;
    FRandomStream CombatRandom;
    bool bRandomReady = false, bPending = false, bCommitted = false, bImpactConsumed = false, bDeathPublished = false;
    LH::Rules::FManaParameters ManaParameters;
    bool bManaTickerEnabled=false, bRecoveryMenuPaused=false;
    TMap<FName,double> CooldownEnds;
    double LightRemaining=0;
    FLHEntityId StableEntity;
    FLHAttackEvent PendingEvent;
    ELHAttackOutcome PendingOutcome=ELHAttackOutcome::ImpactInvalidated;
    ELHAttackCancelReason CancelReason=ELHAttackCancelReason::Explicit;
    bool bPublishingCommit=false, bDeferredFinish=false, bLifecycleTransition=false;
    int32 PublicationDepth=0;
    uint64 LifeRevision = 0, PendingTargetLifeRevision = 0;
    TWeakObjectPtr<ULHCombatComponent> PendingTarget;
    FLHHitIdentity PendingIdentity;
    FGameplayAbilitySpecHandle AttackHandle;
    // Scope is one target life; clear only via explicit restore/initialization after cancelling actions.
    TSet<FLHHitIdentity> AppliedHits;
};
