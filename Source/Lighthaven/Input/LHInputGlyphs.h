#pragma once
#include "CoreMinimal.h"
#include "InputCoreTypes.h"
// Registers a Slate input observer, retaining logical menu focus on device changes.
class LIGHTHAVEN_API FLHInputGlyphs
{
public:
    static void Initialize();
    static void Observe(FKey Key);
    static bool Gamepad();
    static FString MenuHints();
    static FString GameplayHints();
};
