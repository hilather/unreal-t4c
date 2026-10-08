#pragma once
#include "Commandlets/Commandlet.h"
#include "LHGenerateFrontendMapCommandlet.generated.h"
UCLASS()
class ULHGenerateFrontendMapCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    ULHGenerateFrontendMapCommandlet();
    virtual int32 Main(const FString& Params) override;
};
