#pragma once
#include "Character/LHCharacterAuthority.h"
namespace LHWave2
{
// Framework policy: finite numeric Rules capacity is an unused display placeholder.
struct FLHCarryCapacityPolicy
{
    bool bUnlimited=true;
    FString Provenance=TEXT("deferred by owner decision");
};
LIGHTHAVEN_API FLHCarryCapacityPolicy CarryCapacityPolicy();
LIGHTHAVEN_API FLHCharacterProfile PrototypeProfile();
LIGHTHAVEN_API FLHNumber PrototypeNumber(double V);
LIGHTHAVEN_API FLHInteger PrototypeInteger(int64 V);
LIGHTHAVEN_API TArray<FLHQuestionAnswer> PrototypeAnswers();
LIGHTHAVEN_API FString CatalogHash();
}
