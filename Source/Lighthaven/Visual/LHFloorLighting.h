#pragma once
#include "CoreMinimal.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/Actor.h"
#include "Visual/LHVisualKit.h"
#include "LHFloorLighting.generated.h"

class UExponentialHeightFogComponent;
struct FPostProcessSettings;

namespace LHFloorLighting
{
    // Prototype presentation profiles. B1 retains its independent lighting path.
    struct FProfile
    {
        float EV100, SkyIntensity;
        FLinearColor SkyColor;
        float FixtureLumens, FixtureRadius, FixtureKelvin;
        float FillLumens, FillRadius, FillHeight;
        FLinearColor FillColor;
        float PlayerLumens;
        FLinearColor PlayerColor, HazeColor;
        float HazeOpacity;
    };
    LIGHTHAVEN_API const FProfile* FindProfile(ELHVisualStyle Style);
    LIGHTHAVEN_API void ConfigurePostProcess(FPostProcessSettings& Settings, ELHVisualStyle Style);
    // Reapplied after imported materials rebuild, including PostLoad/BeginPlay.
    LIGHTHAVEN_API void ConfigureChurchExterior(ALHVisualPiece* Piece);
}

// Low, local reflected glow at the existing imported floor inlays. Separate
// from fixture lights so audits can retain the original torch pool inventory.
UCLASS()
class LIGHTHAVEN_API ULHRitualSigilLightComponent : public UPointLightComponent
{
    GENERATED_BODY()
public:
    ULHRitualSigilLightComponent();
    static void Configure(ALHVisualPiece* Piece);
};

UCLASS()
class LIGHTHAVEN_API ULHFloorFixtureLightComponent : public UPointLightComponent
{
    GENERATED_BODY()
public:
    ULHFloorFixtureLightComponent();
    static ULHFloorFixtureLightComponent* Configure(ALHVisualPiece* Piece, FVector FlameLocal=FVector(0,25,14));
    void LogRuntimeState(const TCHAR* Stage) const;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
    float BaseLumens=0.f;
    uint32 PhaseSeed=0;
    bool bFlicker=false;
};

UCLASS()
class LIGHTHAVEN_API ALHFloorAtmosphere : public AActor
{
    GENERATED_BODY()
public:
    ALHFloorAtmosphere();
    void Configure(ELHVisualStyle InStyle);
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void PostLoad() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditAnywhere) ELHVisualStyle Style=ELHVisualStyle::Church;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UExponentialHeightFogComponent> Haze;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> ReadabilityFill;
};
