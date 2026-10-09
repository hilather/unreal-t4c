#pragma once
#include "Commandlets/Commandlet.h"
#include "LHGenerateBasementBMapsCommandlet.generated.h"

UCLASS()
class ULHGenerateBasementBMapsCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    ULHGenerateBasementBMapsCommandlet();
    virtual int32 Main(const FString& Params) override;
};
