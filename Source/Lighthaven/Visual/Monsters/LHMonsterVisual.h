#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LHMonsterVisual.generated.h"
class ALHVisualPiece;
class UStaticMesh;
enum class ELHMonsterMotion : uint8 { Idle, Move, Telegraph, Strike, Hit, Dead };
// Presentation clock only. No gameplay writes, random draws or timers.
UCLASS()
class LIGHTHAVEN_API ULHMonsterVisual : public USceneComponent
{
    GENERATED_BODY()
public:
    ULHMonsterVisual();
    bool Build(FName DefinitionId, double Radius, double HalfHeight);
    static bool Known(FName DefinitionId);
    static uint32 RecipeFingerprint(FName DefinitionId);
    void Attack(double ImpactDelay, double CommitTime = -1);
    void CancelAttack();
    void Strike();
    void Hit();
    void Die();
    void AdvancePresentation(float Seconds, bool Moving);
    void SetPresentationEnabled(bool Enabled) { bPresentationEnabled=Enabled; SetVisibility(Enabled, true); SetComponentTickEnabled(Enabled); }
    ELHMonsterMotion GetMotion() const { return Motion; }
    const TArray<TObjectPtr<USceneComponent>>& GetParts() const { return Parts; }
    virtual void TickComponent(float DeltaTime, ELevelTick Type, FActorComponentTickFunction* Tick) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    void Clear();
    // Hard CDO references expose engine primitive cook dependencies.
    UPROPERTY() TArray<TObjectPtr<UStaticMesh>> ShapeMeshes;
    UPROPERTY() TArray<TObjectPtr<USceneComponent>> Parts;
    UPROPERTY() TArray<TObjectPtr<ALHVisualPiece>> KitPieces;
    TArray<FTransform> Rest;
    TArray<bool> Weapons;
    FName Definition;
    double FlyingFloor=0, CommitWorldTime=-1;
    double Clock=0, AttackElapsed=0, Impact=0, HitRemaining=0, StrikeRemaining=0, DeathElapsed=0;
    bool Pending=false, Dead=false, bPresentationEnabled=true;
    ELHMonsterMotion Motion=ELHMonsterMotion::Idle;
};
