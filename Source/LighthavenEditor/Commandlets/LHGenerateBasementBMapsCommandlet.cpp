#include "LHGenerateBasementBMapsCommandlet.h"
#include "LHMapDressing.h"
#include "Editor.h"
#include "Framework/LHArrivalReview.h"
#include "FileHelpers.h"
#include "ActorFactories/ActorFactory.h"
#include "Builders/CubeBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/TargetPoint.h"
#include "Engine/SpotLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Engine/TextureCube.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "Framework/LHGameMode.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "World/LHAreaRegistry.h"
#include "World/LHWorldMarkers.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "HAL/FileManager.h"

namespace LHBasementBGenerator
{
// All geometry is Prototype, V-01 / LH_Prototype_v1; lighting is A-04.
// Baked rectangle-union strips preserve absent floor without a rectangular backing slab.
struct FBuilder
{
    UWorld* World;
    UStaticMesh* Cube;
    bool bOK = true;
    TMap<FName, UMaterialInstanceConstant*> Palette;
    void Material(const TCHAR* Name, const TCHAR* Hex)
    {
        auto* Parent = LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        if (!Parent) { bOK=false; return; }
        // Embedded in this map, no independently saved material asset.
        auto* M = NewObject<UMaterialInstanceConstant>(World,FName(Name),RF_Public|RF_Transactional);
        M->SetParentEditorOnly(Parent);
        M->SetVectorParameterValueEditorOnly(TEXT("Color"),FLinearColor(FColor::FromHex(Hex)));
        Palette.Add(FName(Name),M);
    }
    template<class T> T* Actor(const TCHAR* Name, FVector Position, FRotator Rotation = FRotator::ZeroRotator)
    {
        FActorSpawnParameters P;
        P.Name = FName(Name);
        P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        T* A = World->SpawnActor<T>(Position, Rotation, P);
        if (!A) { bOK = false; return nullptr; }
        A->SetActorLabel(Name);
        A->Tags.Append({FName(TEXT("LH.W3.Generated")), FName(TEXT("LH.Prototype.V01"))});
        return A;
    }
    void Box(const TCHAR* Name, FVector Center, FVector Size, bool bCollision = true, FRotator Rotation = FRotator::ZeroRotator)
    {
        if (auto* A = Actor<AStaticMeshActor>(Name, Center, Rotation))
        {
            auto* C = A->GetStaticMeshComponent();
            C->SetStaticMesh(Cube);
            const FString Label(Name);
            const FName Key = Label.Contains(TEXT("Pale")) ? TEXT("Pale") : Label.Contains(TEXT("Stain")) ? TEXT("Red")
                : Label.Contains(TEXT("DarkFloor")) ? TEXT("Dark") : Label.StartsWith(TEXT("Floor"))
                || Label.Contains(TEXT("Ramp")) || Label.Contains(TEXT("Landing")) ? TEXT("Floor") : TEXT("Stone");
            if (auto* const* M=Palette.Find(Key)) C->SetMaterial(0,*M);
            C->SetMobility(EComponentMobility::Static);
            C->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
            C->SetCollisionProfileName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision"));
            C->SetCanEverAffectNavigation(bCollision);
            A->SetActorScale3D(Size / 100.0);
        }
    }
    void Ramp(const TCHAR* Name, FVector Start, FVector End, double Width)
    {
        const FVector Delta = End - Start;
        const double Run = FVector(Delta.X, Delta.Y, 0).Size();
        const double Angle = FMath::Atan2(Delta.Z, Run);
        const double Yaw = FMath::Atan2(Delta.Y, Delta.X);
        const FVector Normal(-FMath::Sin(Angle)*FMath::Cos(Yaw), -FMath::Sin(Angle)*FMath::Sin(Yaw), FMath::Cos(Angle));
        Box(Name, (Start+End)/2 - Normal*10, FVector(Delta.Size(),Width,20), true,
            FRotator(FMath::RadiansToDegrees(Angle),FMath::RadiansToDegrees(Yaw),0));
    }
    void Light(const TCHAR* Name, FVector Position, TCHAR Type, bool bCell = false)
    {
        UPointLightComponent* C = nullptr;
        if (Type == 'F')
        {
            if (auto* A = Actor<ASpotLight>(Name, Position, FRotator(-90,0,0)))
            {
                auto* Spot = CastChecked<USpotLightComponent>(A->GetLightComponent());
                Spot->SetInnerConeAngle(50); Spot->SetOuterConeAngle(65); C = Spot;
            }
        }
        else if (auto* A = Actor<APointLight>(Name, Position)) C = CastChecked<UPointLightComponent>(A->GetLightComponent());
        if (C)
        {
            C->SetMobility(EComponentMobility::Movable);
            C->SetIntensityUnits(ELightUnits::Lumens);
            C->SetUseTemperature(true);
            C->SetTemperature(Type == 'N' || Type == 'F' ? 6500 : 2200);
            C->SetIntensity(bCell ? 600 : Type == 'F' ? 4000 : Type == 'N' ? 1800 : Type == 'W' ? 1500 : 900);
            C->SetAttenuationRadius(Type == 'F' ? 2000 : 1400);
            C->SetCastShadows(false);
        }
    }
    void CoverageLighting()
    {
        // Prototype presentation: floor strips are sampled at <=8m spacing.
        // Collect first: spawning while iterating the actor array invalidates that iteration.
        TArray<FVector> Positions;
        for(AActor* A:World->PersistentLevel->Actors)
        {
            if(!A || !A->GetName().StartsWith(TEXT("Floor"))) continue;
            const FVector Center=A->GetActorLocation();
            const FVector Size=A->GetActorScale3D()*100.;
            const int32 NX=FMath::CeilToInt(Size.X/800.);
            const int32 NY=FMath::CeilToInt(Size.Y/800.);
            for(int32 X=0;X<NX;++X) for(int32 Y=0;Y<NY;++Y)
            {
                const FVector P(Center.X-Size.X/2+(X+.5)*Size.X/NX,
                    Center.Y-Size.Y/2+(Y+.5)*Size.Y/NY,250);
                // Adjacent narrow union strips can share a fill; cap redundant overlap.
                if(!Positions.ContainsByPredicate([&](const FVector& Q){ return FVector::DistSquared2D(P,Q)<FMath::Square(400.); }))
                    Positions.Add(P);
            }
        }
        for(int32 I=0;I<Positions.Num();++I)
            Light(*FString::Printf(TEXT("Coverage_%03d"),I),Positions[I],'N');
    }
    bool WriteManifest(const FString& Map) const
    {
        TArray<FString> Lines;
        for (AActor* A : World->PersistentLevel->Actors)
        {
            if (!A || !A->Tags.Contains(TEXT("LH.W3.Generated"))) continue;
            FString Line=A->GetName()+TEXT("|")+A->GetClass()->GetPathName()+TEXT("|")+A->GetActorTransform().ToString();
            if (auto* M=Cast<ALHSpawnMarker>(A)) Line+=TEXT("|")+M->SpawnId.ToString()+TEXT("|")+M->EnemyDefinitionId.Value.ToString();
            if (auto* P=Cast<ALHPortal>(A)) Line+=TEXT("|")+P->PortalId.ToString()+TEXT("|")+P->Source.LocalId.ToString()+TEXT("|")+P->Destination.Area.Content.Value.ToString()+TEXT("|")+P->Destination.LocalId.ToString();
            Lines.Add(Line);
        }
        Lines.Sort();
        const FString Directory=FPaths::ProjectSavedDir()/TEXT("BasementB");
        return IFileManager::Get().MakeDirectory(*Directory,true)
            && FFileHelper::SaveStringArrayToFile(Lines,*(Directory/ (Map+TEXT(".txt"))));
    }
    void Common(bool bB3, const FLHAreaDefinition& Area)
    {
        Material(TEXT("Stone"),bB3 ? TEXT("78634B") : TEXT("514B42"));
        Material(TEXT("Floor"),TEXT("695640")); Material(TEXT("Pale"),TEXT("9B9484"));
        Material(TEXT("Red"),TEXT("733D35")); Material(TEXT("Dark"),TEXT("333333"));
        World->GetWorldSettings()->DefaultGameMode = ALHGameMode::StaticClass();
        if (auto* A = Actor<ASkyLight>(TEXT("Ambient"), FVector(0,0,800)))
        {
            auto* C = A->GetLightComponent(); C->SetMobility(EComponentMobility::Movable);
            C->SetIntensity(.8f); C->SetCastShadows(false);
            C->SourceType=SLS_SpecifiedCubemap;
            C->bRealTimeCapture=false; C->bLowerHemisphereIsBlack=false;
            auto* Neutral=LoadObject<UTextureCube>(nullptr,TEXT("/Engine/EngineResources/GrayLightTextureCube.GrayLightTextureCube"));
            if (!Neutral) bOK=false; else C->SetCubemap(Neutral);
            // Fixed engine gray calibration source; host must compare its brightness with other floors.
        }
        if (auto* A = Actor<APostProcessVolume>(TEXT("Exposure"), FVector::ZeroVector))
        {
            A->bUnbound = true;
            A->Settings.bOverride_AutoExposureMinBrightness = true;
            A->Settings.bOverride_AutoExposureMaxBrightness = true;
            A->Settings.AutoExposureMinBrightness = 4.f;
            A->Settings.AutoExposureMaxBrightness = 4.f;
        // W4-09: explicit physical-camera exposure, independent of extended range.
        A->Settings.bOverride_AutoExposureMethod=true; A->Settings.AutoExposureMethod=AEM_Manual;
        A->Settings.bOverride_AutoExposureApplyPhysicalCameraExposure=true; A->Settings.AutoExposureApplyPhysicalCameraExposure=true;
        A->Settings.bOverride_CameraISO=true; A->Settings.CameraISO=100;
        A->Settings.bOverride_CameraShutterSpeed=true; A->Settings.CameraShutterSpeed=FMath::Pow(2.f,4.f)/4.f; // Prototype EV100 4; B3/B4 only, +1.5 stops from W4-09d.
        A->Settings.bOverride_DepthOfFieldFstop=true; A->Settings.DepthOfFieldFstop=2;
            A->Settings.bOverride_AutoExposureBias = true; A->Settings.AutoExposureBias = 0;
        }
        if (auto* A = Actor<ANavMeshBoundsVolume>(TEXT("NavigationBounds"), FVector(bB3 ? 700 : 1400,3500,200)))
        {
            auto* B = NewObject<UCubeBuilder>(); B->X = bB3 ? 10600 : 9000; B->Y = 10000; B->Z = 1600;
            UActorFactory::CreateBrushForVolumeActor(A,B);
        }
        for (const auto& E : Area.Entrances)
        {
            const FString Name = TEXT("Arrival_") + E.Id.LocalId.ToString();
            if (auto* A = Actor<ALHEntranceMarker>(*Name,E.SafeTransform.GetLocation(),E.SafeTransform.Rotator()))
            { A->EntranceId=E.Id; A->SafeArrivalTransform=E.SafeTransform; A->bSafetyReviewed=LHArrivalReview::IsReviewed(E); }
        }
        const auto& Entry = Area.Entrances[0].SafeTransform;
        Actor<APlayerStart>(TEXT("PlayerStart"),Entry.GetLocation()+FVector(0,0,100),Entry.Rotator());
        for (const auto& S : Area.Spawns)
        {
            if (auto* A = Actor<ALHSpawnMarker>(*S.Alias.ToString(),S.Anchor.GetLocation(),S.Anchor.Rotator()))
            { A->Area=Area.Id; A->SpawnId=S.SpawnId; A->EnemyDefinitionId=S.Enemy; A->Tags.Add(TEXT("LH.Spawn.Prototype")); }
        }
        for (const auto& P : Area.Portals)
        {
            const bool bReturn = P.Source.LocalId == FName(TEXT("Entry"));
            const FVector Position = bB3 ? (bReturn ? FVector(3200,1360,80) : FVector(4700,7060,-80)) : FVector(-210,600,80);
            const FString Name = TEXT("Portal_") + P.Source.LocalId.ToString();
            if (auto* A = Actor<ALHPortal>(*Name,Position,FRotator(0,bB3 ? 90 : 180,0)))
            { A->PortalId=P.Portal.InstanceId; A->Source=P.Source; A->Destination=P.Destination;
              A->Direction=bReturn ? ELHPortalDirection::Return : ELHPortalDirection::Descent;
              A->Tags.Add(TEXT("LH.Portal.ExplicitInteraction"));
              auto* Bounds=NewObject<UBoxComponent>(A,TEXT("InteractionBounds"));
              A->AddInstanceComponent(Bounds); Bounds->SetupAttachment(A->GetRootComponent());
              Bounds->SetRelativeLocation(FVector(0,0,150)); Bounds->SetBoxExtent(FVector(50,150,150));
              Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision); Bounds->SetGenerateOverlapEvents(false);
              Bounds->RegisterComponent();
              Box(*(Name+TEXT("DirectionMark")),Position+FVector(0,0,3),FVector(80,20,4),false,FRotator(0,bB3 ? 90 : 180,0));
            }
        }
    }
};
void Geometry3(FBuilder& B)
{
    B.Actor<ATargetPoint>(TEXT("B3.Entry"),FVector(3500,500,0));
    B.Actor<ATargetPoint>(TEXT("B3.LowerWest"),FVector(1700,-300,0));
    B.Actor<ATargetPoint>(TEXT("B3.C3Cell"),FVector(1650,-350,0));
    B.Actor<ATargetPoint>(TEXT("B3.West"),FVector(-200,300,0));
    B.Actor<ATargetPoint>(TEXT("B3.WestBay"),FVector(1300,1600,0));
    B.Actor<ATargetPoint>(TEXT("B3.UpperCentral"),FVector(100,3000,0));
    B.Actor<ATargetPoint>(TEXT("B3.BranchNear"),FVector(-1800,3000,0));
    B.Actor<ATargetPoint>(TEXT("B3.BranchEnd"),FVector(-3600,3100,0));
    B.Actor<ATargetPoint>(TEXT("B3.East"),FVector(2900,3900,0));
    B.Actor<ATargetPoint>(TEXT("B3.LowerRight"),FVector(5000,3300,0));
    B.Actor<ATargetPoint>(TEXT("B3.Descent"),FVector(4700,6100,0));
    B.Box(TEXT("Floor000"), FVector(1700,-900,-10), FVector(1400,600,20));
    B.Box(TEXT("Floor001"), FVector(-200,-480,-10), FVector(1600,240,20));
    B.Box(TEXT("Floor002"), FVector(1700,-480,-10), FVector(1400,240,20));
    B.Box(TEXT("Floor003"), FVector(700,-280,-10), FVector(3400,160,20));
    B.Box(TEXT("Floor004"), FVector(700,-180,-10), FVector(3400,40,20));
    B.Box(TEXT("Floor005"), FVector(3500,-180,-10), FVector(1400,40,20));
    B.Box(TEXT("Floor006"), FVector(1600,-100,-10), FVector(5200,120,20));
    B.Box(TEXT("Floor007"), FVector(-200,60,-10), FVector(1600,200,20));
    B.Box(TEXT("Floor008"), FVector(2600,60,-10), FVector(3200,200,20));
    B.Box(TEXT("Floor009"), FVector(-200,300,-10), FVector(1600,280,20));
    B.Box(TEXT("Floor010"), FVector(1700,300,-10), FVector(1400,280,20));
    B.Box(TEXT("Floor011"), FVector(3500,300,-10), FVector(1400,280,20));
    B.Box(TEXT("Floor012"), FVector(-200,520,-10), FVector(1600,160,20));
    B.Box(TEXT("Floor013"), FVector(1700,520,-10), FVector(1400,160,20));
    B.Box(TEXT("Floor014"), FVector(3980,520,-10), FVector(2360,160,20));
    B.Box(TEXT("Floor015"), FVector(-200,620,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor016"), FVector(3980,620,-10), FVector(2360,40,20));
    B.Box(TEXT("Floor017"), FVector(230,700,-10), FVector(2460,120,20));
    B.Box(TEXT("Floor018"), FVector(3980,700,-10), FVector(2360,120,20));
    B.Box(TEXT("Floor019"), FVector(230,780,-10), FVector(2460,40,20));
    B.Box(TEXT("Floor020"), FVector(3500,780,-10), FVector(1400,40,20));
    B.Box(TEXT("Floor021"), FVector(5000,780,-10), FVector(320,40,20));
    B.Box(TEXT("Floor022"), FVector(230,875,-10), FVector(2460,150,20));
    B.Box(TEXT("Floor023"), FVector(3500,875,-10), FVector(1400,150,20));
    B.Box(TEXT("Floor024"), FVector(5000,875,-10), FVector(320,150,20));
    B.Box(TEXT("Floor025"), FVector(230,955,-10), FVector(2460,10,20));
    B.Box(TEXT("Floor026"), FVector(2925,955,-10), FVector(250,10,20));
    B.Box(TEXT("Floor027"), FVector(3775,955,-10), FVector(850,10,20));
    B.Box(TEXT("Floor028"), FVector(5000,955,-10), FVector(320,10,20));
    B.Box(TEXT("Floor029"), FVector(-200,1080,-10), FVector(1600,240,20));
    B.Box(TEXT("Floor030"), FVector(1300,1080,-10), FVector(320,240,20));
    B.Box(TEXT("Floor031"), FVector(2925,1080,-10), FVector(250,240,20));
    B.Box(TEXT("Floor032"), FVector(3775,1080,-10), FVector(850,240,20));
    B.Box(TEXT("Floor033"), FVector(5000,1080,-10), FVector(320,240,20));
    B.Box(TEXT("Floor034"), FVector(0,1360,-10), FVector(320,320,20));
    B.Box(TEXT("Floor035"), FVector(1300,1360,-10), FVector(1000,320,20));
    B.Box(TEXT("Floor036"), FVector(5000,1360,-10), FVector(320,320,20));
    B.Box(TEXT("Floor037"), FVector(0,1760,-10), FVector(320,480,20));
    B.Box(TEXT("Floor038"), FVector(1300,1760,-10), FVector(1000,480,20));
    B.Box(TEXT("Floor039"), FVector(5000,1760,-10), FVector(320,480,20));
    B.Box(TEXT("Floor040"), FVector(0,2100,-10), FVector(320,200,20));
    B.Box(TEXT("Floor041"), FVector(5000,2100,-10), FVector(320,200,20));
    B.Box(TEXT("Floor042"), FVector(100,2300,-10), FVector(1800,200,20));
    B.Box(TEXT("Floor043"), FVector(5000,2300,-10), FVector(320,200,20));
    B.Box(TEXT("Floor044"), FVector(-3600,2420,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor045"), FVector(-1800,2420,-10), FVector(1200,40,20));
    B.Box(TEXT("Floor046"), FVector(100,2420,-10), FVector(1800,40,20));
    B.Box(TEXT("Floor047"), FVector(5000,2420,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor048"), FVector(-3600,2520,-10), FVector(1600,160,20));
    B.Box(TEXT("Floor049"), FVector(-1800,2520,-10), FVector(1200,160,20));
    B.Box(TEXT("Floor050"), FVector(100,2520,-10), FVector(1800,160,20));
    B.Box(TEXT("Floor051"), FVector(2100,2520,-10), FVector(1320,160,20));
    B.Box(TEXT("Floor052"), FVector(5000,2520,-10), FVector(1600,160,20));
    B.Box(TEXT("Floor053"), FVector(-3600,2680,-10), FVector(1600,160,20));
    B.Box(TEXT("Floor054"), FVector(-1800,2680,-10), FVector(1200,160,20));
    B.Box(TEXT("Floor055"), FVector(100,2680,-10), FVector(1800,160,20));
    B.Box(TEXT("Floor056"), FVector(2100,2680,-10), FVector(1320,160,20));
    B.Box(TEXT("Floor057"), FVector(5000,2680,-10), FVector(1600,160,20));
    B.Box(TEXT("Floor058"), FVector(-3600,2800,-10), FVector(1600,80,20));
    B.Box(TEXT("Floor059"), FVector(-1800,2800,-10), FVector(1200,80,20));
    B.Box(TEXT("Floor060"), FVector(100,2800,-10), FVector(1800,80,20));
    B.Box(TEXT("Floor061"), FVector(1600,2800,-10), FVector(320,80,20));
    B.Box(TEXT("Floor062"), FVector(2600,2800,-10), FVector(320,80,20));
    B.Box(TEXT("Floor063"), FVector(5000,2800,-10), FVector(1600,80,20));
    B.Box(TEXT("Floor064"), FVector(-1700,3000,-10), FVector(5400,320,20));
    B.Box(TEXT("Floor065"), FVector(1600,3000,-10), FVector(320,320,20));
    B.Box(TEXT("Floor066"), FVector(2600,3000,-10), FVector(320,320,20));
    B.Box(TEXT("Floor067"), FVector(5000,3000,-10), FVector(1600,320,20));
    B.Box(TEXT("Floor068"), FVector(-3600,3180,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor069"), FVector(-1800,3180,-10), FVector(1200,40,20));
    B.Box(TEXT("Floor070"), FVector(100,3180,-10), FVector(1800,40,20));
    B.Box(TEXT("Floor071"), FVector(1600,3180,-10), FVector(320,40,20));
    B.Box(TEXT("Floor072"), FVector(2600,3180,-10), FVector(320,40,20));
    B.Box(TEXT("Floor073"), FVector(5000,3180,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor074"), FVector(-3600,3220,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor075"), FVector(-1800,3220,-10), FVector(1200,40,20));
    B.Box(TEXT("Floor076"), FVector(100,3220,-10), FVector(1800,40,20));
    B.Box(TEXT("Floor077"), FVector(1600,3220,-10), FVector(320,40,20));
    B.Box(TEXT("Floor078"), FVector(2900,3220,-10), FVector(1400,40,20));
    B.Box(TEXT("Floor079"), FVector(5000,3220,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor080"), FVector(-3600,3320,-10), FVector(1600,160,20));
    B.Box(TEXT("Floor081"), FVector(-1800,3320,-10), FVector(1200,160,20));
    B.Box(TEXT("Floor082"), FVector(480,3320,-10), FVector(2560,160,20));
    B.Box(TEXT("Floor083"), FVector(2900,3320,-10), FVector(1400,160,20));
    B.Box(TEXT("Floor084"), FVector(5000,3320,-10), FVector(1600,160,20));
    B.Box(TEXT("Floor085"), FVector(-3600,3480,-10), FVector(1600,160,20));
    B.Box(TEXT("Floor086"), FVector(-1800,3480,-10), FVector(1200,160,20));
    B.Box(TEXT("Floor087"), FVector(480,3480,-10), FVector(2560,160,20));
    B.Box(TEXT("Floor088"), FVector(2900,3480,-10), FVector(1400,160,20));
    B.Box(TEXT("Floor089"), FVector(5000,3480,-10), FVector(1600,160,20));
    B.Box(TEXT("Floor090"), FVector(-3600,3580,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor091"), FVector(-1800,3580,-10), FVector(1200,40,20));
    B.Box(TEXT("Floor092"), FVector(100,3580,-10), FVector(1800,40,20));
    B.Box(TEXT("Floor093"), FVector(2900,3580,-10), FVector(1400,40,20));
    B.Box(TEXT("Floor094"), FVector(5000,3580,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor095"), FVector(-3600,3620,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor096"), FVector(100,3620,-10), FVector(1800,40,20));
    B.Box(TEXT("Floor097"), FVector(2900,3620,-10), FVector(1400,40,20));
    B.Box(TEXT("Floor098"), FVector(5000,3620,-10), FVector(1600,40,20));
    B.Box(TEXT("Floor099"), FVector(-3600,3720,-10), FVector(1600,160,20));
    B.Box(TEXT("Floor100"), FVector(100,3720,-10), FVector(1800,160,20));
    B.Box(TEXT("Floor101"), FVector(4000,3720,-10), FVector(3600,160,20));
    B.Box(TEXT("Floor102"), FVector(4000,3880,-10), FVector(3600,160,20));
    B.Box(TEXT("Floor103"), FVector(2900,4080,-10), FVector(1400,240,20));
    B.Box(TEXT("Floor104"), FVector(5000,4080,-10), FVector(1600,240,20));
    B.Box(TEXT("Floor105"), FVector(2900,4400,-10), FVector(1400,400,20));
    B.Box(TEXT("Floor106"), FVector(4700,4400,-10), FVector(320,400,20));
    B.Box(TEXT("Floor107"), FVector(4700,5000,-10), FVector(320,800,20));
    B.Box(TEXT("Floor108"), FVector(4700,6000,-10), FVector(1000,1200,20));
    B.Box(TEXT("Floor109"), FVector(4375,6700,-10), FVector(350,200,20));
    B.Box(TEXT("Floor110"), FVector(5025,6700,-10), FVector(350,200,20));
    B.Box(TEXT("Boundary000"), FVector(-4410,3100,150), FVector(20,1400,500));
    B.Box(TEXT("Boundary001"), FVector(-2790,2620,150), FVector(20,440,500));
    B.Box(TEXT("Boundary002"), FVector(-2790,3480,150), FVector(20,640,500));
    B.Box(TEXT("Boundary003"), FVector(-2410,2620,150), FVector(20,440,500));
    B.Box(TEXT("Boundary004"), FVector(-2410,3380,150), FVector(20,440,500));
    B.Box(TEXT("Boundary005"), FVector(-1190,2620,150), FVector(20,440,500));
    B.Box(TEXT("Boundary006"), FVector(-1190,3380,150), FVector(20,440,500));
    B.Box(TEXT("Boundary007"), FVector(-1010,300,150), FVector(20,1800,500));
    B.Box(TEXT("Boundary008"), FVector(-810,2520,150), FVector(20,640,500));
    B.Box(TEXT("Boundary009"), FVector(-810,3480,150), FVector(20,640,500));
    B.Box(TEXT("Boundary010"), FVector(-170,1700,150), FVector(20,1000,500));
    B.Box(TEXT("Boundary011"), FVector(170,1700,150), FVector(20,1000,500));
    B.Box(TEXT("Boundary012"), FVector(610,-480,150), FVector(20,240,500));
    B.Box(TEXT("Boundary013"), FVector(610,300,150), FVector(20,680,500));
    B.Box(TEXT("Boundary014"), FVector(610,1080,150), FVector(20,240,500));
    B.Box(TEXT("Boundary015"), FVector(790,1600,150), FVector(20,800,500));
    B.Box(TEXT("Boundary016"), FVector(990,-780,150), FVector(20,840,500));
    B.Box(TEXT("Boundary017"), FVector(990,280,150), FVector(20,640,500));
    B.Box(TEXT("Boundary018"), FVector(1010,2720,150), FVector(20,1040,500));
    B.Box(TEXT("Boundary019"), FVector(1010,3680,150), FVector(20,240,500));
    B.Box(TEXT("Boundary020"), FVector(1130,1080,150), FVector(20,240,500));
    B.Box(TEXT("Boundary021"), FVector(1430,2840,150), FVector(20,800,500));
    B.Box(TEXT("Boundary022"), FVector(1470,920,150), FVector(20,560,500));
    B.Box(TEXT("Boundary023"), FVector(1770,3160,150), FVector(20,800,500));
    B.Box(TEXT("Boundary024"), FVector(1810,1600,150), FVector(20,800,500));
    B.Box(TEXT("Boundary025"), FVector(2190,3900,150), FVector(20,1400,500));
    B.Box(TEXT("Boundary026"), FVector(2410,-680,150), FVector(20,1040,500));
    B.Box(TEXT("Boundary027"), FVector(2410,380,150), FVector(20,440,500));
    B.Box(TEXT("Boundary028"), FVector(2430,2980,150), FVector(20,440,500));
    B.Box(TEXT("Boundary029"), FVector(2770,2820,150), FVector(20,760,500));
    B.Box(TEXT("Boundary030"), FVector(2790,-180,150), FVector(20,40,500));
    B.Box(TEXT("Boundary031"), FVector(2790,680,150), FVector(20,1040,500));
    B.Box(TEXT("Boundary032"), FVector(3030,1360,150), FVector(20,320,500));
    B.Box(TEXT("Boundary033"), FVector(3370,1360,150), FVector(20,320,500));
    B.Box(TEXT("Boundary034"), FVector(3610,3420,150), FVector(20,440,500));
    B.Box(TEXT("Boundary035"), FVector(3610,4280,150), FVector(20,640,500));
    B.Box(TEXT("Boundary036"), FVector(4190,3020,150), FVector(20,1240,500));
    B.Box(TEXT("Boundary037"), FVector(4190,4080,150), FVector(20,240,500));
    B.Box(TEXT("Boundary038"), FVector(4190,6100,150), FVector(20,1400,500));
    B.Box(TEXT("Boundary039"), FVector(4210,120,150), FVector(20,640,500));
    B.Box(TEXT("Boundary040"), FVector(4210,980,150), FVector(20,440,500));
    B.Box(TEXT("Boundary041"), FVector(4530,4800,150), FVector(20,1200,500));
    B.Box(TEXT("Boundary042"), FVector(4530,7060,150), FVector(20,320,500));
    B.Box(TEXT("Boundary043"), FVector(4540,6850,150), FVector(20,100,500));
    B.Box(TEXT("Boundary044"), FVector(4830,1580,150), FVector(20,1640,500));
    B.Box(TEXT("Boundary045"), FVector(4860,6850,150), FVector(20,100,500));
    B.Box(TEXT("Boundary046"), FVector(4870,4800,150), FVector(20,1200,500));
    B.Box(TEXT("Boundary047"), FVector(4870,7060,150), FVector(20,320,500));
    B.Box(TEXT("Boundary048"), FVector(5170,1420,150), FVector(20,1960,500));
    B.Box(TEXT("Boundary049"), FVector(5210,6100,150), FVector(20,1400,500));
    B.Box(TEXT("Boundary050"), FVector(5810,3300,150), FVector(20,1800,500));
    B.Box(TEXT("Boundary051"), FVector(1700,-1210,150), FVector(1400,20,500));
    B.Box(TEXT("Boundary052"), FVector(-200,-610,150), FVector(1600,20,500));
    B.Box(TEXT("Boundary053"), FVector(800,-370,150), FVector(400,20,500));
    B.Box(TEXT("Boundary054"), FVector(3500,-210,150), FVector(1400,20,500));
    B.Box(TEXT("Boundary055"), FVector(2600,-170,150), FVector(400,20,500));
    B.Box(TEXT("Boundary056"), FVector(800,-30,150), FVector(400,20,500));
    B.Box(TEXT("Boundary057"), FVector(2600,170,150), FVector(400,20,500));
    B.Box(TEXT("Boundary058"), FVector(4680,430,150), FVector(960,20,500));
    B.Box(TEXT("Boundary059"), FVector(1700,610,150), FVector(1400,20,500));
    B.Box(TEXT("Boundary060"), FVector(1030,630,150), FVector(860,20,500));
    B.Box(TEXT("Boundary061"), FVector(4520,770,150), FVector(640,20,500));
    B.Box(TEXT("Boundary062"), FVector(870,970,150), FVector(540,20,500));
    B.Box(TEXT("Boundary063"), FVector(970,1190,150), FVector(340,20,500));
    B.Box(TEXT("Boundary064"), FVector(1630,1190,150), FVector(340,20,500));
    B.Box(TEXT("Boundary065"), FVector(-580,1210,150), FVector(840,20,500));
    B.Box(TEXT("Boundary066"), FVector(380,1210,150), FVector(440,20,500));
    B.Box(TEXT("Boundary067"), FVector(2920,1210,150), FVector(240,20,500));
    B.Box(TEXT("Boundary068"), FVector(3780,1210,150), FVector(840,20,500));
    B.Box(TEXT("Boundary069"), FVector(3200,1530,150), FVector(320,20,500));
    B.Box(TEXT("Boundary070"), FVector(1300,2010,150), FVector(1000,20,500));
    B.Box(TEXT("Boundary071"), FVector(-480,2190,150), FVector(640,20,500));
    B.Box(TEXT("Boundary072"), FVector(580,2190,150), FVector(840,20,500));
    B.Box(TEXT("Boundary073"), FVector(-3600,2390,150), FVector(1600,20,500));
    B.Box(TEXT("Boundary074"), FVector(-1800,2390,150), FVector(1200,20,500));
    B.Box(TEXT("Boundary075"), FVector(4520,2390,150), FVector(640,20,500));
    B.Box(TEXT("Boundary076"), FVector(5480,2390,150), FVector(640,20,500));
    B.Box(TEXT("Boundary077"), FVector(2100,2430,150), FVector(1320,20,500));
    B.Box(TEXT("Boundary078"), FVector(2100,2770,150), FVector(680,20,500));
    B.Box(TEXT("Boundary079"), FVector(-2600,2830,150), FVector(400,20,500));
    B.Box(TEXT("Boundary080"), FVector(-1000,2830,150), FVector(400,20,500));
    B.Box(TEXT("Boundary081"), FVector(-2600,3170,150), FVector(400,20,500));
    B.Box(TEXT("Boundary082"), FVector(-1000,3170,150), FVector(400,20,500));
    B.Box(TEXT("Boundary083"), FVector(2320,3190,150), FVector(240,20,500));
    B.Box(TEXT("Boundary084"), FVector(3180,3190,150), FVector(840,20,500));
    B.Box(TEXT("Boundary085"), FVector(1220,3230,150), FVector(440,20,500));
    B.Box(TEXT("Boundary086"), FVector(1380,3570,150), FVector(760,20,500));
    B.Box(TEXT("Boundary087"), FVector(-1800,3610,150), FVector(1200,20,500));
    B.Box(TEXT("Boundary088"), FVector(3900,3630,150), FVector(600,20,500));
    B.Box(TEXT("Boundary089"), FVector(-3600,3810,150), FVector(1600,20,500));
    B.Box(TEXT("Boundary090"), FVector(100,3810,150), FVector(1800,20,500));
    B.Box(TEXT("Boundary091"), FVector(3900,3970,150), FVector(600,20,500));
    B.Box(TEXT("Boundary092"), FVector(4370,4210,150), FVector(340,20,500));
    B.Box(TEXT("Boundary093"), FVector(5330,4210,150), FVector(940,20,500));
    B.Box(TEXT("Boundary094"), FVector(2900,4610,150), FVector(1400,20,500));
    B.Box(TEXT("Boundary095"), FVector(4370,5390,150), FVector(340,20,500));
    B.Box(TEXT("Boundary096"), FVector(5030,5390,150), FVector(340,20,500));
    B.Box(TEXT("Boundary097"), FVector(4375,6810,150), FVector(350,20,500));
    B.Box(TEXT("Boundary098"), FVector(5025,6810,150), FVector(350,20,500));
    B.Box(TEXT("Boundary099"), FVector(4545,6890,150), FVector(10,20,500));
    B.Box(TEXT("Boundary100"), FVector(4855,6890,150), FVector(10,20,500));
    B.Box(TEXT("Boundary101"), FVector(4700,7230,150), FVector(320,20,500));
    B.Box(TEXT("C01Door0A"), FVector(2800,-140,200), FVector(20,40,400));
    B.Box(TEXT("C01Door0B"), FVector(2800,140,200), FVector(20,40,400));
    B.Box(TEXT("C01Door0Lintel"), FVector(2800,0,350), FVector(20,240,100));
    B.Box(TEXT("C01Door1A"), FVector(2400,-140,200), FVector(20,40,400));
    B.Box(TEXT("C01Door1B"), FVector(2400,140,200), FVector(20,40,400));
    B.Box(TEXT("C01Door1Lintel"), FVector(2400,0,350), FVector(20,240,100));
    B.Box(TEXT("C02Door0A"), FVector(1000,-340,200), FVector(20,40,400));
    B.Box(TEXT("C02Door0B"), FVector(1000,-60,200), FVector(20,40,400));
    B.Box(TEXT("C02Door0Lintel"), FVector(1000,-200,350), FVector(20,240,100));
    B.Box(TEXT("C02Door1A"), FVector(600,-340,200), FVector(20,40,400));
    B.Box(TEXT("C02Door1B"), FVector(600,-60,200), FVector(20,40,400));
    B.Box(TEXT("C02Door1Lintel"), FVector(600,-200,350), FVector(20,240,100));
    B.Box(TEXT("C03Door0A"), FVector(-140,1200,200), FVector(40,20,400));
    B.Box(TEXT("C03Door0B"), FVector(140,1200,200), FVector(40,20,400));
    B.Box(TEXT("C03Door0Lintel"), FVector(0,1200,350), FVector(240,20,100));
    B.Box(TEXT("C03Door1A"), FVector(-140,2200,200), FVector(40,20,400));
    B.Box(TEXT("C03Door1B"), FVector(140,2200,200), FVector(40,20,400));
    B.Box(TEXT("C03Door1Lintel"), FVector(0,2200,350), FVector(240,20,100));
    B.Box(TEXT("C04Door0A"), FVector(1000,3260,200), FVector(20,40,400));
    B.Box(TEXT("C04Door0B"), FVector(1000,3540,200), FVector(20,40,400));
    B.Box(TEXT("C04Door0Lintel"), FVector(1000,3400,350), FVector(20,240,100));
    B.Box(TEXT("C04Door1A"), FVector(2460,3200,200), FVector(40,20,400));
    B.Box(TEXT("C04Door1B"), FVector(2740,3200,200), FVector(40,20,400));
    B.Box(TEXT("C04Door1Lintel"), FVector(2600,3200,350), FVector(240,20,100));
    B.Box(TEXT("C05Door0A"), FVector(3600,3660,200), FVector(20,40,400));
    B.Box(TEXT("C05Door0B"), FVector(3600,3940,200), FVector(20,40,400));
    B.Box(TEXT("C05Door0Lintel"), FVector(3600,3800,350), FVector(20,240,100));
    B.Box(TEXT("C05Door1A"), FVector(4200,3660,200), FVector(20,40,400));
    B.Box(TEXT("C05Door1B"), FVector(4200,3940,200), FVector(20,40,400));
    B.Box(TEXT("C05Door1Lintel"), FVector(4200,3800,350), FVector(20,240,100));
    B.Box(TEXT("C06Door0A"), FVector(4860,2400,200), FVector(40,20,400));
    B.Box(TEXT("C06Door0B"), FVector(5140,2400,200), FVector(40,20,400));
    B.Box(TEXT("C06Door0Lintel"), FVector(5000,2400,350), FVector(240,20,100));
    B.Box(TEXT("C06Door1A"), FVector(4200,460,200), FVector(20,40,400));
    B.Box(TEXT("C06Door1B"), FVector(4200,740,200), FVector(20,40,400));
    B.Box(TEXT("C06Door1Lintel"), FVector(4200,600,350), FVector(20,240,100));
    B.Box(TEXT("C07Door0A"), FVector(4560,4200,200), FVector(40,20,400));
    B.Box(TEXT("C07Door0B"), FVector(4840,4200,200), FVector(40,20,400));
    B.Box(TEXT("C07Door0Lintel"), FVector(4700,4200,350), FVector(240,20,100));
    B.Box(TEXT("C07Door1A"), FVector(4560,5400,200), FVector(40,20,400));
    B.Box(TEXT("C07Door1B"), FVector(4840,5400,200), FVector(40,20,400));
    B.Box(TEXT("C07Door1Lintel"), FVector(4700,5400,350), FVector(240,20,100));
    B.Box(TEXT("C08Door0A"), FVector(-800,2860,200), FVector(20,40,400));
    B.Box(TEXT("C08Door0B"), FVector(-800,3140,200), FVector(20,40,400));
    B.Box(TEXT("C08Door0Lintel"), FVector(-800,3000,350), FVector(20,240,100));
    B.Box(TEXT("C08Door1A"), FVector(-1200,2860,200), FVector(20,40,400));
    B.Box(TEXT("C08Door1B"), FVector(-1200,3140,200), FVector(20,40,400));
    B.Box(TEXT("C08Door1Lintel"), FVector(-1200,3000,350), FVector(20,240,100));
    B.Box(TEXT("C09Door0A"), FVector(-2400,2860,200), FVector(20,40,400));
    B.Box(TEXT("C09Door0B"), FVector(-2400,3140,200), FVector(20,40,400));
    B.Box(TEXT("C09Door0Lintel"), FVector(-2400,3000,350), FVector(20,240,100));
    B.Box(TEXT("C09Door1A"), FVector(-2800,2860,200), FVector(20,40,400));
    B.Box(TEXT("C09Door1B"), FVector(-2800,3140,200), FVector(20,40,400));
    B.Box(TEXT("C09Door1Lintel"), FVector(-2800,3000,350), FVector(20,240,100));
    B.Box(TEXT("C10Door0A"), FVector(600,660,200), FVector(20,40,400));
    B.Box(TEXT("C10Door0B"), FVector(600,940,200), FVector(20,40,400));
    B.Box(TEXT("C10Door0Lintel"), FVector(600,800,350), FVector(20,240,100));
    B.Box(TEXT("C10Door1A"), FVector(1160,1200,200), FVector(40,20,400));
    B.Box(TEXT("C10Door1B"), FVector(1440,1200,200), FVector(40,20,400));
    B.Box(TEXT("C10Door1Lintel"), FVector(1300,1200,350), FVector(240,20,100));
    B.Box(TEXT("CellWestA"), FVector(1390,-535,200), FVector(20,130,400));
    B.Box(TEXT("CellWestB"), FVector(1390,-165,200), FVector(20,130,400));
    B.Box(TEXT("CellWestLintel"), FVector(1390,-350,350), FVector(20,240,100));
    B.Box(TEXT("CellEast"), FVector(1910,-350,200), FVector(20,500,400));
    B.Box(TEXT("CellSouth"), FVector(1650,-610,200), FVector(500,20,400));
    B.Box(TEXT("CellNorth"), FVector(1650,-90,200), FVector(500,20,400));
    B.Box(TEXT("EastRecessWestA"), FVector(2690,3740,200), FVector(20,80,400));
    B.Box(TEXT("EastRecessWestB"), FVector(2690,4060,200), FVector(20,80,400));
    B.Box(TEXT("EastRecessWestLintel"), FVector(2690,3900,350), FVector(20,240,100));
    B.Box(TEXT("EastRecessEast"), FVector(3110,3900,200), FVector(20,400,400));
    B.Box(TEXT("EastRecessSouth"), FVector(2900,3690,200), FVector(400,20,400));
    B.Box(TEXT("EastRecessNorth"), FVector(2900,4110,200), FVector(400,20,400));
    B.Box(TEXT("WestStubA"), FVector(-350,400,200), FVector(500,20,400));
    B.Box(TEXT("WestStubB"), FVector(-100,600,200), FVector(20,400,400));
    B.Box(TEXT("LowerRightDivider"), FVector(4900,3150,200), FVector(20,500,400));
    B.Box(TEXT("PaleRepair"), FVector(2000,-500,1), FVector(120,120,2), false);
    B.Box(TEXT("BranchEndStain"), FVector(-3700,3300,1), FVector(200,200,2), false);
    B.Ramp(TEXT("ReturnRamp"), FVector(3200,950,0), FVector(3200,1200,80),300);
    B.Box(TEXT("ReturnLanding"), FVector(3200,1360,70), FVector(320,320,20));
    B.Ramp(TEXT("DescentRamp"), FVector(4700,6600,0), FVector(4700,6900,-80),300);
    B.Box(TEXT("DescentLanding"), FVector(4700,7060,-90), FVector(320,320,20));
    B.Light(TEXT("T01"), FVector(2900,1000,250), 'N');
    B.Light(TEXT("T02"), FVector(2360,-600,250), 'T');
    B.Light(TEXT("T03"), FVector(1860,-350,250), 'T', true);
    B.Light(TEXT("T04"), FVector(-960,600,250), 'T');
    B.Light(TEXT("T05"), FVector(1760,1700,250), 'T');
    B.Light(TEXT("T06"), FVector(130,1700,250), 'T');
    B.Light(TEXT("T07"), FVector(100,3760,250), 'T');
    B.Light(TEXT("T08"), FVector(-1800,3560,250), 'T');
    B.Light(TEXT("T09"), FVector(-4360,3300,250), 'T');
    B.Light(TEXT("T10"), FVector(1470,2600,250), 'N');
    B.Light(TEXT("T11"), FVector(2730,3000,250), 'N');
    B.Light(TEXT("T12"), FVector(2900,4560,250), 'T');
    B.Light(TEXT("T13"), FVector(5760,3400,250), 'T');
    B.Light(TEXT("T14"), FVector(5130,600,250), 'T');
    B.Light(TEXT("T15"), FVector(4830,4800,250), 'T');
    B.Light(TEXT("T16"), FVector(5000,6600,250), 'W');
    B.Light(TEXT("T17"), FVector(4240,6000,250), 'N');
}
void Geometry4(FBuilder& B)
{
    B.Actor<ATargetPoint>(TEXT("B4.Entry"),FVector(600,600,0));
    B.Actor<ATargetPoint>(TEXT("B4.Hall"),FVector(1100,3200,0));
    B.Actor<ATargetPoint>(TEXT("B4.WestChamber"),FVector(-1800,3400,0));
    B.Actor<ATargetPoint>(TEXT("B4.EastChamber"),FVector(4500,3400,0));
    B.Actor<ATargetPoint>(TEXT("B4.BalorkArena"),FVector(1400,6800,0));
    B.Actor<ATargetPoint>(TEXT("B4.Reliquary"),FVector(3520,6100,0));
    B.Actor<ATargetPoint>(TEXT("B4.Ossuary"),FVector(3720,7620,0));
    B.Box(TEXT("Floor000"), FVector(600,220,-10), FVector(1200,440,20));
    B.Box(TEXT("Floor001"), FVector(600,445,-10), FVector(1200,10,20));
    B.Box(TEXT("Floor002"), FVector(725,600,-10), FVector(950,300,20));
    B.Box(TEXT("Floor003"), FVector(600,755,-10), FVector(1200,10,20));
    B.Box(TEXT("Floor004"), FVector(600,980,-10), FVector(1200,440,20));
    B.Box(TEXT("Floor005"), FVector(900,1320,-10), FVector(320,240,20));
    B.Box(TEXT("Floor006"), FVector(600,1520,-10), FVector(920,160,20));
    B.Box(TEXT("Floor007"), FVector(600,1680,-10), FVector(920,160,20));
    B.Box(TEXT("Floor008"), FVector(300,1980,-10), FVector(320,440,20));
    B.Box(TEXT("Floor009"), FVector(1100,2300,-10), FVector(2200,200,20));
    B.Box(TEXT("Floor010"), FVector(-1800,2720,-10), FVector(2000,640,20));
    B.Box(TEXT("Floor011"), FVector(1100,2720,-10), FVector(2200,640,20));
    B.Box(TEXT("Floor012"), FVector(4500,2720,-10), FVector(2200,640,20));
    B.Box(TEXT("Floor013"), FVector(-1800,3090,-10), FVector(2000,100,20));
    B.Box(TEXT("Floor014"), FVector(2800,3090,-10), FVector(5600,100,20));
    B.Box(TEXT("Floor015"), FVector(1400,3250,-10), FVector(8400,220,20));
    B.Box(TEXT("Floor016"), FVector(-300,3410,-10), FVector(5000,100,20));
    B.Box(TEXT("Floor017"), FVector(4500,3410,-10), FVector(2200,100,20));
    B.Box(TEXT("Floor018"), FVector(-1800,3830,-10), FVector(2000,740,20));
    B.Box(TEXT("Floor019"), FVector(1100,3830,-10), FVector(2200,740,20));
    B.Box(TEXT("Floor020"), FVector(4500,3830,-10), FVector(2200,740,20));
    B.Box(TEXT("Floor021"), FVector(-1800,4300,-10), FVector(2000,200,20));
    B.Box(TEXT("Floor022"), FVector(1100,4300,-10), FVector(550,200,20));
    B.Box(TEXT("Floor023"), FVector(4500,4300,-10), FVector(2200,200,20));
    B.Box(TEXT("Floor024"), FVector(1100,4900,-10), FVector(550,1000,20));
    B.Box(TEXT("Floor025"), FVector(2110,6100,-10), FVector(4220,1400,20));
    B.Box(TEXT("Floor026"), FVector(2110,6810,-10), FVector(4220,20,20));
    B.Box(TEXT("Floor027"), FVector(2310,7510,-10), FVector(4620,1380,20));
    B.Box(TEXT("Floor028"), FVector(3720,8310,-10), FVector(1800,220,20));
    B.Box(TEXT("Boundary000"), FVector(-2810,3400,150), FVector(20,2000,500));
    B.Box(TEXT("Boundary001"), FVector(-790,2770,150), FVector(20,740,500));
    B.Box(TEXT("Boundary002"), FVector(-790,3930,150), FVector(20,940,500));
    B.Box(TEXT("Boundary003"), FVector(-380,600,150), FVector(20,320,500));
    B.Box(TEXT("Boundary004"), FVector(-40,445,150), FVector(20,10,500));
    B.Box(TEXT("Boundary005"), FVector(-40,755,150), FVector(20,10,500));
    B.Box(TEXT("Boundary006"), FVector(-10,225,150), FVector(20,450,500));
    B.Box(TEXT("Boundary007"), FVector(-10,975,150), FVector(20,450,500));
    B.Box(TEXT("Boundary008"), FVector(-10,2670,150), FVector(20,940,500));
    B.Box(TEXT("Boundary009"), FVector(-10,3830,150), FVector(20,740,500));
    B.Box(TEXT("Boundary010"), FVector(-10,6800,150), FVector(20,2800,500));
    B.Box(TEXT("Boundary011"), FVector(130,1820,150), FVector(20,760,500));
    B.Box(TEXT("Boundary012"), FVector(470,1980,150), FVector(20,440,500));
    B.Box(TEXT("Boundary013"), FVector(730,1320,150), FVector(20,240,500));
    B.Box(TEXT("Boundary014"), FVector(815,4800,150), FVector(20,1200,500));
    B.Box(TEXT("Boundary015"), FVector(1070,1480,150), FVector(20,560,500));
    B.Box(TEXT("Boundary016"), FVector(1210,600,150), FVector(20,1200,500));
    B.Box(TEXT("Boundary017"), FVector(1385,4800,150), FVector(20,1200,500));
    B.Box(TEXT("Boundary018"), FVector(2210,2620,150), FVector(20,840,500));
    B.Box(TEXT("Boundary019"), FVector(2210,3780,150), FVector(20,840,500));
    B.Box(TEXT("Boundary020"), FVector(2810,8310,150), FVector(20,220,500));
    B.Box(TEXT("Boundary021"), FVector(3390,2720,150), FVector(20,640,500));
    B.Box(TEXT("Boundary022"), FVector(3390,3880,150), FVector(20,1040,500));
    B.Box(TEXT("Boundary023"), FVector(4230,6110,150), FVector(20,1420,500));
    B.Box(TEXT("Boundary024"), FVector(4630,7620,150), FVector(20,1600,500));
    B.Box(TEXT("Boundary025"), FVector(5610,3400,150), FVector(20,2000,500));
    B.Box(TEXT("Boundary026"), FVector(600,-10,150), FVector(1200,20,500));
    B.Box(TEXT("Boundary027"), FVector(-210,430,150), FVector(320,20,500));
    B.Box(TEXT("Boundary028"), FVector(-25,440,150), FVector(50,20,500));
    B.Box(TEXT("Boundary029"), FVector(-25,760,150), FVector(50,20,500));
    B.Box(TEXT("Boundary030"), FVector(-210,770,150), FVector(320,20,500));
    B.Box(TEXT("Boundary031"), FVector(370,1210,150), FVector(740,20,500));
    B.Box(TEXT("Boundary032"), FVector(1130,1210,150), FVector(140,20,500));
    B.Box(TEXT("Boundary033"), FVector(440,1430,150), FVector(600,20,500));
    B.Box(TEXT("Boundary034"), FVector(760,1770,150), FVector(600,20,500));
    B.Box(TEXT("Boundary035"), FVector(70,2190,150), FVector(140,20,500));
    B.Box(TEXT("Boundary036"), FVector(1330,2190,150), FVector(1740,20,500));
    B.Box(TEXT("Boundary037"), FVector(-1800,2390,150), FVector(2000,20,500));
    B.Box(TEXT("Boundary038"), FVector(4500,2390,150), FVector(2200,20,500));
    B.Box(TEXT("Boundary039"), FVector(2800,3030,150), FVector(1200,20,500));
    B.Box(TEXT("Boundary040"), FVector(-400,3130,150), FVector(800,20,500));
    B.Box(TEXT("Boundary041"), FVector(2800,3370,150), FVector(1200,20,500));
    B.Box(TEXT("Boundary042"), FVector(-400,3470,150), FVector(800,20,500));
    B.Box(TEXT("Boundary043"), FVector(412.5,4210,150), FVector(825,20,500));
    B.Box(TEXT("Boundary044"), FVector(1787.5,4210,150), FVector(825,20,500));
    B.Box(TEXT("Boundary045"), FVector(-1800,4410,150), FVector(2000,20,500));
    B.Box(TEXT("Boundary046"), FVector(4500,4410,150), FVector(2200,20,500));
    B.Box(TEXT("Boundary047"), FVector(412.5,5390,150), FVector(825,20,500));
    B.Box(TEXT("Boundary048"), FVector(2797.5,5390,150), FVector(2845,20,500));
    B.Box(TEXT("Boundary049"), FVector(4420,6810,150), FVector(400,20,500));
    B.Box(TEXT("Boundary050"), FVector(1410,8210,150), FVector(2820,20,500));
    B.Box(TEXT("Boundary051"), FVector(3720,8430,150), FVector(1800,20,500));
    B.Box(TEXT("C01Door0A"), FVector(760,1200,200), FVector(40,20,400));
    B.Box(TEXT("C01Door0B"), FVector(1040,1200,200), FVector(40,20,400));
    B.Box(TEXT("C01Door0Lintel"), FVector(900,1200,350), FVector(240,20,100));
    B.Box(TEXT("C01Door1A"), FVector(160,2200,200), FVector(40,20,400));
    B.Box(TEXT("C01Door1B"), FVector(440,2200,200), FVector(40,20,400));
    B.Box(TEXT("C01Door1Lintel"), FVector(300,2200,350), FVector(240,20,100));
    B.Box(TEXT("C02Door0A"), FVector(0,3160,200), FVector(20,40,400));
    B.Box(TEXT("C02Door0B"), FVector(0,3440,200), FVector(20,40,400));
    B.Box(TEXT("C02Door0Lintel"), FVector(0,3300,350), FVector(20,240,100));
    B.Box(TEXT("C02Door1A"), FVector(-800,3160,200), FVector(20,40,400));
    B.Box(TEXT("C02Door1B"), FVector(-800,3440,200), FVector(20,40,400));
    B.Box(TEXT("C02Door1Lintel"), FVector(-800,3300,350), FVector(20,240,100));
    B.Box(TEXT("C03Door0A"), FVector(2200,3060,200), FVector(20,40,400));
    B.Box(TEXT("C03Door0B"), FVector(2200,3340,200), FVector(20,40,400));
    B.Box(TEXT("C03Door0Lintel"), FVector(2200,3200,350), FVector(20,240,100));
    B.Box(TEXT("C03Door1A"), FVector(3400,3060,200), FVector(20,40,400));
    B.Box(TEXT("C03Door1B"), FVector(3400,3340,200), FVector(20,40,400));
    B.Box(TEXT("C03Door1Lintel"), FVector(3400,3200,350), FVector(20,240,100));
    B.Box(TEXT("C04Door0A"), FVector(837.5,4200,200), FVector(25,20,400));
    B.Box(TEXT("C04Door0B"), FVector(1362.5,4200,200), FVector(25,20,400));
    B.Box(TEXT("C04Door0Lintel"), FVector(1100,4200,380), FVector(500,20,40));
    B.Box(TEXT("C04Door1A"), FVector(837.5,5400,200), FVector(25,20,400));
    B.Box(TEXT("C04Door1B"), FVector(1362.5,5400,200), FVector(25,20,400));
    B.Box(TEXT("C04Door1Lintel"), FVector(1100,5400,380), FVector(500,20,40));
    B.Box(TEXT("ArenaPartitionLowA"), FVector(2810,5625,200), FVector(20,450,400));
    B.Box(TEXT("ArenaPartitionLowB"), FVector(2810,6580,200), FVector(20,460,400));
    B.Box(TEXT("ArenaPartitionLowLintel"), FVector(2810,6100,380), FVector(20,500,40));
    B.Box(TEXT("ArenaPartitionHighA"), FVector(2810,7080,200), FVector(20,540,400));
    B.Box(TEXT("ArenaPartitionHighB"), FVector(2810,8025,200), FVector(20,350,400));
    B.Box(TEXT("ArenaPartitionHighLintel"), FVector(2810,7600,380), FVector(20,500,40));
    B.Box(TEXT("ReliquaryPartitionA"), FVector(3035,6810,200), FVector(430,20,400));
    B.Box(TEXT("ReliquaryPartitionB"), FVector(3985,6810,200), FVector(470,20,400));
    B.Box(TEXT("ReliquaryPartitionLintel"), FVector(3500,6810,380), FVector(500,20,40));
    B.Box(TEXT("EastBaffle"), FVector(4000,3200,200), FVector(20,1200,400));
    B.Box(TEXT("BlockedPlinth"), FVector(3700,2500,60), FVector(600,200,120));
    B.Box(TEXT("Altar"), FVector(1600,7200,40), FVector(400,400,80));
    B.Box(TEXT("AltarTop"), FVector(1600,7200,100), FVector(200,200,40));
    B.Box(TEXT("DarkFloorFeature"), FVector(1000,2950,1), FVector(400,300,2), false);
    B.Box(TEXT("PaleUpright0"), FVector(4560,8250,45), FVector(40,40,90), false);
    B.Box(TEXT("PaleUpright1"), FVector(4400,8350,45), FVector(40,40,90), false);
    B.Box(TEXT("PaleUpright2"), FVector(5480,4250,45), FVector(40,40,90), false);
    B.Ramp(TEXT("ReturnRamp"), FVector(250,600,0), FVector(-50,600,80),300);
    B.Box(TEXT("ReturnLanding"), FVector(-210,600,70), FVector(320,320,20));
    B.Light(TEXT("T01"), FVector(100,1000,250), 'N');
    B.Light(TEXT("T02"), FVector(300,1730,250), 'T');
    B.Light(TEXT("T03"), FVector(40,2700,250), 'T');
    B.Light(TEXT("T04"), FVector(2160,3800,250), 'T');
    B.Light(TEXT("T05"), FVector(-2700,3400,250), 'T');
    B.Light(TEXT("T06"), FVector(5500,3500,250), 'T');
    B.Light(TEXT("T07"), FVector(3900,4360,250), 'N');
    B.Light(TEXT("T08"), FVector(855,5200,250), 'T');
    B.Light(TEXT("T09"), FVector(1345,5200,250), 'T');
    B.Light(TEXT("T10"), FVector(1600,8100,250), 'W');
    B.Light(TEXT("F01"), FVector(1200,6600,600), 'F');
    B.Light(TEXT("T11"), FVector(4180,6100,250), 'T');
    B.Light(TEXT("T12"), FVector(4580,7900,250), 'T');
}
}
ULHGenerateBasementBMapsCommandlet::ULHGenerateBasementBMapsCommandlet()
{
    IsClient=false; IsServer=false; IsEditor=true; LogToConsole=true;
}
int32 ULHGenerateBasementBMapsCommandlet::Main(const FString& Params)
{
    using namespace LHBasementBGenerator;
    TArray<FString> Errors;
    if (!LHWorld::ValidateRegistry(Errors))
    { for (const auto& E : Errors) UE_LOG(LogTemp, Error, TEXT("%s"), *E); return 1; }
    auto* Cube = LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!GEditor || !Cube) return 1;
    for (const bool bB3 : {true,false})
    {
        FLHAreaId Id; Id.Content.Value=bB3 ? TEXT("Area.TempleB3") : TEXT("Area.TempleB4");
        const auto* Area=LHWorld::FindArea(Id);
        if (!Area || Area->Spawns.Num()!=(bB3 ? 22 : 13) || Area->Entrances.Num()!=(bB3 ? 2 : 1)
            || Area->Portals.Num()!=(bB3 ? 2 : 1)) return 1;
        for (const auto& Spawn : Area->Spawns)
        {
            const FName Enemy=Spawn.Enemy.Value;
            const bool bCommon=Enemy==TEXT("Enemy.BrownRat") || Enemy==TEXT("Enemy.GreenSlime")
                || Enemy==TEXT("Enemy.GiantBat") || Enemy==TEXT("Enemy.Atrocity");
            const bool bFloor=bB3 ? (Enemy==TEXT("Enemy.Goblin") || Enemy==TEXT("Enemy.GoblinWarrior")) : Enemy==TEXT("Enemy.Balork");
            if (!bCommon && !bFloor) { UE_LOG(LogTemp,Error,TEXT("Unassigned species: %s"),*Enemy.ToString()); return 1; }
        }
        UWorld* World=GEditor->NewMap(false);
        if (!World) return 1;
        FBuilder B{World,Cube,true,{}};
        B.Common(bB3,*Area);
        if (bB3) Geometry3(B); else Geometry4(B);
        B.CoverageLighting();
        B.bOK &= LHMapDressing::Dress(World,bB3?ELHVisualStyle::B3Crypt:ELHVisualStyle::B4Ritual,
            bB3?TArray<FVector>{{3900,-200,1000},{-600,1600,1000},{5300,5400,1000}}:TArray<FVector>{{1100,100,900},{1700,2500,1100},{3500,7100,1200}},
            bB3?TArray<FVector>{{2900,700,0},{400,2500,0},{4300,6400,0}}:TArray<FVector>{{600,900,0},{1000,3700,0},{1600,7400,100}});
        const FString Package=bB3 ? TEXT("/Game/Lighthaven/Maps/L_TempleB3") : TEXT("/Game/Lighthaven/Maps/L_TempleB4");
        const FString Filename=FPackageName::LongPackageNameToFilename(Package,FPackageName::GetMapPackageExtension());
        if (!B.bOK || !IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename),true) || !FEditorFileUtils::SaveMap(World,Filename)
            || !B.WriteManifest(bB3 ? TEXT("L_TempleB3") : TEXT("L_TempleB4"))) return 1;
        UE_LOG(LogTemp,Display,TEXT("Generated %s: %d spawns, %d directed portals"),*Package,Area->Spawns.Num(),Area->Portals.Num());
    }
    return 0;
}
