#include "Framework/LHPlayerState.h"
#include "Abilities/LHCombatComponent.h"
#include "GameFramework/Pawn.h"
ALHPlayerState::ALHPlayerState()
{
    Combat = CreateDefaultSubobject<ULHCombatComponent>(TEXT("Combat"));
}
UAbilitySystemComponent* ALHPlayerState::GetAbilitySystemComponent() const { return Combat; }
void ALHPlayerState::InitializeAvatar(APawn* Avatar) { Combat->InitializeCombatActorInfo(this, Avatar); }
void ALHPlayerState::ClearAvatar() { Combat->ClearCombatAvatar(); }
