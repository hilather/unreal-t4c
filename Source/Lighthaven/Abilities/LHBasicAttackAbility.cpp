#include "Abilities/LHBasicAttackAbility.h"
#include "Abilities/LHCombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
ULHBasicAttackAbility::ULHBasicAttackAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}
void ULHBasicAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    ++ActivationSerial;
    auto* Combat = Cast<ULHCombatComponent>(ActorInfo->AbilitySystemComponent.Get());
    if (!Combat || !GetWorld() || !CommitAbility(Handle, ActorInfo, ActivationInfo) || Combat->CommitAttack() != ELHCommandReason::None)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }
    if (!IsActive()) return; // Attribute listeners may cancel during the resource deduction.
    if (auto* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get())) Character->GetCharacterMovement()->StopMovementImmediately();
    // Zero-delay impacts still execute through this explicit path; never an animation notify.
    if (Combat->GetImpactDelay() <= 0) Impact();
    else GetWorld()->GetTimerManager().SetTimer(ImpactTimer, this, &ULHBasicAttackAbility::Impact, Combat->GetImpactDelay(), false);
}
void ULHBasicAttackAbility::Impact()
{
    const uint64 Serial = ActivationSerial;
    const auto Handle = CurrentSpecHandle;
    const auto* ActorInfo = CurrentActorInfo;
    const auto ActivationInfo = CurrentActivationInfo;
    if (auto* Combat = Cast<ULHCombatComponent>(GetAbilitySystemComponentFromActorInfo()))
    {
        // Copy: publication can cancel and overwrite the component's pending identity.
        const FLHHitIdentity Identity = Combat->GetPendingIdentity();
        Combat->ResolveImpact(Identity);
    }
    // An InstancedPerActor object may now belong to a replacement activation.
    if (Serial == ActivationSerial && IsActive()) EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
void ULHBasicAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (Handle != CurrentSpecHandle || ActorInfo != CurrentActorInfo || !IsEndAbilityValid(Handle, ActorInfo)) return;
    if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(ImpactTimer);
    if (ActorInfo) if (auto* Combat = Cast<ULHCombatComponent>(ActorInfo->AbilitySystemComponent.Get())) Combat->FinishAttack();
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, false, bWasCancelled);
}
