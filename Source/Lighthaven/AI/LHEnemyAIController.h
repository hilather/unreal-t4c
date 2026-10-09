#pragma once
#include "AIController.h"
#include "AI/LHEnemyRuntime.h"
#include "LHEnemyAIController.generated.h"
class ALHEnemyCharacter;
class ULHEncounterDirector;
enum class ELHAIState : uint8 { Idle, Acquire, Chase, Attack, ReturnHome, Dead };
UCLASS()
class LIGHTHAVEN_API ALHEnemyAIController : public AAIController
{
    GENERATED_BODY()
public:
    ALHEnemyAIController();
    void InitializeRuntime(ALHEnemyCharacter* Enemy, ULHEncounterDirector* Owner, const FVector& Home);
    void Advance(float Seconds);
    void Suspend();
    void EnterDead();
    ELHAIState GetState() const { return State; }
    int32 GetFailedPaths() const { return FailedPaths; }
    bool IsPursuing() const { return State == ELHAIState::Chase || State == ELHAIState::Attack; }
    // Called by navigation completion and synchronous rejected requests; bounded by spec.
    void RecordPathFailure();
protected:
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
    virtual void Tick(float DeltaSeconds) override;
private:
    void ReturnHome();
    bool MoveSafely(const FVector& Destination);
    ULHCombatComponent* TargetCombat() const;
    TWeakObjectPtr<ALHEnemyCharacter> Enemy;
    TWeakObjectPtr<ULHEncounterDirector> Director;
    TWeakObjectPtr<AActor> Target;
    FVector Anchor = FVector::ZeroVector, LastSafeLocation = FVector::ZeroVector;
    ELHAIState State = ELHAIState::Idle;
    double DecisionElapsed = 0;
    int32 FailedPaths = 0;
    bool bHomePathExhausted = false, bSuspended = false;
};
