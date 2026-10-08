#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Core/LHCommands.h"
#include "Rules/LHRules.h"
#include "LHCombatComponent.generated.h"
class ULHAttributeSet;

// Runtime adapter inputs must come from validated definitions, never UI supplied numbers.
struct LIGHTHAVEN_API FLHBasicAttackConfig
{
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

UCLASS()
class LIGHTHAVEN_API ULHCombatComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()
public:
    ULHCombatComponent();
    // Export GAS forwarding calls through Lighthaven for consumers in other modules.
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
    FLHHitIdentity GetPendingIdentity() const { return PendingIdentity; }
    float GetImpactDelay() const { return static_cast<float>(Attack.ImpactSeconds.Value); }
    ULHAttributeSet* GetCombatAttributes() const { return Attributes; }
    double GetRemainingCooldown() const;
    bool RestoreRemainingCooldown(double Seconds);
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
    double CooldownEnd = 0;
    uint64 LifeRevision = 0, PendingTargetLifeRevision = 0;
    TWeakObjectPtr<ULHCombatComponent> PendingTarget;
    FLHHitIdentity PendingIdentity;
    FGameplayAbilitySpecHandle AttackHandle;
    // Scope is one target life; clear only via explicit restore/initialization after cancelling actions.
    TSet<FLHHitIdentity> AppliedHits;
};
