#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/LHIdentity.h"
#include "LHGameState.generated.h"
UCLASS()
class LIGHTHAVEN_API ALHGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, Category="Session") FLHAreaId CurrentArea;
};
