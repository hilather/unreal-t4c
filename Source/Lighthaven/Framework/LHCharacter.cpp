#include "Framework/LHCharacter.h"
#include "Visual/Player/LHPlayerVisual.h"
#include "Framework/LHPlayerState.h"
#include "Framework/LHSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Abilities/LHCombatComponent.h"
ALHCharacter::ALHCharacter()
{
    PlayerVisual=CreateDefaultSubobject<ULHPlayerVisualComponent>(TEXT("PlayerVisual"));
    PlayerVisual->SetupAttachment(RootComponent);
    SpellLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("SpellLight"));
    SpellLight->SetupAttachment(RootComponent);
    SpellLight->SetMobility(EComponentMobility::Movable);
    SpellLight->SetAttenuationRadius(600.f); // R-03 missing radius: Prototype 600cm.
    SpellLight->SetIntensity(3000.f); // Prototype presentation intensity; host visual review.
    SpellLight->SetVisibility(false);
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

UAbilitySystemComponent* ALHCharacter::GetAbilitySystemComponent() const
{
    const auto* State = GetPlayerState<ALHPlayerState>();
    return State ? State->GetAbilitySystemComponent() : nullptr;
}

void ALHCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const auto* State=GetPlayerState<ALHPlayerState>();
    SpellLight->SetVisibility(State && State->GetCombatComponent()->GetLightRemainingSeconds()>0);
    FLHCharacterRecord Appearance;
    bool bHasAppearance=false;
    if(auto* GI=GetGameInstance()) if(auto* SS=GI->GetSubsystem<ULHSessionSubsystem>())
        if(auto Live=SS->Session(); Live && Live->HasCharacter()) { Appearance=Live->Snapshot().Character; bHasAppearance=true; }
    PlayerVisual->Present(bHasAppearance?&Appearance:nullptr,State?State->GetCombatComponent():nullptr,GetVelocity().Size2D(),DeltaSeconds);
    if (auto* Instance=GetGameInstance()) if (auto* Subsystem=Instance->GetSubsystem<ULHSessionSubsystem>())
        if (auto Session=Subsystem->Session(); Session && State && State->GetCombatAvatar()==this) Session->TickGameplay(DeltaSeconds);
}
