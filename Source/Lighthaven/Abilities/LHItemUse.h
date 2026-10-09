#pragma once
#include "Abilities/LHEquipmentCombat.h"
#include "Core/LHCommands.h"
namespace LHAbilities
{
struct FLHUseItemContext
{
    FLHSaveSnapshot Snapshot;
    FLHCombatItemLookup ItemLookup;
    FLHEntityId Owner;
    double MaximumMana=0;
    bool bAlive=false;
};
LIGHTHAVEN_API ELHCommandReason ExecuteUseItem(FLHUseItemContext&, const FLHUseItemRequest&);
}
