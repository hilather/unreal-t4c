#pragma once

#include "Commandlets/Commandlet.h"
#include "LHGenerateDevMapsCommandlet.generated.h"

UCLASS()
class ULHGenerateDevMapsCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    ULHGenerateDevMapsCommandlet();
    virtual int32 Main(const FString& Params) override;
};
