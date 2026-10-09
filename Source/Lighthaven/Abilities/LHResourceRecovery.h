#pragma once
#include "Abilities/LHCombatComponent.h"
namespace LHAbilities
{
// Snapshot max mana is earned base; caller synchronizes derived max explicitly when modifiers exist.
LIGHTHAVEN_API bool AdvanceManaRegen(FLHSaveSnapshot&, double AliveUnpausedSeconds, const LH::Rules::FManaParameters&);
LIGHTHAVEN_API void CaptureCooldowns(const ULHCombatComponent&, const FLHEntityId& Owner, TArray<FLHCooldownRecord>&);
LIGHTHAVEN_API bool RestoreCooldowns(ULHCombatComponent&, const FLHEntityId& Owner, const TArray<FLHCooldownRecord>&);
}
