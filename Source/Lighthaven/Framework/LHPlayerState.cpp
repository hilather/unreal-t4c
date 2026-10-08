#include "Framework/LHPlayerState.h"
#include "Character/LHCharacterAuthority.h"
#include "Abilities/LHCombatComponent.h"
#include "GameFramework/Pawn.h"
#include "Framework/LHPlayerController.h"
ALHPlayerState::ALHPlayerState()
{
    CharacterAuthority = CreateDefaultSubobject<ULHCharacterAuthorityComponent>(TEXT("CharacterAuthority"));
    Combat = CreateDefaultSubobject<ULHCombatComponent>(TEXT("Combat"));
}
UAbilitySystemComponent* ALHPlayerState::GetAbilitySystemComponent() const { return Combat; }
void ALHPlayerState::InitializeAvatar(APawn* Avatar)
{
    Combat->OnDeath.RemoveAll(this);
    Combat->OnImpact.RemoveAll(this);
    Combat->InitializeCombatActorInfo(this, Avatar);
    Combat->OnDeath.AddUObject(this, &ALHPlayerState::HandleAvatarDeath);
    Combat->OnImpact.AddWeakLambda(this, [](const FLHHitIdentity& Id, const LH::Rules::FCombatResult& Result)
    {
        UE_LOG(LogTemp, Display, TEXT("LH combat impact %s/%d: hit=%d damage=%.2f (rules output; dev tuning is Prototype)"),
            *Id.ActivationId.ToString(), Id.ImpactIndex, Result.bHit, Result.Damage);
    });
}
void ALHPlayerState::HandleAvatarDeath(const FLHHitIdentity&)
{
    if (auto* Pawn=Cast<APawn>(Combat->GetAvatarActor()))
        if (auto* Controller=Cast<ALHPlayerController>(Pawn->GetController())) Controller->ClearHeldMovement();
    ClearAvatar();
}
void ALHPlayerState::ClearAvatar() { Combat->ClearCombatAvatar(); }
