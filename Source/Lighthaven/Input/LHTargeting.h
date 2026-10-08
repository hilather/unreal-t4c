#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Input/LHInputConfig.h"
#include "LHTargeting.generated.h"
// Provider must use authoritative life state and navigation reachability; unknown means false.
UINTERFACE(BlueprintType)
class LIGHTHAVEN_API ULHControlTarget : public UInterface
{
    GENERATED_BODY()
};
class LIGHTHAVEN_API ILHControlTarget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable) bool IsControlTargetAlive() const;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable) bool IsControlTargetReachable(AActor* Avatar) const;
};
struct FLHTargetCandidate
{
    FString Key;
    double DistanceSquared=0;
    bool bAlive=false, bReachable=false, bVisible=false;
};
namespace LHControls
{
    LIGHTHAVEN_API TArray<FString> OrderedTargets(const TArray<FLHTargetCandidate>& Candidates, double MaxDistanceSquared);
    LIGHTHAVEN_API FString CycleTarget(const TArray<FString>& Ordered, const FString& Current, int32 Direction);
    struct LIGHTHAVEN_API FMovementState
    {
        FVector2D Held=FVector2D::ZeroVector;
        ELHInputContext Context=ELHInputContext::Gameplay;
        void Clear() { Held=FVector2D::ZeroVector; }
        void SwitchContext(ELHInputContext Next) { Clear(); Context=Next; }
    };
}
