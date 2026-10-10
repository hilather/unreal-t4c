#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Input/LHInputConfig.h"
#include "Input/LHTargeting.h"
#include "Core/LHCommands.h"
#include "LHPlayerController.generated.h"
struct FInputActionValue;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLHControlRequest, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLHScreenRequest, FName, Screen);
UCLASS()
class LIGHTHAVEN_API ALHPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ALHPlayerController();
    virtual void SetupInputComponent() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void PlayerTick(float DeltaSeconds) override;
    virtual void FlushPressedKeys() override;
    virtual bool InputKey(const FInputKeyEventArgs& Params) override;
    virtual void OnPossess(APawn* Pawn) override;
    virtual void OnUnPossess() override;
    UFUNCTION(BlueprintCallable) void SetControlContext(ELHInputContext Context);
    UFUNCTION(BlueprintCallable) void ClearHeldMovement();
    // Shared startup/possession/resume seam; keeps the existing release gate.
    void EstablishGameplayInput();
    UFUNCTION(BlueprintPure) AActor* GetSelectedTarget() const { return SelectedTarget.Get(); }
    // Native transient-world intent seam; canonical command IDs/replay remain future work.
    UFUNCTION(BlueprintCallable) bool SelectTarget(AActor* Target);
    UFUNCTION(BlueprintCallable) void CycleTarget(int32 Direction) { Cycle(Direction); }
    UFUNCTION(BlueprintCallable) ELHCommandReason RequestSelectedAttack();
    FVector2D GetHeldMovement() const { return Movement.Held; }
    // Semantic movement ingress shared by Enhanced Input and integration tests.
    void SubmitMovement(FVector2D Axis);
    // Notification only: listeners must not implement a second damage path.
    UPROPERTY(BlueprintAssignable) FLHControlRequest OnAttackRequested;
    UPROPERTY(BlueprintAssignable) FLHControlRequest OnInteractRequested;
    UPROPERTY(BlueprintAssignable) FLHScreenRequest OnScreenRequested;
    UPROPERTY(BlueprintAssignable) FLHScreenRequest OnMenuInput;
    UPROPERTY(EditAnywhere, Category="Prototype") float SelectionRange=200.f;
    UPROPERTY(Transient) TObjectPtr<ULHInputConfig> InputConfig;
private:
    friend class FLHReachHorizontal;
    TSharedPtr<class FLHWave2Session> LiveSession;
    TSharedPtr<class FLHUIPresenter> JournalPresenter;
    TSharedPtr<class SLHFrontendWidget> JournalWidget;
    void CloseJournal();
    bool bCloseJournalRequested=false;
    ELHInputContext ActiveContext=ELHInputContext::Gameplay;
    LHControls::FMovementState Movement;
    TWeakObjectPtr<AActor> SelectedTarget;
    FDelegateHandle DeactivateHandle;
    FDelegateHandle ReactivateHandle;
    bool bApplicationActive=true;
    TSet<FKey> FlushedMovementKeys;
    void ApplicationDeactivated();
    void ApplicationReactivated();
    bool MovementInputsReleased() const;
    bool bHadFocus=true;
    bool bAwaitMoveRelease=true;
    bool HasLiveMovementAvatar() const;
    void Move(const FInputActionValue& Value);
    void StopMove(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Zoom(const FInputActionValue& Value);
    void ToggleRun();
    void Attack();
    void Hotbar1(); void Hotbar2(); void Hotbar3(); void Hotbar4(); void Hotbar5(); void Hotbar6(); void HotbarItem();
    TSharedPtr<class SWidget> HudWidget;
    void ShowTargetScreen(FName Screen, const FLHEntityId& Target);

    void Interact();
    void NextTarget();
    void PrevTarget();
    void CancelTarget();
    void SelectMouse();
    void OpenCharacter();
    void OpenInventory();
    void OpenAbilities();
    void PauseMenu();
    void Navigate(const FInputActionValue& Value);
    void Confirm();
    void Back();
    float EffectiveSelectionRange() const;
    bool ValidTarget(AActor* Actor) const;
    void Cycle(int32 Direction);
    void OpenScreen(FName Screen);
};
