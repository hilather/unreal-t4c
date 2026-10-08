#pragma once
#include "Character/LHCharacterAuthority.h"
namespace LHWave2
{
LIGHTHAVEN_API FLHCharacterProfile PrototypeProfile();
LIGHTHAVEN_API FLHNumber PrototypeNumber(double V);
LIGHTHAVEN_API FLHInteger PrototypeInteger(int64 V);
LIGHTHAVEN_API TArray<FLHQuestionAnswer> PrototypeAnswers();
LIGHTHAVEN_API FString CatalogHash();
}
