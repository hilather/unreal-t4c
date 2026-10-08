#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "LHEnemyCharacter.generated.h"
class ULHCombatComponent;
// Also the native dummy target. No invented health/damage defaults or presentation assets.
UCLASS()
class LIGHTHAVEN_API ALHEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    ALHEnemyCharacter();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    ULHCombatComponent* GetCombatComponent() const { return Combat; }
    void InitializeAfterRestore();
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY(VisibleAnywhere) TObjectPtr<ULHCombatComponent> Combat;
};
