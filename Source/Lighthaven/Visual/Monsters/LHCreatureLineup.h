#pragma once
#include "CoreMinimal.h"
#if !UE_BUILD_SHIPPING
class ULHMonsterVisual;
namespace LHCreatureLineupPrivate
{
    // Development capture/test seam. Requires real imported art; rejects invalid times.
    LIGHTHAVEN_API bool Sample(ULHMonsterVisual* Visual, FName Definition, const FString& Action, double Normalized);
}
#endif
