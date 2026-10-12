#pragma once
#include "CoreMinimal.h"
#include "Core/LHSaveSnapshot.h"
class ALHEnemyCharacter;
namespace LHLootMarker
{
    LIGHTHAVEN_API bool Lootable(const FLHCorpseLootRecord& Corpse);
    LIGHTHAVEN_API void Present(ALHEnemyCharacter* Enemy,const FLHSaveSnapshot& Snapshot);
}
