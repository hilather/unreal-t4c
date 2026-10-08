#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LHInputConfig.generated.h"
class UInputAction;
class UInputMappingContext;
UENUM(BlueprintType)
enum class ELHInputContext : uint8 { Gameplay, UI, Creation };
UCLASS()
class LIGHTHAVEN_API ULHInputConfig : public UObject
{
    GENERATED_BODY()
public:
    void Initialize();
    UInputAction* Action(FName Name) const;
    UInputMappingContext* Context(ELHInputContext Mode) const;
    bool HasDeviceParity(FName Name) const;
    static const TArray<FName>& GameplayActions();
private:
    UPROPERTY() TMap<FName, TObjectPtr<UInputAction>> Actions;
    UPROPERTY() TArray<TObjectPtr<UInputMappingContext>> Contexts;
};
