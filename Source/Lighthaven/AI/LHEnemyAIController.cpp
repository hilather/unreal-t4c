#include "AI/LHEnemyAIController.h"
#include "AI/LHEncounterDirector.h"
#include "Framework/LHEnemyCharacter.h"
#include "AbilitySystemInterface.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
ALHEnemyAIController::ALHEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;
    bAllowTickBeforeBeginPlay = false;
}
void ALHEnemyAIController::InitializeRuntime(ALHEnemyCharacter* E, ULHEncounterDirector* D, const FVector& Home)
{ Enemy = E; Director = D; Anchor = Home; LastSafeLocation = E->GetActorLocation(); Possess(E); }
ULHCombatComponent* ALHEnemyAIController::TargetCombat() const
{
    const auto* Interface = Cast<IAbilitySystemInterface>(Target.Get());
    return Interface ? Cast<ULHCombatComponent>(Interface->GetAbilitySystemComponent()) : nullptr;
}
void ALHEnemyAIController::Suspend()
{
    if (State == ELHAIState::Dead) return;
    bSuspended = true; StopMovement();
    if (Enemy.IsValid()) { Enemy->GetCombatComponent()->CancelAllAbilities(); Enemy->GetCharacterMovement()->StopMovementImmediately(); }
}
void ALHEnemyAIController::EnterDead()
{ State = ELHAIState::Dead; Target.Reset(); StopMovement(); }
void ALHEnemyAIController::ReturnHome()
{
    State = ELHAIState::ReturnHome; Target.Reset(); FailedPaths = 0; bHomePathExhausted = false;
    StopMovement();
    if (Enemy.IsValid()) Enemy->GetCombatComponent()->CancelAllAbilities();
}
void ALHEnemyAIController::RecordPathFailure()
{
    if (!Enemy.IsValid() || State == ELHAIState::Dead || bSuspended) return;
    ++FailedPaths;
    if (FailedPaths >= Enemy->GetRuntimeSpec()->PathRetryLimit.Value)
    {
        if (State == ELHAIState::ReturnHome) { bHomePathExhausted = true; StopMovement(); }
        else ReturnHome();
    }
}
void ALHEnemyAIController::OnMoveCompleted(FAIRequestID ID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(ID, Result);
    if (Result.Code == EPathFollowingResult::Blocked || Result.Code == EPathFollowingResult::Invalid || Result.Code == EPathFollowingResult::OffPath) RecordPathFailure();
    else if (Result.IsSuccess()) FailedPaths = 0;
}
bool ALHEnemyAIController::MoveSafely(const FVector& Destination)
{
    auto* E = Enemy.Get(); auto* D = Director.Get();
    if (!E || !D || bHomePathExhausted) return false;
    const auto& S = *E->GetRuntimeSpec();
    if (GetMoveStatus() == EPathFollowingStatus::Moving) return true;
    auto* Path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), E->GetActorLocation(), Destination, E);
    if (!Path || !Path->IsValid() || Path->IsPartial() || Path->PathPoints.Num() < 2) { RecordPathFailure(); return false; }
    for (int32 I = 1; I < Path->PathPoints.Num(); ++I)
    {
        if (!D->IsSafeSegment(Path->PathPoints[I-1], Path->PathPoints[I], S.CapsuleRadiusCm.Value, S.CapsuleHalfHeightCm.Value) ||
            FVector::Dist2D(Path->PathPoints[I], Anchor) > S.LeashRadiusCm.Value)
        { RecordPathFailure(); return false; }
    }
    FAIMoveRequest Request;
    Request.SetGoalLocation(Destination); Request.SetAcceptanceRadius(S.HomeToleranceCm.Value);
    Request.SetAllowPartialPath(false); Request.SetUsePathfinding(true); Request.SetReachTestIncludesAgentRadius(false);
    if (MoveTo(Request) == EPathFollowingRequestResult::Failed) { RecordPathFailure(); return false; }
    return true;
}
void ALHEnemyAIController::Tick(float Seconds)
{
    Super::Tick(Seconds);
    // Only a cached target guard per frame, never acquisition/navigation/world scans.
    auto* D = Director.Get(); auto* E = Enemy.Get();
    if (!D || !E || State == ELHAIState::Dead) return;
    auto* C = TargetCombat();
    if (!D->IsSimulationEnabled() || (Target.IsValid() && (!C || !C->IsAlive()))) { Suspend(); return; }
    const auto& S = *E->GetRuntimeSpec();
    const FVector Here = E->GetActorLocation();
    if (!D->IsSafeSegment(LastSafeLocation, Here, S.CapsuleRadiusCm.Value, S.CapsuleHalfHeightCm.Value) ||
        FVector::Dist2D(Here, Anchor) > S.LeashRadiusCm.Value)
    {
        E->GetCharacterMovement()->StopMovementImmediately();
        E->SetActorLocation(LastSafeLocation); ReturnHome();
    }
    else LastSafeLocation = Here;
    // PostPhysics runs after pawn movement and before TimerManager impact callbacks.
    if (Target.IsValid() && !D->IsSafeSegment(E->GetActorLocation(), Target->GetActorLocation())) ReturnHome();
}
void ALHEnemyAIController::Advance(float Seconds)
{
    if (State == ELHAIState::Dead) return;
    auto* E = Enemy.Get(); auto* D = Director.Get();
    if (!E || !D || !E->GetRuntimeSpec()) return;
    if (!E->IsAlive()) { EnterDead(); return; }
    if (!D->IsSimulationEnabled()) { Suspend(); return; }
    bSuspended = false;
    const auto& S = *E->GetRuntimeSpec();
    DecisionElapsed += Seconds;
    if (DecisionElapsed < S.DecisionIntervalSeconds.Value) return;
    DecisionElapsed = 0; // At most one decision per caller boundary, no catch-up loop.
    auto* Player = D->Player.Get();
    const auto* Interface = Cast<IAbilitySystemInterface>(Player);
    auto* PC = Interface ? Cast<ULHCombatComponent>(Interface->GetAbilitySystemComponent()) : nullptr;
    if (!Player || !PC || !PC->IsAlive()) { Suspend(); return; }
    const FVector Here = E->GetActorLocation();
    if (State == ELHAIState::ReturnHome)
    {
        if (FVector::Dist2D(Here, Anchor) <= S.HomeToleranceCm.Value)
        { StopMovement(); State = ELHAIState::Idle; FailedPaths = 0; bHomePathExhausted = false; }
        else MoveSafely(Anchor);
        return;
    }
    if (State == ELHAIState::Idle || State == ELHAIState::Acquire)
    {
        State = ELHAIState::Acquire;
        if (FVector::Dist(Here, Player->GetActorLocation()) > S.AggroRadiusCm.Value ||
            FVector::Dist2D(Player->GetActorLocation(), Anchor) > S.LeashRadiusCm.Value || !D->HasSight(E, Player) ||
            !D->IsSafeSegment(Here, Player->GetActorLocation()) || !D->CanPursue(this, S.MaxActivePursuers.Value))
        { State = ELHAIState::Idle; return; }
        Target = Player; State = ELHAIState::Chase; FailedPaths = 0;
        return;
    }
    auto* TC = TargetCombat();
    if (!TC || !TC->IsAlive() || FVector::Dist2D(Here, Anchor) > S.LeashRadiusCm.Value ||
        FVector::Dist2D(Target->GetActorLocation(), Anchor) > S.LeashRadiusCm.Value ||
        !D->IsSafeSegment(Here, Target->GetActorLocation())) { ReturnHome(); return; }
    if (FVector::Dist(Here, Target->GetActorLocation()) <= S.Attack.RangeCm.Value && D->HasSight(E, Target.Get()))
    { StopMovement(); State = ELHAIState::Attack; E->GetCombatComponent()->RequestBasicAttack(TC); }
    else { State = ELHAIState::Chase; MoveSafely(Target->GetActorLocation()); }
}
