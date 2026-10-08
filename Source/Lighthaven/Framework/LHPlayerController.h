#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Input/LHInputConfig.h"
#include "Input/LHTargeting.h"
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
    virtual void OnUnPossess() override;
    UFUNCTION(BlueprintCallable) void SetControlContext(ELHInputContext Context);
    UFUNCTION(BlueprintCallable) void ClearHeldMovement();
    UFUNCTION(BlueprintPure) AActor* GetSelectedTarget() const { return SelectedTarget.Get(); }
    // Explicit unconnected seams: integrator binds these to authority, converting actor to stable ID.
    UPROPERTY(BlueprintAssignable) FLHControlRequest OnAttackRequested;
    UPROPERTY(BlueprintAssignable) FLHControlRequest OnInteractRequested;
    UPROPERTY(BlueprintAssignable) FLHScreenRequest OnScreenRequested;
    UPROPERTY(BlueprintAssignable) FLHScreenRequest OnMenuInput;
    UPROPERTY(EditAnywhere, Category="Prototype") float SelectionRange=2000.f;
    UPROPERTY(Transient) TObjectPtr<ULHInputConfig> InputConfig;
private:
    ELHInputContext ActiveContext=ELHInputContext::Gameplay;
    LHControls::FMovementState Movement;
    TWeakObjectPtr<AActor> SelectedTarget;
    FDelegateHandle DeactivateHandle;
    bool bHadFocus=true;
    bool bAwaitMoveRelease=true;
    void Move(const FInputActionValue& Value);
    void StopMove(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Zoom(const FInputActionValue& Value);
    void ToggleRun();
    void Attack();
    void Interact();
    void NextTarget();
    void PrevTarget();
    void CancelTarget();
    void SelectMouse();
    void OpenCharacter();
    void OpenInventory();
    void PauseMenu();
    void Navigate(const FInputActionValue& Value);
    void Confirm();
    void Back();
    bool ValidTarget(AActor* Actor) const;
    void Cycle(int32 Direction);
    void OpenScreen(FName Screen);
};
