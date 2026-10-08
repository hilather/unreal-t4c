#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Abilities/LHBasicAttackAbility.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include <limits>
namespace
{
bool Ready(const FLHNumber& N)
{
    return N.Resolution == ELHValueResolution::Resolved && N.Provenance.Status != ELHProvenanceStatus::Missing &&
        N.Provenance.Status != ELHProvenanceStatus::Disputed && FMath::IsFinite(N.Value) && N.Value >= 0 &&
        N.Value <= std::numeric_limits<float>::max();
}
bool LiveActor(const AActor* Actor) { return IsValid(Actor) && !Actor->IsActorBeingDestroyed(); }
}
ULHCombatComponent::ULHCombatComponent()
{
    SetIsReplicatedByDefault(false);
    Attributes = CreateDefaultSubobject<ULHAttributeSet>(TEXT("CombatAttributes"));
    AddAttributeSetSubobject(Attributes.Get());
}
void ULHCombatComponent::InitializeCombatActorInfo(AActor* Owner, AActor* Avatar)
{
    CancelAllAbilities();
    FinishAttack();
    ++LifeRevision;
    InitAbilityActorInfo(Owner, Avatar);
    bDeathPublished = !IsAlive();
    AppliedHits.Reset();
    if (!AttackHandle.IsValid()) AttackHandle = GiveAbility(FGameplayAbilitySpec(ULHBasicAttackAbility::StaticClass(), 1));
}
void ULHCombatComponent::ClearCombatAvatar()
{
    CancelAllAbilities();
    FinishAttack();
    ClearActorInfo();
}
void ULHCombatComponent::ConfigureAttack(const FLHBasicAttackConfig& Config, const LH::Rules::FRequirementInput& Requirements)
{
    if (bPending) return;
    Attack = Config;
    RequirementInput = Requirements;
}
bool ULHCombatComponent::IsAlive() const
{
    return LiveActor(GetAvatarActor()) && FMath::IsFinite(Attributes->GetHealth()) && Attributes->GetHealth() > 0;
}
bool ULHCombatComponent::InRangeAndSight(const ULHCombatComponent* Target) const
{
    if (!Target || !LiveActor(GetAvatarActor()) || !LiveActor(Target->GetAvatarActor()) || GetAvatarActor() == Target->GetAvatarActor()) return false;
    UWorld* World = GetAvatarActor()->GetWorld();
    if (!World || World != Target->GetAvatarActor()->GetWorld()) return false;
    const FVector Start = GetAvatarActor()->GetActorLocation(), End = Target->GetAvatarActor()->GetActorLocation();
    const double Distance = FVector::Dist(Start, End);
    if (!FMath::IsFinite(Distance) || Distance > Attack.RangeCm.Value) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(LHAttackSight), false, GetAvatarActor());
    Params.AddIgnoredActor(Target->GetAvatarActor());
    return !World->LineTraceTestByChannel(Start, End, ECC_Visibility, Params);
}
ELHCommandReason ULHCombatComponent::ValidateAttack(ULHCombatComponent* Target) const
{
    if (!IsValid(Target) || !LiveActor(Target->GetAvatarActor())) return ELHCommandReason::NotFound;
    if (!IsAlive() || !Target->IsAlive()) return ELHCommandReason::InvalidLifeState;
    if (bPending) return ELHCommandReason::ActiveAction;
    if (GetRemainingCooldown() > 0) return ELHCommandReason::Cooldown;
    if (!bRandomReady || !Ready(Attack.ManaCost) || !Ready(Attack.CooldownSeconds) || !Ready(Attack.ImpactSeconds) ||
        !Ready(Attack.RangeCm) || !Ready(Attack.WeaponMinimum) || !Ready(Attack.WeaponMaximum)) return ELHCommandReason::UnresolvedRules;
    const auto Eligibility = LH::Rules::CheckRequirements(Attack.RequirementPolicy, Attack.Eligibility, RequirementInput);
    if (!Eligibility.Diagnostic.IsAccepted()) return Eligibility.Diagnostic.Reason == LH::Rules::EReason::Ineligible ? ELHCommandReason::Ineligible : ELHCommandReason::UnresolvedRules;
    // Validate formula parameters without consuming the combat random stream or mutating state.
    LH::Rules::FCombatInput Input;
    Input.WeaponMinimum = Attack.WeaponMinimum.Value; Input.WeaponMaximum = Attack.WeaponMaximum.Value;
    Input.Accuracy = Attributes->GetAccuracy(); Input.DamageBonus = Attributes->GetDamageBonus();
    Input.Avoidance = Target->Attributes->GetAvoidance(); Input.Armor = Target->Attributes->GetArmor(); Input.Resistance = Target->Attributes->GetResistance();
    if (!LH::Rules::ResolveCombat(Attack.Combat, Input).Diagnostic.IsAccepted()) return ELHCommandReason::UnresolvedRules;
    if (!FMath::IsFinite(Attributes->GetMana()) || Attributes->GetMana() < Attack.ManaCost.Value) return ELHCommandReason::InsufficientMana;
    if (!InRangeAndSight(Target)) return ELHCommandReason::OutOfRange;
    return ELHCommandReason::None;
}
ELHCommandReason ULHCombatComponent::RequestBasicAttack(ULHCombatComponent* Target)
{
    if (!GetWorld() || GetWorld()->GetNetMode() != NM_Standalone) return ELHCommandReason::InvalidRequest;
    const auto Reason = ValidateAttack(Target);
    if (Reason != ELHCommandReason::None) return Reason;
    if (!AttackHandle.IsValid()) return ELHCommandReason::InvalidRequest;
    PendingTarget = Target;
    PendingTargetLifeRevision = Target->LifeRevision;
    PendingIdentity = {FGuid::NewGuid(), 0};
    // GAS activates synchronously on the local game thread; CommitAttack establishes pending state.
    if (!TryActivateAbility(AttackHandle, false)) { FinishAttack(); return ELHCommandReason::Busy; }
    return ELHCommandReason::None;
}
ELHCommandReason ULHCombatComponent::CommitAttack()
{
    const auto Reason = ValidateAttack(PendingTarget.Get());
    if (Reason != ELHCommandReason::None || !PendingIdentity.ActivationId.IsValid()) return Reason == ELHCommandReason::None ? ELHCommandReason::InvalidRequest : Reason;
    bPending = true; bCommitted = true; bImpactConsumed = false;
    CooldownEnd = GetWorld()->GetTimeSeconds() + Attack.CooldownSeconds.Value;
    SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(), Attributes->GetMana() - static_cast<float>(Attack.ManaCost.Value));
    return ELHCommandReason::None;
}
bool ULHCombatComponent::ResolveImpact(const FLHHitIdentity& Id)
{
    if (!bPending || !bCommitted || bImpactConsumed || !(Id == PendingIdentity)) return false;
    bImpactConsumed = true; // Consume even failed/missed callbacks, before any event can reenter.
    auto* Target = PendingTarget.Get();
    if (!IsAlive() || !IsValid(Target) || !Target->IsAlive() || Target->LifeRevision != PendingTargetLifeRevision || !InRangeAndSight(Target)) return false;
    LH::Rules::FCombatInput Input;
    Input.Accuracy = Attributes->GetAccuracy(); Input.Avoidance = Target->Attributes->GetAvoidance();
    Input.DamageBonus = Attributes->GetDamageBonus(); Input.Armor = Target->Attributes->GetArmor(); Input.Resistance = Target->Attributes->GetResistance();
    Input.WeaponMinimum = Attack.WeaponMinimum.Value; Input.WeaponMaximum = Attack.WeaponMaximum.Value;
    Input.HitRoll = CombatRandom.GetFraction(); Input.DamageRoll = CombatRandom.GetFraction();
    const auto Result = LH::Rules::ResolveCombat(Attack.Combat, Input);
    if (!Result.Diagnostic.IsAccepted()) return false;
    if (Result.Value.bHit && !Target->ApplyResolvedDamage(Id, Result.Value.Damage)) return false;
    OnImpact.Broadcast(Id, Result.Value);
    return true;
}
bool ULHCombatComponent::ApplyResolvedDamage(const FLHHitIdentity& Id, double Damage)
{
    if (!IsAlive() || AppliedHits.Contains(Id) || !FMath::IsFinite(Damage) || Damage < 0) return false;
    AppliedHits.Add(Id);
    const float Health = static_cast<float>(FMath::Max(0.0, static_cast<double>(Attributes->GetHealth()) - Damage));
    const bool bDied = Health <= 0 && !bDeathPublished;
    if (bDied) bDeathPublished = true; // Set before attribute delegates or lifecycle listeners run.
    SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), Health);
    if (bDied)
    {
        CancelAllAbilities();
        FinishAttack();
        OnDeath.Broadcast(Id);
    }
    return true;
}
void ULHCombatComponent::FinishAttack()
{
    bPending = false; bCommitted = false; bImpactConsumed = false;
    PendingTarget.Reset(); PendingIdentity = {};
}
double ULHCombatComponent::GetRemainingCooldown() const
{
    return GetWorld() ? FMath::Max(0.0, CooldownEnd - GetWorld()->GetTimeSeconds()) : 0;
}
bool ULHCombatComponent::RestoreRemainingCooldown(double Seconds)
{
    if (bPending || !GetWorld() || !FMath::IsFinite(Seconds) || Seconds < 0) return false;
    CooldownEnd = GetWorld()->GetTimeSeconds() + Seconds;
    return true;
}
