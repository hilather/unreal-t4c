#include "Framework/LHPlayerController.h"
#include "Framework/LHCharacter.h"
#include "Framework/LHPlayerState.h"
#include "Framework/LHEnemyCharacter.h"
#include "Framework/LHDevCombatFixture.h"
#include "Abilities/LHCombatComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/CoreDelegates.h"
#include "UnrealClient.h"
ALHPlayerController::ALHPlayerController() { bShowMouseCursor=true; }
void ALHPlayerController::BeginPlay()
{
    Super::BeginPlay();
    DeactivateHandle=FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this,&ALHPlayerController::ClearHeldMovement);
    SetControlContext(ActiveContext);
}
void ALHPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
    FCoreDelegates::ApplicationWillDeactivateDelegate.Remove(DeactivateHandle);
    if (auto* State=GetPlayerState<ALHPlayerState>()) State->ClearAvatar();
    ClearHeldMovement();
    Super::EndPlay(Reason);
}
void ALHPlayerController::OnPossess(APawn* Pawn)
{
    Super::OnPossess(Pawn);
    if (auto* State=GetPlayerState<ALHPlayerState>())
    {
        LHDevCombat::InitializeForMap(State->GetCombatComponent(), Pawn);
        State->InitializeAvatar(Pawn);
    }
    ClearHeldMovement();
}
void ALHPlayerController::OnUnPossess()
{
    if (auto* State=GetPlayerState<ALHPlayerState>()) State->ClearAvatar();
    ClearHeldMovement(); SelectedTarget.Reset(); Super::OnUnPossess();
}
void ALHPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputConfig) { InputConfig=NewObject<ULHInputConfig>(this); InputConfig->Initialize(); }
    auto* E=Cast<UEnhancedInputComponent>(InputComponent);
    if (!ensureMsgf(E,TEXT("Integrator must configure EnhancedInputComponent as the default input component"))) return;
    E->BindAction(InputConfig->Action("Move"),ETriggerEvent::Triggered,this,&ALHPlayerController::Move);
    E->BindAction(InputConfig->Action("Move"),ETriggerEvent::Completed,this,&ALHPlayerController::StopMove);
    E->BindAction(InputConfig->Action("Move"),ETriggerEvent::Canceled,this,&ALHPlayerController::StopMove);
    E->BindAction(InputConfig->Action("Look"),ETriggerEvent::Triggered,this,&ALHPlayerController::Look);
    E->BindAction(InputConfig->Action("Zoom"),ETriggerEvent::Triggered,this,&ALHPlayerController::Zoom);
#define LH_BIND(Name, Method) E->BindAction(InputConfig->Action(Name),ETriggerEvent::Started,this,&ALHPlayerController::Method)
    LH_BIND("ToggleRun",ToggleRun); LH_BIND("Attack",Attack); LH_BIND("Interact",Interact);
    LH_BIND("TargetNext",NextTarget); LH_BIND("TargetPrev",PrevTarget); LH_BIND("CancelTarget",CancelTarget);
    LH_BIND("OpenCharacter",OpenCharacter); LH_BIND("OpenInventory",OpenInventory); LH_BIND("Pause",PauseMenu);
    LH_BIND("Confirm",Confirm); LH_BIND("Back",Back);
