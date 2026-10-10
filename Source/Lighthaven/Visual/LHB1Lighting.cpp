#include "Visual/LHB1Lighting.h"
#include "Visual/LHVisualKit.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

ULHB1TorchLightComponent::ULHB1TorchLightComponent()
{
    PrimaryComponentTick.bCanEverTick=true;
    SetMobility(EComponentMobility::Movable);
    SetCastShadows(false);
    SetIntensityUnits(ELightUnits::Lumens);
    SetUseTemperature(true); SetTemperature(2000.f);
    SetIntensity(1800.f); SetAttenuationRadius(700.f);
    bUseInverseSquaredFalloff=true;
    SourceRadius=12.f; SoftSourceRadius=20.f;
    SetCanEverAffectNavigation(false);
}
float ULHB1TorchLightComponent::Flicker(double GameSeconds, uint32 Seed)
{
    const double Phase=(Seed%4096)*2.0*PI/4096.0;
    return 1.f+.04f*FMath::Sin(GameSeconds*3.7+Phase)+.02f*FMath::Sin(GameSeconds*7.1+Phase*1.7);
}
ULHB1TorchLightComponent* ULHB1TorchLightComponent::Configure(ALHVisualPiece* Piece,FVector FlameLocal)
{
    if(!Piece) return nullptr;
    const auto& Recipe=Piece->GetRecipe();
    const FString Id=Recipe.Id.ToString();
    auto* Existing=Piece->FindComponentByClass<ULHB1TorchLightComponent>();
    if(Recipe.Style!=ELHVisualStyle::B1Cellar || !(Id.EndsWith(TEXT(".Torch")) || Id.EndsWith(TEXT(".Sconce"))))
    { if(Existing) Existing->DestroyComponent(); return nullptr; }
    auto* Light=Existing?Existing:NewObject<ULHB1TorchLightComponent>(Piece,NAME_None,RF_Transient);
    if(!Existing)
    {
        Piece->AddInstanceComponent(Light);
        Light->SetupAttachment(Piece->GetRootComponent());
        Light->RegisterComponent();
    }
    Light->SetRelativeLocation(FlameLocal);
    // Position is stable across loads; actor names may change in PIE or cooking.
    const FVector P=Piece->GetActorLocation();
    Light->PhaseSeed=HashCombine(HashCombine(GetTypeHash(FMath::RoundToInt(P.X)),GetTypeHash(FMath::RoundToInt(P.Y))),GetTypeHash(FMath::RoundToInt(P.Z)));
    return Light;
}
void ULHB1TorchLightComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
    if(GetWorld()) SetIntensity(1800.f*Flicker(GetWorld()->GetTimeSeconds(),PhaseSeed));
}
ALHB1Atmosphere::ALHB1Atmosphere()
{
    PrimaryActorTick.bCanEverTick=true;
    Haze=CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("B1Haze"));
    SetRootComponent(Haze);
    Haze->SetMobility(EComponentMobility::Movable);
    Haze->FogDensity=.008f; Haze->FogHeightFalloff=.25f; Haze->FogMaxOpacity=.12f;
    // A faint blue-grey distance floor also softens the black cutaway void.
    // Prototype radiance, capped to 12% fog opacity; host capture must verify luma.
    Haze->FogInscatteringLuminance=FLinearColor(.10f,.14f,.20f);
    Haze->bEnableVolumetricFog=false;
    ReadabilityFill=CreateDefaultSubobject<UPointLightComponent>(TEXT("B1PlayerReadability"));
    ReadabilityFill->SetupAttachment(Haze);
    ReadabilityFill->SetMobility(EComponentMobility::Movable);
    ReadabilityFill->SetIntensityUnits(ELightUnits::Lumens);
    ReadabilityFill->SetIntensity(450.f); ReadabilityFill->SetAttenuationRadius(750.f);
    ReadabilityFill->SetLightColor(FLinearColor(.55f,.7f,1.f));
    ReadabilityFill->SetCastShadows(false);
    ReadabilityFill->bUseInverseSquaredFalloff=true;
    ReadabilityFill->SetVisibility(false);
}
void ALHB1Atmosphere::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const auto* PC=GetWorld()?GetWorld()->GetFirstPlayerController():nullptr;
    const APawn* Pawn=PC?PC->GetPawn():nullptr;
    ReadabilityFill->SetVisibility(Pawn!=nullptr);
    if(Pawn) ReadabilityFill->SetWorldLocation(Pawn->GetActorLocation()+FVector(0,0,220));
}
