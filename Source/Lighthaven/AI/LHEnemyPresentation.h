#pragma once
#include "CoreMinimal.h"
class AActor;
class USceneComponent;
namespace LHEnemyPresentation
{
LIGHTHAVEN_API bool Known(FName Id);
LIGHTHAVEN_API void Build(AActor* Owner, USceneComponent* Root, FName Id);
LIGHTHAVEN_API void Dead(USceneComponent* Root, FName Id);
}
