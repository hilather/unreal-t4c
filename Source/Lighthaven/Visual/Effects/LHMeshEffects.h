#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Abilities/LHCombatComponent.h"
#include "LHMeshEffects.generated.h"
class UStaticMeshComponent;
USTRUCT()
struct FLHMeshEffectSlot
{
    GENERATED_BODY()
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
    FName Kind;
    FLHHitIdentity Identity;
    FVector Start=FVector::ZeroVector,End=FVector::ZeroVector;
    float Age=0,Duration=0;
    bool Travel=false;
};
UCLASS()
class LIGHTHAVEN_API ULHMeshEffects : public UActorComponent
{
    GENERATED_BODY()
public:
    ULHMeshEffects();
    void Bind(ULHCombatComponent* Combat);
    bool Emit(FName Kind,FVector Start,FVector End,float Duration,bool Travel=false,const FLHHitIdentity& Identity={});
    void Advance(float Seconds);
    int32 ActiveCount() const;
    static constexpr int32 PoolCap=16;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    void Committed(const FLHAttackEvent& Event);
    void Finished(const FLHAttackEvent& Event,ELHAttackOutcome Outcome);
    AActor* TargetActor(const FLHAttackEvent& Event) const;
    FVector Target(const FLHAttackEvent& Event) const;
    FVector Ground(const FLHAttackEvent& Event) const;
    UPROPERTY() TArray<FLHMeshEffectSlot> Slots;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Orb;
    TWeakObjectPtr<ULHCombatComponent> Bound;
    FDelegateHandle CommitHandle,FinishHandle;
    float OrbFade=0;
};
