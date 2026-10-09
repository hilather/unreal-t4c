#pragma once
#include "Commandlets/Commandlet.h"
#include "LHGenerateBasementAMapsCommandlet.generated.h"

UCLASS()
class ULHGenerateBasementAMapsCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    ULHGenerateBasementAMapsCommandlet();
    virtual int32 Main(const FString& Params) override;
};
