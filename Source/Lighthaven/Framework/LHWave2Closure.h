#pragma once
#include "Character/LHCharacterAuthority.h"
namespace LHWave2
{
// Explicit allowlisted mechanical closure. No ContentHash, cosmetic IDs or transient state.
LIGHTHAVEN_API TArray<uint8> MechanicalClosure(const FLHCharacterProfile& Profile);
LIGHTHAVEN_API TArray<uint8> GameplayCatalogClosure();
}
