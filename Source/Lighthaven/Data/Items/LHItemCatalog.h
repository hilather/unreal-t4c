#pragma once
#include "Character/LHCharacterAuthority.h"
#include "Abilities/LHEquipmentCombat.h"
struct LIGHTHAVEN_API FLHItemCatalogRow
{
    FLHContentId Id;
    FLHCharacterItemDefinition Character;
    FLHCombatItemData Combat;
    FLHInteger BuyGold, SellGold;
    FLHNumber Encumbrance; // Historical field stays unresolved for unknown loot.
    FLHPolicyField CarryPolicy;
    FLHFieldProvenance Membership;
    // False means carry-only: unknown mechanical stats and sale price never imply zero.
    bool bEquipable=false;
};
struct LIGHTHAVEN_API FLHStarterGrant { FLHContentId Item; FLHInteger Quantity; };
namespace LHItemData
{
LIGHTHAVEN_API const TArray<FLHItemCatalogRow>& Catalog();
LIGHTHAVEN_API const FLHItemCatalogRow* Find(const FLHContentId&);
LIGHTHAVEN_API const TArray<FLHStarterGrant>& StartingKit();
}