#undef LH_BIND
    E->BindAction(InputConfig->Action("Navigate"),ETriggerEvent::Triggered,this,&ALHPlayerController::Navigate);
    E->BindAction(InputConfig->Action("SelectMouse"),ETriggerEvent::Started,this,&ALHPlayerController::SelectMouse);
    SetControlContext(ActiveContext);
}
void ALHPlayerController::ClearHeldMovement()
{
    Movement.Clear(); bAwaitMoveRelease=true; FlushPressedKeys();
    if (auto* C=Cast<ALHCharacter>(GetPawn())) { C->ConsumeMovementInputVector(); C->GetCharacterMovement()->StopMovementImmediately(); }
}
void ALHPlayerController::SetControlContext(ELHInputContext Context)
{
    ClearHeldMovement(); Movement.SwitchContext(Context); ActiveContext=Context;
    if (!InputConfig) { InputConfig=NewObject<ULHInputConfig>(this); InputConfig->Initialize(); }
    if (auto* Local=GetLocalPlayer()) if (auto* S=Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    {
        for (int32 I=0; I<3; ++I) S->RemoveMappingContext(InputConfig->Context(ELHInputContext(I)));
        FModifyContextOptions Options; Options.bIgnoreAllPressedKeysUntilRelease=true;
        S->AddMappingContext(InputConfig->Context(Context),0,Options);
    }
}
void ALHPlayerController::PlayerTick(float DeltaSeconds)
{
    Super::PlayerTick(DeltaSeconds);
    auto* V=GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
    const bool Focus=V && V->Viewport && V->Viewport->IsForegroundWindow();
    if (!Focus) { if (bHadFocus) ClearHeldMovement(); bHadFocus=false; return; }
    if (!bHadFocus) { SetControlContext(ActiveContext); bHadFocus=true; }
    if (bAwaitMoveRelease && !IsInputKeyDown(EKeys::W) && !IsInputKeyDown(EKeys::A) && !IsInputKeyDown(EKeys::S) && !IsInputKeyDown(EKeys::D) && FMath::Abs(GetInputAnalogKeyState(EKeys::Gamepad_LeftX))<0.2f && FMath::Abs(GetInputAnalogKeyState(EKeys::Gamepad_LeftY))<0.2f) bAwaitMoveRelease=false;
    if (SelectedTarget.IsValid() && !ValidTarget(SelectedTarget.Get())) SelectedTarget.Reset();
    if (ActiveContext==ELHInputContext::Gameplay) if (auto* C=Cast<ALHCharacter>(GetPawn()))
    {
        auto* State=GetPlayerState<ALHPlayerState>();
        if (State && State->GetCombatComponent()->IsActionPending()) { C->ConsumeMovementInputVector(); return; }
        const FRotator Yaw(0,C->CameraYaw(),0);
        C->AddMovementInput(Yaw.Vector(),Movement.Held.Y);
        C->AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y),Movement.Held.X);
    }
}
void ALHPlayerController::Move(const FInputActionValue& V) { SubmitMovement(V.Get<FVector2D>()); }
void ALHPlayerController::SubmitMovement(FVector2D Axis)
{
    if (Axis.IsNearlyZero()) bAwaitMoveRelease=false;
    if (bHadFocus && !bAwaitMoveRelease && ActiveContext==ELHInputContext::Gameplay)
        Movement.Held=Axis.GetClampedToMaxSize(1);
}
void ALHPlayerController::StopMove(const FInputActionValue&) { Movement.Clear(); }
void ALHPlayerController::Look(const FInputActionValue& V)
{
    if (!bHadFocus || ActiveContext!=ELHInputContext::Gameplay) return;
    if (auto* C=Cast<ALHCharacter>(GetPawn()))
    {
        // Mouse2D only contributes while middle dragging; right stick remains independent.
        FVector2D D=V.Get<FVector2D>();
        const FVector2D Stick(GetInputAnalogKeyState(EKeys::Gamepad_RightX),GetInputAnalogKeyState(EKeys::Gamepad_RightY));
        D=IsInputKeyDown(EKeys::MiddleMouseButton) ? D : Stick;
        C->RotateCamera(D*GetWorld()->GetDeltaSeconds()*90.f);
    }
}
void ALHPlayerController::Zoom(const FInputActionValue& V) { if (!bHadFocus || ActiveContext!=ELHInputContext::Gameplay) return; if (auto* C=Cast<ALHCharacter>(GetPawn())) { const bool Pad=IsInputKeyDown(EKeys::Gamepad_DPad_Up) || IsInputKeyDown(EKeys::Gamepad_DPad_Down); C->ZoomCamera(V.Get<float>()*(Pad ? GetWorld()->GetDeltaSeconds()*5.f : 1.f)); } }
void ALHPlayerController::ToggleRun() { if (auto* C=Cast<ALHCharacter>(GetPawn())) C->ToggleRun(); }
bool ALHPlayerController::ValidTarget(AActor* A) const
{
    if (!IsValid(A) || A==GetPawn() || !GetPawn()) return false;
    if (auto* Enemy=Cast<ALHEnemyCharacter>(A))
    {
        auto* State=GetPlayerState<ALHPlayerState>();
        if (!State) return false;
        // Reuse authority's life/range/LOS checks. Temporary action/resource rejection
        // does not remove a selectable target; submission still validates everything.
        const auto Reason=State->GetCombatComponent()->ValidateAttack(Enemy->GetCombatComponent());
        if (Reason!=ELHCommandReason::None && Reason!=ELHCommandReason::ActiveAction &&
            Reason!=ELHCommandReason::Cooldown && Reason!=ELHCommandReason::InsufficientMana) return false;
        if (!Enemy->GetCombatComponent()->IsAlive()) return false;

    }
    else
    {
        if (!A->Implements<ULHControlTarget>()) return false;
        if (!ILHControlTarget::Execute_IsControlTargetAlive(A) || !ILHControlTarget::Execute_IsControlTargetReachable(A,GetPawn())) return false;
    }
    if (FVector::DistSquared(A->GetActorLocation(),GetPawn()->GetActorLocation())>FMath::Square(SelectionRange)) return false;
    FHitResult Hit; FCollisionQueryParams Params; Params.AddIgnoredActor(GetPawn());
    const bool Blocked=GetWorld()->LineTraceSingleByChannel(Hit,GetPawn()->GetActorLocation(),A->GetActorLocation(),ECC_Visibility,Params);
    return !Blocked || Hit.GetActor()==A;
}
void ALHPlayerController::Cycle(int32 Direction)
{
    if (ActiveContext!=ELHInputContext::Gameplay) return;
    TArray<FLHTargetCandidate> Candidates; TMap<FString,AActor*> Actors;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It) if (ValidTarget(*It))
    {
        const FString Key=It->GetPathName(); Actors.Add(Key,*It);
        Candidates.Add({Key,FVector::DistSquared(It->GetActorLocation(),GetPawn()->GetActorLocation()),true,true,true});
    }
    const auto Order=LHControls::OrderedTargets(Candidates,FMath::Square(SelectionRange));
    const FString Key=LHControls::CycleTarget(Order,SelectedTarget.IsValid() ? SelectedTarget->GetPathName() : FString(),Direction);
    SelectedTarget=Actors.FindRef(Key);
}
void ALHPlayerController::NextTarget() { Cycle(1); }
void ALHPlayerController::PrevTarget() { Cycle(-1); }
void ALHPlayerController::CancelTarget() { SelectedTarget.Reset(); }
void ALHPlayerController::SelectMouse()
{
    if (ActiveContext!=ELHInputContext::Gameplay || !bHadFocus) return;
    FHitResult Hit; GetHitResultUnderCursor(ECC_Visibility,false,Hit);
    SelectedTarget=ValidTarget(Hit.GetActor()) ? Hit.GetActor() : nullptr;
}
bool ALHPlayerController::SelectTarget(AActor* Target)
{
    SelectedTarget=ActiveContext==ELHInputContext::Gameplay && ValidTarget(Target) ? Target : nullptr;
    return SelectedTarget.IsValid();
}
ELHCommandReason ALHPlayerController::RequestSelectedAttack()
{
    if (ActiveContext!=ELHInputContext::Gameplay) return ELHCommandReason::InvalidRequest;
    auto* State=GetPlayerState<ALHPlayerState>();
    auto* Enemy=Cast<ALHEnemyCharacter>(SelectedTarget.Get());
    if (!State || !Enemy) return ELHCommandReason::NotFound;
    const auto Reason=State->GetCombatComponent()->RequestBasicAttack(Enemy->GetCombatComponent());
    UE_LOG(LogTemp, Display, TEXT("LH attack request: %s"), *StaticEnum<ELHCommandReason>()->GetNameStringByValue(static_cast<int64>(Reason)));
    if (Reason==ELHCommandReason::None) { ClearHeldMovement(); OnAttackRequested.Broadcast(Enemy); }
    return Reason;
}
void ALHPlayerController::Attack() { RequestSelectedAttack(); }
// Wave 2+: intent notification only, no interaction transaction exists yet.
void ALHPlayerController::Interact() { if (!SelectedTarget.IsValid()) Cycle(1); if (ActiveContext==ELHInputContext::Gameplay && ValidTarget(SelectedTarget.Get())) OnInteractRequested.Broadcast(SelectedTarget.Get()); }
void ALHPlayerController::OpenScreen(FName Screen) { SetControlContext(ELHInputContext::UI); OnScreenRequested.Broadcast(Screen); }
void ALHPlayerController::OpenCharacter() { OpenScreen("Character"); }
void ALHPlayerController::OpenInventory() { OpenScreen("Inventory"); }
void ALHPlayerController::PauseMenu() { OpenScreen("Pause"); }
void ALHPlayerController::Navigate(const FInputActionValue& V) { const auto D=V.Get<FVector2D>(); OnMenuInput.Broadcast(FMath::Abs(D.Y)>FMath::Abs(D.X) ? (D.Y>0 ? "Up" : "Down") : (D.X>0 ? "Right" : "Left")); }
void ALHPlayerController::Confirm() { OnMenuInput.Broadcast("Confirm"); }
void ALHPlayerController::Back() { OnMenuInput.Broadcast("Back"); }
