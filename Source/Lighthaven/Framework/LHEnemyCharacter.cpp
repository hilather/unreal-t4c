#include "Framework/LHEnemyCharacter.h"
#include "Abilities/LHCombatComponent.h"
ALHEnemyCharacter::ALHEnemyCharacter()
{
    Combat = CreateDefaultSubobject<ULHCombatComponent>(TEXT("Combat"));
}
UAbilitySystemComponent* ALHEnemyCharacter::GetAbilitySystemComponent() const { return Combat; }
void ALHEnemyCharacter::BeginPlay() { Super::BeginPlay(); InitializeAfterRestore(); }
void ALHEnemyCharacter::InitializeAfterRestore() { Combat->InitializeCombatActorInfo(this, this); }
void ALHEnemyCharacter::EndPlay(const EEndPlayReason::Type Reason) { Combat->ClearCombatAvatar(); Super::EndPlay(Reason); }
