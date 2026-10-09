#pragma once
#include "Commandlets/Commandlet.h"
#include "LHGenerateHubMapCommandlet.generated.h"

UCLASS()
class ULHGenerateHubMapCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    ULHGenerateHubMapCommandlet();
    virtual int32 Main(const FString& Params) override;
};
