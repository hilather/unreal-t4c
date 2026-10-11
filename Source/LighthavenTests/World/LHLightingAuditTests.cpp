#include "Misc/AutomationTest.h"
#include "Visual/LHB1Lighting.h"
#include "Visual/LHVisualKit.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "World/LHAreaRegistry.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"
#include "Components/LightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "UObject/Package.h"
#include "UObject/LinkerInstancingContext.h"
#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "Misc/App.h"
#include "Framework/LHCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Visual/LHB1ArtBinding.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHLightingAuditTest,"Lighthaven.World.LightingAudit",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHLightingAuditTest::RunTest(const FString& Parameters)
{
    int32 Index=0;
    const int32 LandmarkLocal[]={20,12,17,17,13}; // Existing landmark inventory remains intact.
    for(const auto& Area:LHWorld::Registry())
    {
        UPackage* Package=LoadPackage(nullptr,*Area.Map.GetLongPackageName(),LOAD_None);
        UWorld* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
        if(!TestNotNull(*Area.Map.ToString(),World)) { ++Index; continue; }
        int32 Local=0,Sun=0,Sky=0,Exposure=0,Coverage=0,Atmosphere=0,Fixtures=0;
        TArray<FVector> CoveragePositions;
        for(TActorIterator<AActor> It(World);It;++It)
        {
            if(auto* A=Cast<ALHB1Atmosphere>(*It))
            {
                ++Atmosphere;
                TestEqual(TEXT("B1 haze density"),A->Haze->FogDensity,.008f);
                TestFalse(TEXT("B1 no volumetric fog"),A->Haze->bEnableVolumetricFog);
                TestEqual(TEXT("B1 haze bounded opacity"),A->Haze->FogMaxOpacity,.12f);
                TestEqual(TEXT("B1 warm haze radiance"),A->Haze->FogInscatteringLuminance,FLinearColor(.18f,.125f,.075f));
            }
            if(auto* Piece=Cast<ALHVisualPiece>(*It))
            {
                const auto& R=Piece->GetRecipe(); const FString Id=R.Id.ToString();
                const bool Eligible=Index==1 && R.Style==ELHVisualStyle::B1Cellar &&
                    (Id.EndsWith(TEXT(".Torch")) || Id.EndsWith(TEXT(".Sconce")));
                TArray<ULHB1TorchLightComponent*> TorchLights; Piece->GetComponents(TorchLights);
                TestEqual(TEXT("Exactly one light per B1 fixture, zero elsewhere"),TorchLights.Num(),Eligible?1:0);
                if(Eligible) ++Fixtures;
            }
            TArray<ULightComponent*> Lights; It->GetComponents(Lights);
            for(auto* C:Lights)
            {
                TestEqual(TEXT("No unbuilt static/stationary light"),C->Mobility,EComponentMobility::Movable);
                if(auto* P=Cast<UPointLightComponent>(C))
                {
                    ++Local;
                    const bool bCoverage=It->GetName().StartsWith(TEXT("Coverage_")) || (It->GetName().StartsWith(TEXT("A04_T1")) && It->GetName().Len()==9);
                    if(bCoverage) { ++Coverage; CoveragePositions.Add(It->GetActorLocation()); }
                    TArray<UPrimitiveComponent*> Primitives; It->GetComponents(Primitives);
                    for(auto* Primitive:Primitives) TestEqual(TEXT("Light has no blocking collision"),Primitive->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
                    TestEqual(TEXT("Local units lumens"),P->IntensityUnits,ELightUnits::Lumens);
                    if(Index==1)
                    {
                        TestFalse(TEXT("B1 no legacy grid light"),It->GetName().StartsWith(TEXT("A04_T")));
                        TestFalse(TEXT("B1 point lights cast no shadows"),P->CastShadows);
                        const bool Bounce=P->GetName().StartsWith(TEXT("B1RoomBounce"));
                        TestTrue(TEXT("B1 bounded pool or room-fill radius"),P->AttenuationRadius<=(Bounce?3000.f:750.f));
                        TestTrue(TEXT("B1 pool or room-fill intensity"),Bounce?P->Intensity==14000.f:(P->Intensity>=450.f && P->Intensity<=1908.f));
                    }
                    else TestTrue(TEXT("Local intensity Prototype 600..4000 lm"),P->Intensity>=600 && P->Intensity<=4000);
                    if(Index>1) TestTrue(TEXT("Interior local radius at least 1400cm"),P->AttenuationRadius>=1400);
                }
                else if(Cast<UDirectionalLightComponent>(C)) { ++Sun; TestEqual(TEXT("Sun 3000 lux"),C->Intensity,3000.f); }
            }
            TArray<USkyLightComponent*> Skies; It->GetComponents(Skies);
            for(auto* C:Skies)
            {
                ++Sky; TestEqual(TEXT("Sky movable"),C->Mobility,EComponentMobility::Movable);
                TestEqual(TEXT("Prototype neutral sky intensity"),C->Intensity,Index==0?.7f:(Index==1?.12f:.8f));
                TestFalse(TEXT("Fill casts no shadows"),C->CastShadows);
                if(Index>0) TestFalse(TEXT("Interior lower hemisphere fill"),C->bLowerHemisphereIsBlack);
                TestEqual(TEXT("Specified sky cube"),C->SourceType,SLS_SpecifiedCubemap);
                TestNotNull(TEXT("Sky cube must cook"),C->Cubemap.Get());
            }
            if(auto* PP=Cast<APostProcessVolume>(*It))
            {
                ++Exposure; const auto& S=PP->Settings;
                if(Index==1)
                {
                    TestTrue(TEXT("B1 AO"),S.bOverride_AmbientOcclusionIntensity && S.AmbientOcclusionIntensity==.65f);
                    TestTrue(TEXT("B1 AO world radius"),S.bOverride_AmbientOcclusionRadius && S.AmbientOcclusionRadius==120.f && S.bOverride_AmbientOcclusionRadiusInWS && S.AmbientOcclusionRadiusInWS);
                    TestTrue(TEXT("B1 grade"),S.bOverride_ColorSaturation && S.ColorSaturation==FVector4(.95f,.95f,.95f,1.f) && S.bOverride_ColorGainHighlights && S.ColorGainHighlights==FVector4(1.04f,1.01f,.96f,1.f) && S.bOverride_ColorGainShadows && S.ColorGainShadows==FVector4(1.035f,1.f,.94f,1.f));
                    TestTrue(TEXT("B1 flame bloom"),S.bOverride_BloomIntensity && S.BloomIntensity==.25f);
                    TestTrue(TEXT("B1 vignette"),S.bOverride_VignetteIntensity && S.VignetteIntensity==.2f);
                }
                TestTrue(TEXT("Unbound exposure"),PP->bUnbound);
                TestTrue(TEXT("Manual override"),S.bOverride_AutoExposureMethod && S.AutoExposureMethod==AEM_Manual);
                TestTrue(TEXT("Physical override"),S.bOverride_AutoExposureApplyPhysicalCameraExposure && S.AutoExposureApplyPhysicalCameraExposure);
                const float EV=Index==0?10.f:(Index>=3?4.f:2.5f);
                TestTrue(TEXT("Physical camera EV"),S.bOverride_CameraISO && S.bOverride_CameraShutterSpeed && S.bOverride_DepthOfFieldFstop && FMath::IsNearlyEqual(FMath::Log2(FMath::Square(S.DepthOfFieldFstop)*S.CameraShutterSpeed*100/S.CameraISO),EV));
                TestTrue(TEXT("Fixed bounds"),S.bOverride_AutoExposureMinBrightness && S.bOverride_AutoExposureMaxBrightness && S.AutoExposureMinBrightness==EV && S.AutoExposureMaxBrightness==EV);
                TestTrue(TEXT("Zero bias"),S.bOverride_AutoExposureBias && S.AutoExposureBias==0);
            }
        }
        if(Index==1)
        {
            TestEqual(TEXT("One B1 atmosphere"),Atmosphere,1);
            TestEqual(TEXT("No B1 grid lights"),Coverage,0);
            // Relocated landmarks and coverage samples are thinned where pools overlap.
            TestTrue(TEXT("B1 fixtures thinned after wall preference"),Fixtures>=30 && Fixtures<51);
            TestEqual(TEXT("B1 fixtures plus player and four room fills"),Local,Fixtures+5);
        }
        else
        {
            TestEqual(TEXT("No B1 atmosphere on other floors"),Atmosphere,0);
            TestEqual(TEXT("Landmark light inventory retained"),Local-Coverage,LandmarkLocal[Index]);
        }
        if(Index>1)
        {
            TestTrue(TEXT("Interior coverage lights present"),Coverage>=10);
            // Every canonical encounter and entrance needs nearby fill, independently of inventory.
            for(const auto& Spawn:Area.Spawns)
                TestTrue(*FString::Printf(TEXT("Fill reaches encounter %s"),*Spawn.Alias.ToString()),
                    CoveragePositions.ContainsByPredicate([&](const FVector& P){return FVector::DistSquared2D(P,Spawn.Anchor.GetLocation())<=FMath::Square(1400.); }));
            for(const auto& Entry:Area.Entrances)
                TestTrue(*FString::Printf(TEXT("Fill reaches entrance %s"),*Entry.Id.LocalId.ToString()),
                    CoveragePositions.ContainsByPredicate([&](const FVector& P){return FVector::DistSquared2D(P,Entry.SafeTransform.GetLocation())<=FMath::Square(1400.); }));
        }
        TestEqual(TEXT("Directional inventory"),Sun,Index==0?1:0);
        TestEqual(TEXT("One sky"),Sky,1); TestEqual(TEXT("One exposure"),Exposure,1);
        ++Index;
    }
    TestEqual(TEXT("Five maps audited"),Index,5);
    return true;
}

namespace LHB1LoadedLightingAuditPrivate
{
// UE5.8 PointLightComponent.cpp: lumens -> cm^2 candela = lm*10000/(4*pi).
// DeferredLightingCommon.ush:260 radius window (1-(d^2/r^2)^2)^2.
// GetCapsule:309 sets distance bias 1; CapsuleLightIntegrate.ush:56 gives
// 1/(d^2+1) for zero-length point lights (sphere horizon wrap excluded).
// Unoccluded point-source photometric estimate, before temperature/tint, finite-source
// integration, material, skylight, fog, exposure and tonemapping. This is not pixel luma.
double DirectLux(FVector Light,FVector Surface,FVector Normal,double Lumens,double Radius)
{
    const FVector Delta=Light-Surface;
    const double D2=Delta.SizeSquared();
    const double Window=FMath::Square(FMath::Max(0.,1.-FMath::Square(D2/FMath::Square(Radius))));
    return Lumens*10000./(4.*PI)*Window/(D2+1.)*
        FMath::Max(0.,FVector::DotProduct(Delta.GetSafeNormal(),Normal));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHB1LoadedGameLightingTest,"Lighthaven.World.B1LoadedGameLightingAudit",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHB1LoadedGameLightingTest::RunTest(const FString& Parameters)
{
    using namespace LHB1LoadedLightingAuditPrivate;
    const FString Map=LHWorld::Registry()[1].Map.GetLongPackageName();
    const FString Instance=TEXT("/Temp/B1LoadedLighting_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    FLinkerInstancingContext Instancing;
    Instancing.AddPackageMapping(FName(*Map),FName(*Instance));
    // Read serialized recipes afresh: duplicating a previously rebuilt editor map
    // can conceal a PostLoad registration defect with copied transient components.
    UWorld::WorldTypePreLoadMap.Add(FName(*Instance),EWorldType::Game);
    UPackage* Package=LoadPackage(CreatePackage(*Instance),*Map,LOAD_None,nullptr,&Instancing);
    UWorld::WorldTypePreLoadMap.Remove(FName(*Instance));
    UWorld* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
    if(!TestNotNull(TEXT("Fresh serialized B1 world"),World)) return false;
    TestEqual(TEXT("Loaded as game world, not editor world"),World->WorldType,EWorldType::Game);
    TestFalse(TEXT("Serialized game world starts uninitialized"),World->IsInitialized());
    TestTrue(TEXT("Serialized game world has no scene before InitWorld"),World->Scene==nullptr);
    int32 PreInitFixtureLights=0;
    // TActorIterator filters actors in uninitialized levels; inspect serialized
    // PersistentLevel actors directly so pre-scene registration is observable.
    if(TestNotNull(TEXT("Serialized persistent level"),World->PersistentLevel.Get()))
    {
        for(AActor* Actor:World->PersistentLevel->Actors)
        {
            auto* Piece=Cast<ALHVisualPiece>(Actor);
            if(!Piece) continue;
            TArray<ULHB1TorchLightComponent*> Lights; Piece->GetComponents(Lights);
            for(const auto* Light:Lights)
            {
                ++PreInitFixtureLights;
                TestFalse(*FString::Printf(TEXT("%s never registered before game scene exists"),*Piece->GetName()),Light->IsRegistered());
            }
        }
    }
    TestTrue(TEXT("Pre-init assertions examine thinned serialized fixture set"),PreInitFixtureLights>=30 && PreInitFixtureLights<51);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    if(!World->IsInitialized()) World->InitWorld(UWorld::InitializationValues().AllowAudioPlayback(false)
        .CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false));
    World->UpdateWorldComponents(true,false);
    World->InitializeActorsForPlay(FURL());
    // Exercise real actor BeginPlay without starting gameplay/session or spawning a pawn.
    for(TActorIterator<AActor> It(World);It;++It) if(!It->HasActorBegunPlay()) It->DispatchBeginPlay();
    TArray<ULHB1TorchLightComponent*> Fixtures;
    for(TActorIterator<ALHVisualPiece> It(World);It;++It)
    {
        const auto& Recipe=It->GetRecipe();
        const FString Id=Recipe.Id.ToString();
        if(Recipe.Style!=ELHVisualStyle::B1Cellar || !(Id.EndsWith(TEXT(".Torch")) || Id.EndsWith(TEXT(".Sconce")))) continue;
        TArray<ULHB1TorchLightComponent*> Lights; It->GetComponents(Lights);
        const FString Label=It->GetName();
        TestEqual(*FString::Printf(TEXT("%s exactly one loaded flame light"),*Label),Lights.Num(),1);
        for(auto* Light:Lights)
        {
            Fixtures.Add(Light);
            TestTrue(*FString::Printf(TEXT("%s registered in initialized game scene"),*Label),Light->IsRegistered() && Light->GetWorld()==World && World->Scene!=nullptr);
            if(FApp::CanEverRender())
                TestTrue(*FString::Printf(TEXT("%s render state created"),*Label),Light->IsRenderStateCreated());
            TestEqual(*FString::Printf(TEXT("%s lumens"),*Label),Light->IntensityUnits,ELightUnits::Lumens);
            TestEqual(*FString::Printf(TEXT("%s base intensity"),*Label),Light->Intensity,1000.f);
            TestEqual(*FString::Printf(TEXT("%s pool radius"),*Label),Light->AttenuationRadius,375.f);
            TestEqual(*FString::Printf(TEXT("%s source radius"),*Label),Light->SourceRadius,12.f);
            TestEqual(*FString::Printf(TEXT("%s zero source length"),*Label),Light->SourceLength,0.f);
            TestEqual(*FString::Printf(TEXT("%s white input tint before temperature"),*Label),Light->LightColor,FColor::White);
            TestEqual(*FString::Printf(TEXT("%s no distance culling"),*Label),Light->MaxDrawDistance,0.f);
            TestEqual(*FString::Printf(TEXT("%s no distance fade"),*Label),Light->MaxDistanceFadeRange,0.f);
            TestEqual(*FString::Printf(TEXT("%s movable"),*Label),Light->Mobility,EComponentMobility::Movable);
            TestTrue(*FString::Printf(TEXT("%s visible and affects world"),*Label),Light->IsVisible() && !Light->bHiddenInGame && Light->bAffectsWorld && !It->IsHidden());
            TestTrue(*FString::Printf(TEXT("%s physical falloff and temperature"),*Label),Light->bUseInverseSquaredFalloff && Light->bUseTemperature && Light->Temperature==2900.f);
            TestFalse(*FString::Printf(TEXT("%s shadow cost"),*Label),Light->CastShadows);
            TestTrue(*FString::Printf(TEXT("%s flame world position"),*Label),Light->GetComponentLocation().Equals(It->GetActorTransform().TransformPosition(FVector(0,25,14)),.01));
        }
    }
    TestEqual(TEXT("All serialized fixtures survive game load"),Fixtures.Num(),PreInitFixtureLights);
    for(int32 I=0;I<Fixtures.Num();++I)
    {
        const auto* Piece=CastChecked<ALHVisualPiece>(Fixtures[I]->GetOwner());
        TestEqual(TEXT("Every B1 fixture is visual only"),Piece->GetRecipe().Collision.Num(),0);
        if(Piece->Tags.Contains(TEXT("LH.B1.WallFixture")))
        {
            bool OnWallFace=false;
            const FVector Mount=Piece->GetActorLocation();
            const FVector Inward=Piece->GetActorQuat().RotateVector(FVector::RightVector);
            for(TActorIterator<AStaticMeshActor> Wall(World);Wall;++Wall)
            {
                if(!Wall->Tags.Contains(TEXT("LH.Dressing.RetainedCollider"))) continue;
                const FVector Size=Wall->GetActorScale3D().GetAbs()*100.;
                if(Size.Z<80 || FMath::Min(Size.X,Size.Y)>40) continue;
                const FVector Center=Wall->GetActorLocation();
                const bool AlongX=Size.X>=Size.Y;
                const double Sign=(AlongX?Mount.Y-Center.Y:Mount.X-Center.X)>=0?1.:-1.;
                const double Face=AlongX?Center.Y+Sign*Size.Y/2:Center.X+Sign*Size.X/2;
                const double Distance=FMath::Abs((AlongX?Mount.Y:Mount.X)-Face);
                const bool InSegment=AlongX?FMath::Abs(Mount.X-Center.X)<=Size.X/2-44:
                    FMath::Abs(Mount.Y-Center.Y)<=Size.Y/2-44;
                const FVector ExpectedNormal=AlongX?FVector(0,Sign,0):FVector(Sign,0,0);
                if(Distance<=2. && InSegment && FVector::DotProduct(Inward,ExpectedNormal)>.99) OnWallFace=true;
            }
            TestTrue(TEXT("Sconce plate is on a real wall face and points inward"),OnWallFace);
            TestTrue(TEXT("Sconce stays below cropped visual wall top"),Mount.Z+14<=120.);
        }
        TArray<UPrimitiveComponent*> Primitives; Piece->GetComponents(Primitives);
        for(const auto* Primitive:Primitives)
        {
            TestEqual(TEXT("B1 fixture has no gameplay collision"),Primitive->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
            TestFalse(TEXT("B1 fixture has no nav influence"),Primitive->CanEverAffectNavigation());
        }
        for(int32 J=0;J<I;++J)
            TestTrue(TEXT("Relocated B1 fixtures keep at least 3m between warm pools"),
                FVector::DistSquared2D(Fixtures[I]->GetComponentLocation(),Fixtures[J]->GetComponentLocation())>=FMath::Square(300.)-.1);
    }
    auto RuntimeLux=[&](FVector Surface,FVector Normal)
    {
        double Lux=0;
        for(const auto* Light:Fixtures)
            if(Light->IsRegistered() && Light->IsVisible() && !Light->bHiddenInGame && Light->bAffectsWorld &&
                Light->GetOwner() && !Light->GetOwner()->IsHidden())
                Lux+=DirectLux(Light->GetComponentLocation(),Surface,Normal,Light->Intensity,Light->AttenuationRadius);
        return Lux;
    };
    // Actual geometry receivers: the room floor below each flame and the wall
    // behind wall-mounted sconces. Open braziers have no invented wall probe.
    double PoolMean=0., WallMean=0.; int32 WallCount=0;
    for(const auto* Light:Fixtures)
    {
        const FVector Flame=Light->GetComponentLocation();
        FHitResult FloorHit;
        const bool FoundFloor=World->LineTraceSingleByChannel(FloorHit,Flame-FVector(0,0,20),Flame-FVector(0,0,700),ECC_Visibility);
        TestTrue(*FString::Printf(TEXT("%s owner=%s flame=%s pool receiver is an actual walkable floor (hit=%s penetrating=%d normal=%s)"),*Light->GetOwner()->GetName(),*Light->GetOwner()->GetActorLocation().ToString(),*Flame.ToString(),*FloorHit.ImpactPoint.ToString(),FloorHit.bStartPenetrating,*FloorHit.ImpactNormal.ToString()),FoundFloor && !FloorHit.bStartPenetrating && FloorHit.ImpactNormal.Z>.7);
        if(!FoundFloor || FloorHit.bStartPenetrating || FloorHit.ImpactNormal.Z<=.7) continue;
        const double FloorLux=RuntimeLux(FloorHit.ImpactPoint,FloorHit.ImpactNormal);
        PoolMean+=FloorLux;
        TestTrue(TEXT("Every fixture creates a distinct floor pool"),FloorLux>=35. && FloorLux<300.);
        if(CastChecked<ALHVisualPiece>(Light->GetOwner())->GetRecipe().Id.ToString().EndsWith(TEXT(".Sconce")))
        {
            const FVector Inward=Light->GetOwner()->GetActorQuat().RotateVector(FVector::RightVector);
            const FVector Wall=Flame-Inward*25.5;
            const double WallLux=RuntimeLux(Wall,Inward);
            WallMean+=WallLux; ++WallCount;
            TestTrue(TEXT("Wall receiver behind sconce receives warm key"),WallLux>100.);
        }
    }
    // Fixed 100cm samples inside the FOUR real main room floors, 100cm from
    // walls. Samples are independent of the fixture list, so adding more lights
    // cannot move the measured dark gaps to convenient locations.
    struct FRoom { const TCHAR* Name; int32 X0,X1,Y0,Y1; };
    const FRoom Rooms[]={{TEXT("entry"),100,1700,-2700,-1100},
        {TEXT("hub"),100,1700,100,1700},{TEXT("healer"),-3100,-1100,100,1900},
        {TEXT("east"),3100,5300,100,2500}};
    const auto* PlayerDefaults=GetDefault<ALHCharacter>();
    UPointLightComponent* Fill=nullptr;
    TArray<UPointLightComponent*> RoomFills;
    int32 AtmosphereCount=0;
    for(TActorIterator<ALHB1Atmosphere> It(World);It;++It)
    {
        ++AtmosphereCount; Fill=It->ReadabilityFill.Get();
        for(const auto& Bounce:It->RoomFills) RoomFills.Add(Bounce.Get());
    }
    TestEqual(TEXT("Loaded B1 has exactly one readability atmosphere"),AtmosphereCount,1);
    if(!TestNotNull(TEXT("Loaded B1 actual readability fill"),Fill))
    {
        World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
        Package->ClearFlags(RF_Standalone);
        return false;
    }
    TestEqual(TEXT("Four fixed room bounce lights survive serialization"),RoomFills.Num(),4);
    for(const auto* Bounce:RoomFills)
    {
        TestTrue(TEXT("Room fill lights unoccupied gaps"),Bounce->IsRegistered() && Bounce->IsVisible() && Bounce->bAffectsWorld);
        TestFalse(TEXT("Room fill never casts shadows"),Bounce->CastShadows);
        TestEqual(TEXT("Room fill warm-brown tint"),Bounce->LightColor,FLinearColor(.86f,.68f,.47f).ToFColor(true));
    }
    auto BounceLux=[&](FVector Surface,FVector Normal)
    {
        double Lux=0.;
        for(const auto* Bounce:RoomFills)
        {
            const FLinearColor Color(Bounce->LightColor);
            const double Weight=.2126*Color.R+.7152*Color.G+.0722*Color.B;
            Lux+=DirectLux(Bounce->GetComponentLocation(),Surface,Normal,Bounce->Intensity,Bounce->AttenuationRadius)*Weight;
        }
        return Lux;
    };
    TestEqual(TEXT("Loaded readability fill retains450 lumens"),Fill->Intensity,450.f);
    TestEqual(TEXT("Loaded readability fill retains750cm radius"),Fill->AttenuationRadius,750.f);
    TestEqual(TEXT("Loaded readability fill retains warm tint"),Fill->LightColor,FLinearColor(.88f,.69f,.46f).ToFColor(true));
    const double FillHeight=PlayerDefaults->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+220.;
    const FLinearColor FillColor(Fill->LightColor);
    const double FillLuminance=.2126*FillColor.R+.7152*FillColor.G+.0722*FillColor.B;
    int32 Candidates=0, Rejected=0, Samples=0, Gaps=0, Pools=0, FloodedGaps=0;
    double MeanLux=0., GapMean=0., FillGapMean=0., ColoredFillGapMean=0., FloodedMean=0.;
    double TotalMean=0., ReadableGapMean=0., MinGap=DBL_MAX, TotalPoolMean=0.;
    for(const auto& Room:Rooms)
    {
        int32 RoomSamples=0, RoomGaps=0; double RoomMean=0., RoomTotal=0.;
        for(int32 X=Room.X0;X<=Room.X1;X+=100) for(int32 Y=Room.Y0;Y<=Room.Y1;Y+=100)
        {
            ++Candidates;
            FHitResult Hit;
            const bool Found=World->LineTraceSingleByChannel(Hit,FVector(X,Y,80),FVector(X,Y,-700),ECC_Visibility);
            if(!Found || Hit.bStartPenetrating || Hit.ImpactNormal.Z<.7) { ++Rejected; continue; }
            const FVector Floor=Hit.ImpactPoint, Normal=Hit.ImpactNormal;
            const double Lux=RuntimeLux(Floor,Normal);
            const double TotalLux=Lux+BounceLux(Floor,Normal);
            TotalMean+=TotalLux; RoomTotal+=TotalLux;
            // Negative regression: retain ACTUAL positions but restore W5-09b
            // 1800lm/700cm. A flooded profile must fail the same gap criterion.
            double FloodedLux=0.;
            for(const auto* Light:Fixtures)
                FloodedLux+=DirectLux(Light->GetComponentLocation(),Floor,Normal,1800.,700.);
            ++Samples; ++RoomSamples; MeanLux+=Lux; RoomMean+=Lux; FloodedMean+=FloodedLux;
            if(Lux<1.5)
            {
                ++Gaps; ++RoomGaps; GapMean+=Lux;
                ReadableGapMean+=TotalLux; MinGap=FMath::Min(MinGap,TotalLux);
                // Atmosphere Tick follows pawn CENTRE +220, not floor+220.
                // Capsule half-height comes from the actual player defaults.
                const double FillLux=DirectLux(Floor+FVector(0,0,FillHeight),Floor,Normal,Fill->Intensity,Fill->AttenuationRadius);
                FillGapMean+=TotalLux+FillLux;
                ColoredFillGapMean+=TotalLux+FillLux*FillLuminance;
            }
            if(Lux>=8.) { ++Pools; TotalPoolMean+=TotalLux; }
            if(FloodedLux<1.5) ++FloodedGaps;
        }
        TestTrue(*FString::Printf(TEXT("%s retains spaces between warm pools"),Room.Name),RoomGaps>=RoomSamples*.15);
        AddInfo(FString::Printf(TEXT("B1 %s predicted floor mean %.4f warm / %.4f with tinted room fill lux; warm gap fraction %.3f"),Room.Name,RoomMean/RoomSamples,RoomTotal/RoomSamples,double(RoomGaps)/RoomSamples));
    }
    TestTrue(TEXT("At least 25 percent of real room floor samples are warm-light gaps"),Gaps>=Samples*.25);
    TestTrue(TEXT("At least 15 percent of real room floor samples are lit pools"),Pools>=Samples*.15);
    TestTrue(TEXT("Prior flooded attenuation fails the 25 percent gap criterion"),FloodedGaps<Samples*.25);
    TestEqual(TEXT("Fixed room grids inspect every authored candidate"),Candidates,1552);
    TestTrue(TEXT("At least 90 percent of fixed room grid finds walkable surfaces"),Samples>=Candidates*.90);
    TestTrue(TEXT("Unoccupied gaps average at least 3 lux with room bounce"),Gaps>0 && ReadableGapMean/Gaps>=3.);
    TestTrue(TEXT("Every sampled gap has a dim base above .75 lux"),MinGap>=.75);
    TestTrue(TEXT("Unoccupied gaps stay dim below 12 lux on average"),Gaps>0 && ReadableGapMean/Gaps<12.);
    TestTrue(TEXT("Warm pools remain at least three times brighter than gaps"),Pools>0 && Gaps>0 && TotalPoolMean/Pools>=3.*ReadableGapMean/Gaps);
    TestTrue(TEXT("Removing room bounce fails readable-gap target"),Gaps>0 && GapMean/Gaps<3.);
    AddInfo(FString::Printf(TEXT("B1 room bounce: total floor mean %.4f lux; unoccupied gap mean %.4f minimum %.4f lux; pool mean %.4f lux / gap contrast %.3f; bounce-off gap mean %.4f lux"),TotalMean/Samples,ReadableGapMean/FMath::Max(1,Gaps),MinGap,TotalPoolMean/FMath::Max(1,Pools),(TotalPoolMean/FMath::Max(1,Pools))/(ReadableGapMean/FMath::Max(1,Gaps)),GapMean/FMath::Max(1,Gaps)));
    TestTrue(TEXT("Actual pawn-centred fill keeps occupied gaps above 6 direct lux"),Gaps>0 && FillGapMean/Gaps>=6.);
    AddInfo(FString::Printf(TEXT("B1 floor sampling candidates=%d walkable=%d rejected=%d; pawn capsule halfheight=%.1f fill floor height=%.1f actual intensity=%.1f radius=%.1f; approximate linear color luminance weight=%.4f occupied gap tinted estimate=%.4f lux (temperature/material/exposure excluded)"),Candidates,Samples,Rejected,FillHeight-220.,FillHeight,Fill->Intensity,Fill->AttenuationRadius,FillLuminance,ColoredFillGapMean/FMath::Max(1,Gaps)));
    TestTrue(TEXT("At least one actual wall-mounted fixture is audited"),WallCount>0);
    AddInfo(FString::Printf(TEXT("B1 photometric prediction: fixture floor pool mean %.4f lux; wall key mean %.4f lux; room floor mean %.4f lux; warm gap mean %.4f lux; occupied gap with cool fill %.4f lux; restored flooded mean %.4f lux / gap fraction %.3f; pools %.3f gaps %.3f"),
        PoolMean/FMath::Max(1,Fixtures.Num()),WallMean/FMath::Max(1,WallCount),MeanLux/Samples,GapMean/FMath::Max(1,Gaps),FillGapMean/FMath::Max(1,Gaps),FloodedMean/Samples,double(FloodedGaps)/Samples,double(Pools)/Samples,double(Gaps)/Samples));
    AddInfo(TEXT("Rendered basement mean target .15-.25, room1/2/3 predictions .19/.18/.16 (at least +/-.05 uncertainty); these direct lux means exclude material, sky, exposure, fog and tone mapping and cannot establish capture luma."));
    // Verify supports after InitWorld and BeginPlay rebuild them against real
    // physics. Editor PostLoad occurs before a usable floor tracing scene.
    int32 Walls=0,Braziers=0,Sconces=0;
    for(TActorIterator<ALHVisualPiece> It(World);It;++It)
    {
        const auto& Recipe=It->GetRecipe();
        if(Recipe.Style!=ELHVisualStyle::B1Cellar) continue;
        LHB1Art::FFit Fit;
        if(LHB1Art::Resolve(Recipe,Fit) && Fit.AssetName==TEXT("Wall400"))
        {
            ++Walls;
            const auto Backing=LHB1Art::SolidBacking(Recipe,Fit);
            TestEqual(TEXT("Wall has one inset solid core"),Backing.Num(),1);
            if(Backing.Num()!=1) continue;
            const FVector CoreLo=Backing[0].Center-Backing[0].Size/2,CoreHi=Backing[0].Center+Backing[0].Size/2;
            TestTrue(TEXT("Wall core is inset five centimetres on both broad faces"),
                FMath::IsNearlyEqual(CoreLo.Y-Fit.ClipBounds.Min.Y,5.,.01) &&
                FMath::IsNearlyEqual(Fit.ClipBounds.Max.Y-CoreHi.Y,5.,.01));
            auto* Support=It->GetArtSupport();
            const auto* Section=Support?Support->GetProcMeshSection(0):nullptr;
            if(!TestNotNull(TEXT("Wall backing and cut caps are built"),Section)) continue;
            const auto Caps=LHB1Art::MasonryCaps(Recipe,Fit);
            TestTrue(TEXT("Wall coping is segmented into multiple stones"),Caps.Num()>4);
            TestEqual(TEXT("One core plus independently fitted masonry stones"),Section->ProcVertexBuffer.Num(),(1+Caps.Num())*24);
            TestTrue(TEXT("Core mortar recedes 1.5cm below coping top and end joints"),
                FMath::IsNearlyEqual(CoreHi.Z,Fit.ClipBounds.Max.Z-1.5,.01) &&
                FMath::IsNearlyEqual(CoreLo.X,Fit.ClipBounds.Min.X+1.5,.01) &&
                FMath::IsNearlyEqual(CoreHi.X,Fit.ClipBounds.Max.X-1.5,.01));
            FBox SupportBounds(ForceInit);
            double MaximumOverflow=0.;
            for(const auto& Vertex:Section->ProcVertexBuffer)
            {
                const FVector P=Vertex.Position;
                // Procedural vertices serialize to float; fitted recipe math is
                // double. A hundredth centimetre allows only conversion roundoff.
                TestTrue(TEXT("Wall support stays inside masonry crop"),Fit.ClipBounds.ExpandBy(.01).IsInsideOrOn(P));
                SupportBounds+=P;
                for(int32 Axis=0;Axis<3;++Axis)
                    MaximumOverflow=FMath::Max(MaximumOverflow,FMath::Max(Fit.ClipBounds.Min[Axis]-P[Axis],P[Axis]-Fit.ClipBounds.Max[Axis]));
                const bool InCoreDepth=P.Y>=CoreLo.Y-.01 && P.Y<=CoreHi.Y+.01;
                const bool AtPerimeter=P.X<=Fit.ClipBounds.Min.X+6.01 || P.X>=Fit.ClipBounds.Max.X-6.01 ||
                    P.Z<=Fit.ClipBounds.Min.Z+6.01 || P.Z>=Fit.ClipBounds.Max.Z-6.01;
                TestTrue(TEXT("Caps cannot fill broad mortar recesses"),InCoreDepth || AtPerimeter);
            }
            if(MaximumOverflow>0.) AddInfo(FString::Printf(TEXT("%s support maximum crop overflow %.9fcm"),*It->GetName(),MaximumOverflow));
            TestTrue(TEXT("Coping and core together close exact saved silhouette"),
                SupportBounds.Min.Equals(Fit.ClipBounds.Min,.01) && SupportBounds.Max.Equals(Fit.ClipBounds.Max,.01));
            TestEqual(TEXT("Masonry remains visual only"),Support->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
            TestFalse(TEXT("Masonry cannot affect navigation"),Support->CanEverAffectNavigation());
        }
        if(Recipe.Id.ToString().EndsWith(TEXT(".Sconce"))) ++Sconces;
        if(Recipe.Id.ToString().EndsWith(TEXT(".Torch")))
        {
            ++Braziers;
            TestEqual(TEXT("Brazier geometry contributes no collision"),Recipe.Collision.Num(),0);
            TestTrue(TEXT("Brazier bowl has broad circular bounds"),Recipe.Geometry.ContainsByPredicate([](const FLHVisualBox& B){return B.Size.X==64 && B.Size.Y==64;}));
            const auto* Bowl=It->GetMesh()->GetProcMeshSection(0);
            if(TestNotNull(TEXT("Open iron bowl surface"),Bowl))
            {
                TestEqual(TEXT("Revolved bowl has exterior, rim and interior"),Bowl->ProcIndexBuffer.Num()/3,288);
                bool HasLip=false, HasWell=false;
                for(const auto& V:Bowl->ProcVertexBuffer)
                {
                    const FVector P=V.Position-FVector(0,25,0);
                    const double Radius=P.Size2D();
                    TestTrue(TEXT("Bowl is circular rather than a square tabletop"),Radius<=32.01);
                    TestTrue(TEXT("No surface spans the open bowl mouth"),P.Z<-1 || Radius>=27.9);
                    TestTrue(TEXT("Iron vertex tint is dark rather than white"),V.Color.R<=110 && V.Color.G<=110 && V.Color.B<=110);
                    TestTrue(TEXT("Bowl has valid normals"),V.Normal.SizeSquared()>.99);
                    HasLip|=P.Z>=-.01; HasWell|=Radius<.01 && P.Z<-20;
                }
                TestTrue(TEXT("Bowl lip surrounds a recessed well"),HasLip && HasWell);
            }
            for(const auto* Surface:{It->GetMesh(),It->GetArtSupport()})
            {
                const auto* Iron=Cast<UMaterialInstanceDynamic>(Surface->GetMaterial(0));
                TestTrue(TEXT("Bowl and pedestal bind imported iron rather than white primitive material"),Iron && Iron->Parent && Iron->Parent->GetName()==TEXT("MI_B1_iron"));
                UTexture* ORM=nullptr;
                if(Iron && TestTrue(TEXT("Iron roughness and metallic use bound ORM"),Iron->GetTextureParameterValue(FMaterialParameterInfo(TEXT("ORM")),ORM)))
                    if(TestNotNull(TEXT("Iron ORM exists"),ORM)) TestFalse(TEXT("Iron ORM linear"),bool(ORM->SRGB));
            }
            const auto* FlameSection=It->GetMesh()->GetProcMeshSection(1);
            if(TestNotNull(TEXT("Brazier flame surface is built"),FlameSection))
            {
                TestEqual(TEXT("Coal bed and four tapered flames emit160 triangles"),FlameSection->ProcIndexBuffer.Num()/3,160);
                for(const auto& Vertex:FlameSection->ProcVertexBuffer)
                    TestTrue(TEXT("Flame vertices stay inside conservative recipe bounds"),Recipe.Geometry.ContainsByPredicate([&](const FLHVisualBox& B)
                    {
                        if(B.Surface!=1) return false;
                        const FVector Local=B.Rotation.UnrotateVector(Vertex.Position-B.Center);
                        return FMath::Abs(Local.X)<=B.Size.X/2+.01 && FMath::Abs(Local.Y)<=B.Size.Y/2+.01 && FMath::Abs(Local.Z)<=B.Size.Z/2+.01;
                    }));
            }
            const auto* FlameMaterial=Cast<UMaterialInstanceDynamic>(It->GetMesh()->GetMaterial(1));
            TestTrue(TEXT("Brazier flame uses existing imported emissive material"),FlameMaterial && FlameMaterial->Parent && FlameMaterial->Parent->GetName().Contains(TEXT("flame")));
            const auto* Section=It->GetArtSupport()->GetProcMeshSection(0);
            if(!TestNotNull(TEXT("Brazier central pedestal and circular flange are built"),Section)) continue;
            FBox Bounds(ForceInit);
            for(const auto& Vertex:Section->ProcVertexBuffer) Bounds+=Vertex.Position;
            TestEqual(TEXT("Revolved pedestal topology has no four table legs"),Section->ProcIndexBuffer.Num()/3,160);
            for(int32 I=0;I<Section->ProcIndexBuffer.Num();I+=3)
            {
                const auto& A=Section->ProcVertexBuffer[Section->ProcIndexBuffer[I]];
                const auto& B=Section->ProcVertexBuffer[Section->ProcIndexBuffer[I+1]];
                const auto& C=Section->ProcVertexBuffer[Section->ProcIndexBuffer[I+2]];
                TestTrue(TEXT("Pedestal has no collapsed pole triangles or inward faces"),
                    FVector::DotProduct(FVector::CrossProduct(B.Position-A.Position,C.Position-A.Position),A.Normal)<0);
            }
            TestTrue(TEXT("Freestanding brazier flange spans54 centimetres"),FMath::IsNearlyEqual(Bounds.GetSize().X,54.,.01) && FMath::IsNearlyEqual(Bounds.GetSize().Y,54.,.01));
            TestEqual(TEXT("Brazier support has no collision"),It->GetArtSupport()->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
            TestFalse(TEXT("Brazier support has no nav influence"),It->GetArtSupport()->CanEverAffectNavigation());
        }
    }
    TestTrue(TEXT("Real generated masonry was inspected"),Walls>0);
    TestTrue(TEXT("Real generated freestanding braziers were inspected"),Braziers>0);
    TestTrue(TEXT("Wall sconces outnumber freestanding fixtures"),Sconces>Braziers);
    AddInfo(FString::Printf(TEXT("B1 fixtures: wall sconces=%d freestanding iron bowls=%d"),Sconces,Braziers));
    // DestroyWorld unregisters components/releases the scene and clears world
    // standalone/root flags. Remove this isolated context as well; the unique
    // /Temp package can then be reclaimed without disturbing shared editor maps.
    World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
    Package->ClearFlags(RF_Standalone);
    TestTrue(TEXT("Isolated game-world context removed"),GEngine->GetWorldContextFromWorld(World)==nullptr);
    return true;
}
