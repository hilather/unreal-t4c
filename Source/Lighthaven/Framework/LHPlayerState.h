#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "LHPlayerState.generated.h"
class ULHCombatComponent;
class ULHCharacterAuthorityComponent;
struct FLHHitIdentity;
UCLASS()
class LIGHTHAVEN_API ALHPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    ALHPlayerState();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    ULHCombatComponent* GetCombatComponent() const { return Combat; }
    ULHCharacterAuthorityComponent* GetCharacterAuthority() const { return CharacterAuthority; }
    // GAS defaults its avatar to the owner; expose only the bound gameplay pawn.
    APawn* GetCombatAvatar() const;
    void InitializeAvatar(APawn* Avatar);
    void ClearAvatar();
private:
    void HandleAvatarDeath(const FLHHitIdentity& Identity);
    UPROPERTY(VisibleAnywhere) TObjectPtr<ULHCombatComponent> Combat;
    UPROPERTY(VisibleAnywhere) TObjectPtr<ULHCharacterAuthorityComponent> CharacterAuthority;
};
