#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LHBasicAttackAbility.generated.h"
UCLASS()
class LIGHTHAVEN_API ULHBasicAttackAbility : public UGameplayAbility
{
    GENERATED_BODY()
public:
    ULHBasicAttackAbility();
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
    void Impact();
    uint64 ActivationSerial = 0;
    FTimerHandle ImpactTimer;
};
