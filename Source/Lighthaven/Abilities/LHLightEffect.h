#pragma once
#include "Abilities/LHCombatComponent.h"
#include "Core/LHSaveSnapshot.h"
namespace LHAbilities
{
// Atomic capture/restore; empty means no Light. Caller captures at completed boundaries.
LIGHTHAVEN_API bool CaptureLightEffect(const ULHCombatComponent&, const FLHEntityId& Owner, FLHSessionRecord&);
LIGHTHAVEN_API bool RestoreLightEffect(ULHCombatComponent&, const FLHEntityId& Owner, const FLHSessionRecord&);
}
