#pragma once
#include "Core/LHSaveSnapshot.h"
enum class ELHQuiverConsumption : uint8 { Unlimited };
struct LIGHTHAVEN_API FLHWeaponCombatData
{
    FLHNumber MinimumDamage, MaximumDamage, RangeCm;
    bool bBow=false;
    TArray<FLHContentId> CompatibleQuivers;
    FLHNumber QuiverDamageBonus;
    ELHQuiverConsumption QuiverConsumption=ELHQuiverConsumption::Unlimited;
};
struct LIGHTHAVEN_API FLHConsumableEffect { FLHNumber ManaRestore; };
struct LIGHTHAVEN_API FLHCombatItemData
{
    bool bWeapon=false, bQuiver=false, bConsumable=false;
    FLHWeaponCombatData Weapon;
    FLHConsumableEffect Consumable;
};
using FLHCombatItemLookup = TFunction<const FLHCombatItemData*(const FLHContentId&)>;
