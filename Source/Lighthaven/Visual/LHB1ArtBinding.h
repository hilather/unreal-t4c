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
    // Closed backing inside imported masonry; never collision or recipe geometry.
    LIGHTHAVEN_API TArray<FLHVisualBox> SolidBacking(const FLHVisualRecipe& Recipe, const FFit& Fit);
    // Shallow segmented masonry courses close cut surfaces inside the fitted bounds.
    LIGHTHAVEN_API TArray<FLHVisualBox> MasonryCaps(const FLHVisualRecipe& Recipe, const FFit& Fit);
    LIGHTHAVEN_API const TArray<FString>& AssetNames();
    // Unit-scale tiles retain authored UV density; the master clips excess
    // at fitted edges. Stairs rotate to the signed path and crop the run.
    LIGHTHAVEN_API bool Resolve(const FLHVisualRecipe& Recipe, FFit& Out);
}
