#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LHGameMode.generated.h"
UCLASS()
class LIGHTHAVEN_API ALHGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ALHGameMode();
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
#endif
};
