#include "Visual/LHB1Lighting.h"
#include "Visual/LHVisualKit.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogLHB1Lighting, Log, All);

ULHB1TorchLightComponent::ULHB1TorchLightComponent()
{
    PrimaryComponentTick.bCanEverTick=true;
    SetMobility(EComponentMobility::Movable);
    SetCastShadows(false);
    SetIntensityUnits(ELightUnits::Lumens);
    SetUseTemperature(true); SetTemperature(2900.f);
    SetIntensity(1000.f); SetAttenuationRadius(375.f);
    bUseInverseSquaredFalloff=true;
    SourceRadius=12.f; SoftSourceRadius=20.f;
    SetCanEverAffectNavigation(false);
}
float ULHB1TorchLightComponent::Flicker(double GameSeconds, uint32 Seed)
{
    const double Phase=(Seed%4096)*2.0*PI/4096.0;
    return 1.f+.04f*FMath::Sin(GameSeconds*3.7+Phase)+.02f*FMath::Sin(GameSeconds*7.1+Phase*1.7);
}
void ULHB1TorchLightComponent::LogRuntimeState(const TCHAR* Stage) const
{
    const UWorld* World=GetWorld();
    if(!World || !World->IsGameWorld()) return;
    const AActor* Owner=GetOwner();
    UE_LOG(LogLHB1Lighting, Display,
        TEXT("B1Torch stage=%s owner=%s world=%s game=%d initialized=%d scene=%d registered=%d renderState=%d renderDirty=%d actorPos=%s worldPos=%s relativePos=%s intensity=%.3f units=%s radius=%.3f visible=%d hidden=%d ownerHidden=%d mobility=%s affectsWorld=%d shadows=%d inverseSquared=%d maxDrawDistance=%.3f fadeRange=%.3f"),
        Stage,*GetNameSafe(Owner),*GetNameSafe(World),World && World->IsGameWorld(),
        World->IsInitialized(),World->Scene!=nullptr,
        IsRegistered(),IsRenderStateCreated(),IsRenderStateDirty(),
        *(Owner?Owner->GetActorLocation():FVector::ZeroVector).ToString(),
        *GetComponentLocation().ToString(),*GetRelativeLocation().ToString(),Intensity,
        *UEnum::GetValueAsString(IntensityUnits),AttenuationRadius,IsVisible(),bHiddenInGame,
        Owner && Owner->IsHidden(),*UEnum::GetValueAsString(GetMobility()),bAffectsWorld,
        CastShadows,bUseInverseSquaredFalloff,MaxDrawDistance,MaxDistanceFadeRange);
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
    }
    Light->SetRelativeLocation(FlameLocal);
    // Position is stable across loads; actor names may change in PIE or cooking.
    const FVector P=Piece->GetActorLocation();
    Light->PhaseSeed=HashCombine(HashCombine(GetTypeHash(FMath::RoundToInt(P.X)),GetTypeHash(FMath::RoundToInt(P.Y))),GetTypeHash(FMath::RoundToInt(P.Z)));
    // PostLoad runs before InitWorld allocates the scene. Registering then marks
    // the component registered without creating its render state, so normal
    // actor registration skips it later. Leave it for that normal lifecycle;
    // Build/BeginPlay also retry existing lights once the world is ready.
    if(UWorld* World=Piece->GetWorld(); World && World->IsInitialized() && !Light->IsRegistered())
        Light->RegisterComponent();
    if(!Existing) Light->LogRuntimeState(TEXT("ConfigureCreated"));
    return Light;
}
void ULHB1TorchLightComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
    if(GetWorld()) SetIntensity(1000.f*Flicker(GetWorld()->GetTimeSeconds(),PhaseSeed));
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
    // Four high, broad bounce approximations lift unoccupied gaps as well as
    // the player's surroundings. Keep the short warm torch falloff unchanged.
    // Prototype photometry: 14000 lm at 9m gives ~13 lux directly below, before
    // tint; the fixed room grids audit the dimmer edges and pool/gap contrast.
    const FVector Centers[]={{900,-1900,900},{900,900,900},{-2100,1000,900},{4200,1300,900}};
    for(int32 I=0;I<UE_ARRAY_COUNT(Centers);++I)
    {
        auto* Fill=CreateDefaultSubobject<UPointLightComponent>(*FString::Printf(TEXT("B1RoomBounce%d"),I));
        Fill->SetupAttachment(Haze);
        Fill->SetRelativeLocation(Centers[I]);
        Fill->SetMobility(EComponentMobility::Movable);
        Fill->SetIntensityUnits(ELightUnits::Lumens);
        Fill->SetIntensity(14000.f); Fill->SetAttenuationRadius(3000.f);
        Fill->SetLightColor(FLinearColor(.65f,.70f,.80f));
        Fill->SetCastShadows(false); Fill->bUseInverseSquaredFalloff=true;
        Fill->SetCanEverAffectNavigation(false);
        RoomFills.Add(Fill);
    }
}
void ALHB1Atmosphere::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const auto* PC=GetWorld()?GetWorld()->GetFirstPlayerController():nullptr;
    const APawn* Pawn=PC?PC->GetPawn():nullptr;
    ReadabilityFill->SetVisibility(Pawn!=nullptr);
    if(Pawn) ReadabilityFill->SetWorldLocation(Pawn->GetActorLocation()+FVector(0,0,220));
}
