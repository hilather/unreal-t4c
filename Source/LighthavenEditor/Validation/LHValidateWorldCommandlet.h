#pragma once
#include "Commandlets/Commandlet.h"
#include "LHValidateWorldCommandlet.generated.h"
UCLASS()
class ULHValidateWorldCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    ULHValidateWorldCommandlet();
    virtual int32 Main(const FString& Params) override;
};
