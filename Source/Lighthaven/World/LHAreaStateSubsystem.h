#pragma once
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/LHSaveSnapshot.h"
#include "LHAreaStateSubsystem.generated.h"

namespace LHWorld
{
    // Fail closed: preserve caller data and diagnose rather than silently reset enemies/rewards.
    LIGHTHAVEN_API bool ValidateWorld(const FLHWorldRecord& World, FString& Error);
}
UCLASS()
class LIGHTHAVEN_API ULHAreaStateSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    bool Hydrate(const FLHWorldRecord& World, FString& Error);
    bool StoreArea(const FLHAreaRecord& Area, FString& Error);
    bool PersistInto(FLHWorldRecord& World, FString& Error) const;
    const FLHAreaRecord* Find(const FLHAreaId& Area) const;
    virtual void Deinitialize() override;
private:
    FLHWorldRecord State;
    bool bHydrated=false;
};
