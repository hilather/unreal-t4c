#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AI/LHEnemyRuntime.h"
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
    bool ApplyRuntimeSpec(const FLHEnemyRuntimeSpec& InSpec, const FLHSpawnLifeId& InLife, const FLHNumber& Health, FString& Error);
    const FLHEnemyRuntimeSpec* GetRuntimeSpec() const { return bHasSpec ? &Spec : nullptr; }
    const FLHSpawnLifeId& GetLife() const { return Life; }
    FLHEntityId GetEntityId(const FGuid& RunId) const;
    bool IsAlive() const;
    bool IsCorpse() const { return bCorpse; }
    void MarkCorpse();
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY(VisibleAnywhere) TObjectPtr<ULHCombatComponent> Combat;
    UPROPERTY() TObjectPtr<USceneComponent> PresentationRoot;
    FLHEnemyRuntimeSpec Spec;
    FLHSpawnLifeId Life;
    bool bHasSpec = false, bCorpse = false;
};
