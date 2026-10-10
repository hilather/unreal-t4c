#pragma once
#include "Core/LHSaveSnapshot.h"
namespace LHSave
{
// Closed Stage 1 allowlist. Epoch words use exact binary64 uint32 values in existing fields.
LIGHTHAVEN_API bool ValidateDurableLight(const FLHDurableEffectRecord&, const FGuid& Epoch);
LIGHTHAVEN_API FLHDurableEffectRecord MakeDurableLight(const FLHEntityId& Owner, const FGuid& Epoch, double Remaining);
}
