#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LHCharacter.generated.h"
class USpringArmComponent;
class UCameraComponent;
UCLASS()
class LIGHTHAVEN_API ALHCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ALHCharacter();
    void ToggleRun();
    void RotateCamera(FVector2D Delta);
    void ZoomCamera(float Delta);
    float CameraYaw() const;
    UPROPERTY(EditAnywhere, Category="Prototype") float WalkSpeed=220.f;
    UPROPERTY(EditAnywhere, Category="Prototype") float RunSpeed=450.f;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
private:
    bool bRunning=false;
};
