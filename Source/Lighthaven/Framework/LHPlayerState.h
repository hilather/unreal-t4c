#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "LHPlayerState.generated.h"
class ULHCombatComponent;
struct FLHHitIdentity;
UCLASS()
class LIGHTHAVEN_API ALHPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    ALHPlayerState();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    ULHCombatComponent* GetCombatComponent() const { return Combat; }
    void InitializeAvatar(APawn* Avatar);
    void ClearAvatar();
private:
    void HandleAvatarDeath(const FLHHitIdentity& Identity);
    UPROPERTY(VisibleAnywhere) TObjectPtr<ULHCombatComponent> Combat;
};
