#include "Visual/LHFloorLighting.h"
#include "Visual/LHB1Lighting.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/Scene.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogLHFloorLighting, Log, All);

const LHFloorLighting::FProfile* LHFloorLighting::FindProfile(ELHVisualStyle Style)
{
    static const FProfile Church={4.f,.2f,{.65f,.75f,1.f},2400.f,500.f,2800.f,20000.f,1800.f,650.f,{1.f,.78f,.52f},900.f,{1.f,.82f,.62f},{.18f,.145f,.10f},.10f};
    static const FProfile B2={2.5f,.16f,{.56f,.76f,.72f},1000.f,400.f,2900.f,6500.f,1500.f,800.f,{.55f,.76f,.72f},450.f,{.68f,.85f,.82f},{.08f,.14f,.13f},.12f};
    static const FProfile B3={4.f,.12f,{.62f,.72f,1.f},900.f,375.f,3100.f,3500.f,1600.f,750.f,{.63f,.74f,1.f},600.f,{.76f,.83f,1.f},{1.4f,1.8f,2.5f},.12f};
    static const FProfile B4={2.25f,.10f,{.62f,.67f,.83f},1800.f,375.f,2300.f,6500.f,1400.f,750.f,{.62f,.44f,.38f},650.f,{.9f,.7f,.52f},{.16f,.095f,.065f},.15f};
    switch(Style)
    {
    case ELHVisualStyle::Church: return &Church;
    case ELHVisualStyle::B2Damp: return &B2;
    case ELHVisualStyle::B3Crypt: return &B3;
    case ELHVisualStyle::B4Ritual: return &B4;
    default: return nullptr;
    }
}
void LHFloorLighting::ConfigurePostProcess(FPostProcessSettings& S,ELHVisualStyle Style)
{
    const FProfile* P=FindProfile(Style); if(!P) return;
    S.bOverride_AutoExposureMethod=true; S.AutoExposureMethod=EAutoExposureMethod::AEM_Manual;
    S.bOverride_AutoExposureApplyPhysicalCameraExposure=true; S.AutoExposureApplyPhysicalCameraExposure=true;
    S.bOverride_CameraISO=true; S.CameraISO=100.f;
    S.bOverride_DepthOfFieldFstop=true; S.DepthOfFieldFstop=2.f;
    S.bOverride_CameraShutterSpeed=true; S.CameraShutterSpeed=FMath::Pow(2.f,P->EV100)/4.f;
    S.bOverride_AutoExposureBias=true; S.AutoExposureBias=0.f;
    S.bOverride_AutoExposureMinBrightness=true; S.AutoExposureMinBrightness=P->EV100;
    S.bOverride_AutoExposureMaxBrightness=true; S.AutoExposureMaxBrightness=P->EV100;
    S.bOverride_DynamicGlobalIlluminationMethod=true; S.DynamicGlobalIlluminationMethod=EDynamicGlobalIlluminationMethod::None;
    S.bOverride_ReflectionMethod=true; S.ReflectionMethod=EReflectionMethod::None;
    S.bOverride_AmbientOcclusionIntensity=true;
    S.AmbientOcclusionIntensity=Style==ELHVisualStyle::B3Crypt?.50f:(Style==ELHVisualStyle::B2Damp?.65f:.55f);
    S.bOverride_AmbientOcclusionRadius=true; S.AmbientOcclusionRadius=100.f;
    S.bOverride_AmbientOcclusionRadiusInWS=true; S.AmbientOcclusionRadiusInWS=true;
    S.bOverride_MotionBlurAmount=true; S.MotionBlurAmount=0.f;
    S.bOverride_BloomIntensity=true; S.BloomIntensity=.18f;
    S.bOverride_BloomThreshold=true; S.BloomThreshold=1.f;
    S.bOverride_ColorSaturation=true; S.ColorSaturation=FVector4(.93f,.93f,.93f,1.f);
    S.bOverride_VignetteIntensity=true; S.VignetteIntensity=.16f;
    S.bOverride_ColorGainHighlights=true;
    S.bOverride_ColorGainShadows=true;
    switch(Style)
    {
    case ELHVisualStyle::Church:
        S.ColorGainHighlights=FVector4(1.02f,1.f,.98f,1.f); S.ColorGainShadows=FVector4(1.f,1.01f,1.03f,1.f); break;
    case ELHVisualStyle::B2Damp:
        S.ColorGainHighlights=FVector4(.99f,1.02f,1.01f,1.f); S.ColorGainShadows=FVector4(.98f,1.02f,1.01f,1.f); break;
    case ELHVisualStyle::B3Crypt:
        S.ColorGainHighlights=FVector4(1.f,1.01f,1.03f,1.f); S.ColorGainShadows=FVector4(.99f,1.f,1.03f,1.f); break;
    case ELHVisualStyle::B4Ritual:
        S.ColorGainHighlights=FVector4(1.03f,1.f,.98f,1.f); S.ColorGainShadows=FVector4(.88f,.92f,1.02f,1.f); break;
    default: break;
    }
}
ULHFloorFixtureLightComponent::ULHFloorFixtureLightComponent()
{
    PrimaryComponentTick.bCanEverTick=true;
    SetMobility(EComponentMobility::Movable);
    SetCastShadows(false); SetIntensityUnits(ELightUnits::Lumens);
    bUseInverseSquaredFalloff=true; SourceRadius=12.f; SoftSourceRadius=20.f;
    SetCanEverAffectNavigation(false);
}
ULHFloorFixtureLightComponent* ULHFloorFixtureLightComponent::Configure(ALHVisualPiece* Piece,FVector FlameLocal)
{
    if(!Piece) return nullptr;
    const auto& R=Piece->GetRecipe(); const auto* P=LHFloorLighting::FindProfile(R.Style);
    const FString Id=R.Id.ToString();
    const bool Fixture=Id.EndsWith(TEXT(".Torch")) || Id.EndsWith(TEXT(".Sconce"));
    const bool Altar=Id.EndsWith(TEXT(".Altar")) && (R.Style==ELHVisualStyle::Church || R.Style==ELHVisualStyle::B4Ritual);
    auto* Existing=Piece->FindComponentByClass<ULHFloorFixtureLightComponent>();
    if(!P || (!Fixture && !Altar)) { if(Existing) Existing->DestroyComponent(); return nullptr; }
    auto* Light=Existing?Existing:NewObject<ULHFloorFixtureLightComponent>(Piece,NAME_None,RF_Transient);
    if(!Existing) { Piece->AddInstanceComponent(Light); Light->SetupAttachment(Piece->GetRootComponent()); }
    Light->bFlicker=Fixture;
    Light->BaseLumens=Altar?(R.Style==ELHVisualStyle::Church?4000.f:3000.f):P->FixtureLumens;
    Light->SetIntensity(Light->BaseLumens);
    Light->SetAttenuationRadius(Altar?(R.Style==ELHVisualStyle::Church?700.f:500.f):P->FixtureRadius);
    Light->SetRelativeLocation(Altar?FVector(0,0,180):FlameLocal);
    Light->SetUseTemperature(!(Altar && R.Style==ELHVisualStyle::B4Ritual));
    Light->SetTemperature(P->FixtureKelvin);
    Light->SetLightColor(Altar && R.Style==ELHVisualStyle::B4Ritual?FLinearColor(1.f,.3f,.1f):FLinearColor::White);
    Light->SetComponentTickEnabled(Fixture);
    const FVector Pos=Piece->GetActorLocation();
    Light->PhaseSeed=HashCombine(HashCombine(GetTypeHash(FMath::RoundToInt(Pos.X)),GetTypeHash(FMath::RoundToInt(Pos.Y))),GetTypeHash(FMath::RoundToInt(Pos.Z)));
    // Defer until InitWorld creates the scene; normal actor registration also handles this component.
    if(UWorld* World=Piece->GetWorld(); World && World->IsInitialized() && !Light->IsRegistered()) Light->RegisterComponent();
    if(!Existing) Light->LogRuntimeState(TEXT("ConfigureCreated"));
    return Light;
}
void ULHFloorFixtureLightComponent::LogRuntimeState(const TCHAR* Stage) const
{
    const UWorld* World=GetWorld(); if(!World || !World->IsGameWorld()) return;
    UE_LOG(LogLHFloorLighting,Display,TEXT("FloorFixture stage=%s owner=%s initialized=%d scene=%d registered=%d renderState=%d intensity=%.1f radius=%.1f visible=%d position=%s"),
        Stage,*GetNameSafe(GetOwner()),World->IsInitialized(),World->Scene!=nullptr,IsRegistered(),IsRenderStateCreated(),Intensity,AttenuationRadius,IsVisible(),*GetComponentLocation().ToString());
}
void ULHFloorFixtureLightComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* TickFunction)
{
    Super::TickComponent(DeltaTime,TickType,TickFunction);
    if(bFlicker && GetWorld()) SetIntensity(BaseLumens*ULHB1TorchLightComponent::Flicker(GetWorld()->GetTimeSeconds(),PhaseSeed));
}
ALHFloorAtmosphere::ALHFloorAtmosphere()
{
    PrimaryActorTick.bCanEverTick=true;
    Haze=CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("FloorHaze")); SetRootComponent(Haze);
    Haze->SetMobility(EComponentMobility::Movable); Haze->SetCanEverAffectNavigation(false);
    ReadabilityFill=CreateDefaultSubobject<UPointLightComponent>(TEXT("FloorPlayerReadability"));
    ReadabilityFill->SetupAttachment(Haze); ReadabilityFill->SetMobility(EComponentMobility::Movable);
    ReadabilityFill->SetIntensityUnits(ELightUnits::Lumens); ReadabilityFill->SetCastShadows(false);
    ReadabilityFill->bUseInverseSquaredFalloff=true; ReadabilityFill->SetCanEverAffectNavigation(false);
    Configure(Style);
}
void ALHFloorAtmosphere::Configure(ELHVisualStyle InStyle)
{
    Style=InStyle; const auto* P=LHFloorLighting::FindProfile(Style);
    Haze->SetVisibility(P!=nullptr); ReadabilityFill->SetVisibility(false);
    if(!P) return;
    Haze->FogDensity=.008f; Haze->FogHeightFalloff=.25f; Haze->FogMaxOpacity=P->HazeOpacity;
    Haze->FogInscatteringLuminance=P->HazeColor; Haze->bEnableVolumetricFog=false;
    // Prototype depth-targeted crypt haze: keep nearby gameplay receivers out
    // of fog, tint distant cutaway void. HeightFogCommon.ush integrates along
    // the depth ray and caps opacity; HeightFogPixelShader.usf applies exposure.
    Haze->SetStartDistance(Style==ELHVisualStyle::B3Crypt?4000.f:0.f);
    Haze->MarkRenderStateDirty();
    ReadabilityFill->SetIntensity(P->PlayerLumens); ReadabilityFill->SetAttenuationRadius(750.f);
    ReadabilityFill->SetUseTemperature(false); ReadabilityFill->SetLightColor(P->PlayerColor);
}
void ALHFloorAtmosphere::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform); Configure(Style);
}
void ALHFloorAtmosphere::PostLoad()
{
    Super::PostLoad(); Configure(Style);
}
void ALHFloorAtmosphere::BeginPlay()
{
    Super::BeginPlay(); Configure(Style);
    UE_LOG(LogLHFloorLighting,Display,TEXT("FloorAtmosphere stage=BeginPlay owner=%s style=%s registered=%d playerLumens=%.1f hazeOpacity=%.3f"),
        *GetName(),*UEnum::GetValueAsString(Style),ReadabilityFill->IsRegistered(),ReadabilityFill->Intensity,Haze->FogMaxOpacity);
}
void ALHFloorAtmosphere::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const auto* PC=GetWorld()?GetWorld()->GetFirstPlayerController():nullptr;
    const APawn* Pawn=PC?PC->GetPawn():nullptr;
    ReadabilityFill->SetVisibility(Pawn && LHFloorLighting::FindProfile(Style));
    if(Pawn) ReadabilityFill->SetWorldLocation(Pawn->GetActorLocation()+FVector(0,0,220));
}
