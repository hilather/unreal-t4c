#pragma once
#include "Subsystems/GameInstanceSubsystem.h"
#include "Framework/LHWave2Session.h"
#include "LHSessionSubsystem.generated.h"
UCLASS()
class LIGHTHAVEN_API ULHSessionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    TSharedPtr<FLHWave2Session> Session() const { return Live; }
private:
    TSharedPtr<FLHWave2Session> Live;
};
