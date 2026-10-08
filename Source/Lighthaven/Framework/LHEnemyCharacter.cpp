#include "Framework/LHEnemyCharacter.h"
#include "Abilities/LHCombatComponent.h"
#include "Framework/LHDevCombatFixture.h"
#include "Components/CapsuleComponent.h"
ALHEnemyCharacter::ALHEnemyCharacter()
{
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    Combat = CreateDefaultSubobject<ULHCombatComponent>(TEXT("Combat"));
}
UAbilitySystemComponent* ALHEnemyCharacter::GetAbilitySystemComponent() const { return Combat; }
void ALHEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    LHDevCombat::InitializeForMap(Combat, this);
    InitializeAfterRestore();
    Combat->OnDeath.AddWeakLambda(this, [this](const FLHHitIdentity&)
    {
        UE_LOG(LogTemp, Display, TEXT("LH dummy/enemy death: %s"), *GetName());
    });
}
void ALHEnemyCharacter::InitializeAfterRestore() { Combat->InitializeCombatActorInfo(this, this); }
void ALHEnemyCharacter::EndPlay(const EEndPlayReason::Type Reason) { Combat->ClearCombatAvatar(); Super::EndPlay(Reason); }
