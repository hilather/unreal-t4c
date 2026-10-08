#include "Framework/LHCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
ALHCharacter::ALHCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(35.f,90.f);
    bUseControllerRotationYaw=false;
    GetCharacterMovement()->bOrientRotationToMovement=true;
    GetCharacterMovement()->MaxWalkSpeed=WalkSpeed;
    CameraBoom=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->SetRelativeLocation(FVector(0,0,0)); // Capsule center is 90cm above floor.
    CameraBoom->SetRelativeRotation(FRotator(-55,45,0));
    CameraBoom->TargetArmLength=1200;
    CameraBoom->bDoCollisionTest=true;
    CameraBoom->ProbeSize=12;
    Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
    Camera->FieldOfView=45; // Engine horizontal FOV; vertical-axis review remains a visual check.
}
void ALHCharacter::ToggleRun() { bRunning=!bRunning; GetCharacterMovement()->MaxWalkSpeed=bRunning ? RunSpeed : WalkSpeed; }
void ALHCharacter::RotateCamera(FVector2D Delta)
{
    FRotator R=CameraBoom->GetComponentRotation();
    R.Yaw=FMath::Clamp(R.Yaw+Delta.X,0.f,90.f);
    R.Pitch=FMath::Clamp(R.Pitch+Delta.Y,-60.f,-50.f);
    CameraBoom->SetWorldRotation(R);
}
void ALHCharacter::ZoomCamera(float Delta) { CameraBoom->TargetArmLength=FMath::Clamp(CameraBoom->TargetArmLength-Delta*100.f,900.f,1800.f); }
float ALHCharacter::CameraYaw() const { return CameraBoom->GetComponentRotation().Yaw; }
