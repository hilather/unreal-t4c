#pragma once
#include "Visual/LHVisualKit.h"

// Prototype presentation mapping. Export names are fixed by the B1 manifest.
namespace LHB1Art
{
    struct LIGHTHAVEN_API FFit
    {
        FString AssetName;
        TArray<FTransform> Instances;
        FBox ClipBounds = FBox(FVector(-1.e8),FVector(1.e8));
    };
    LIGHTHAVEN_API const TArray<FString>& AssetNames();
    // Unit-scale tiles retain authored UV density; the master clips excess
    // at fitted edges. Stairs rotate to the signed path and crop the run.
    LIGHTHAVEN_API bool Resolve(const FLHVisualRecipe& Recipe, FFit& Out);
}
