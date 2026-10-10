#pragma once
#include "CoreMinimal.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/Actor.h"
#include "LHB1Lighting.generated.h"
class ALHVisualPiece;
class UExponentialHeightFogComponent;

// Prototype B1 presentation, independent of gameplay transforms and save data.
UCLASS()
class LIGHTHAVEN_API ULHB1TorchLightComponent : public UPointLightComponent
{
    GENERATED_BODY()
public:
    ULHB1TorchLightComponent();
    static ULHB1TorchLightComponent* Configure(ALHVisualPiece* Piece, FVector FlameLocal = FVector(0,25,14));
    static float Flicker(double GameSeconds, uint32 Seed);
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
    uint32 PhaseSeed = 0;
};

UCLASS()
class LIGHTHAVEN_API ALHB1Atmosphere : public AActor
{
    GENERATED_BODY()
public:
    ALHB1Atmosphere();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UExponentialHeightFogComponent> Haze;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> ReadabilityFill;
};
