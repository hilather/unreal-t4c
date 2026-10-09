#include "LHGenerateBasementAMapsCommandlet.h"
#include "Editor.h"
#include "Framework/LHArrivalReview.h"
#include "EngineUtils.h"
#include "Misc/SecureHash.h"
#include "FileHelpers.h"
#include "ActorFactories/ActorFactory.h"
#include "Builders/CubeBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshCompiler.h"
#include "Engine/TextureCube.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Engine/TargetPoint.h"
#include "Engine/TriggerBox.h"
#include "Components/BoxComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "Framework/LHGameMode.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "World/LHAreaRegistry.h"
#include "World/LHWorldMarkers.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

namespace LHBasementAGeneration
{
// All geometry is V-01/A-02 prototype, U=100cm. Lighting is A-04 prototype.
struct FRect
{
    double X0, X1, Y0, Y1;
    bool Contains(double X, double Y) const { return X>X0 && X<X1 && Y>Y0 && Y<Y1; }
};
struct FBuilder
{
    UWorld* World;
    UStaticMesh* Cube;
    bool bOK = true;
    int32 GeometryIndex = 0;
    TArray<FRect> Floors, Holes, Ramps;
    template<class T> T* Actor(const FString& Name, FVector P, FRotator R = FRotator::ZeroRotator)
    {
        FActorSpawnParameters S; S.Name=FName(*Name.Replace(TEXT("."),TEXT("_")));
        S.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        T* A=World->SpawnActor<T>(P,R,S);
        if (!A) { bOK=false; return nullptr; }
        A->SetActorLabel(Name); A->Tags.Add(TEXT("LH.W3_02.Generated"));
        A->Tags.Add(TEXT("LH.Provenance.Prototype")); return A;
    }
    void Box(FVector P, FVector Size, bool Collision=true, FRotator R=FRotator::ZeroRotator, bool Visible=true)
    {
        if (auto* A=Actor<AStaticMeshActor>(FString::Printf(TEXT("Geometry_%04d"),GeometryIndex++),P*100,R))
        {
            auto* C=A->GetStaticMeshComponent(); C->SetMobility(EComponentMobility::Static);
            C->SetStaticMesh(Cube); C->SetCollisionProfileName(Collision?TEXT("BlockAll"):TEXT("NoCollision"));
            C->SetCanEverAffectNavigation(Collision); A->SetActorScale3D(Size);
            if(!Visible) { C->SetVisibility(false); A->SetActorHiddenInGame(true); }
        }
    }
    void Wall(FVector P,FVector Size)
    {
        Box(P,Size,true,FRotator::ZeroRotator,false);
        P.Z=.6; Size.Z=1.2; Box(P,Size,false); // Fixed cutaway; full collision retained.
    }
    bool Inside(double X,double Y) const
    {
        return Floors.ContainsByPredicate([&](const FRect& R){return R.Contains(X,Y);})
            && !Holes.ContainsByPredicate([&](const FRect& R){return R.Contains(X,Y);});
    }
    // Partition the exact floor union at all rectangle boundaries; walls only on exposed edges.
    void Geometry()
    {
        TArray<double> X,Y;
        for (const auto* List : {&Floors,&Holes,&Ramps}) for (const auto& R:*List)
        { X.AddUnique(R.X0); X.AddUnique(R.X1); Y.AddUnique(R.Y0); Y.AddUnique(R.Y1); }
        X.Sort(); Y.Sort();
        for(int32 I=0;I<X.Num()-1;++I) for(int32 J=0;J<Y.Num()-1;++J)
        {
            const double CX=(X[I]+X[I+1])/2,CY=(Y[J]+Y[J+1])/2, W=X[I+1]-X[I],H=Y[J+1]-Y[J];
            if(!Inside(CX,CY)) continue;
            if(!Ramps.ContainsByPredicate([&](const FRect& R){return R.Contains(CX,CY);}))
                Box({CX,CY,-.1},{W,H,.2});
            // Outside face at clear bound, no narrowing of listed mouths.
            if(!Inside(X[I]-.001,CY)) Wall({X[I]-.1,CY,1.25},{.2,H,5.5});
            if(!Inside(X[I+1]+.001,CY)) Wall({X[I+1]+.1,CY,1.25},{.2,H,5.5});
            if(!Inside(CX,Y[J]-.001)) Wall({CX,Y[J]-.1,1.25},{W,.2,5.5});
            if(!Inside(CX,Y[J+1]+.001)) Wall({CX,Y[J+1]+.1,1.25},{W,.2,5.5});
        }
    }
    void Stair(const FRect& R,double StartZ,double EndZ,const TCHAR* Name,bool TerminalWall=true)
    {
        const double Run=R.Y1-R.Y0,Rise=EndZ-StartZ, Angle=FMath::Atan2(Rise,Run);
        // Rotated slab top follows endpoints. Negative UE roll rotates +Y toward +Z.
        Box({(R.X0+R.X1)/2,(R.Y0+R.Y1)/2+.1*FMath::Sin(Angle),(StartZ+EndZ)/2-.1*FMath::Cos(Angle)},
            {R.X1-R.X0,FMath::Sqrt(Run*Run+Rise*Rise),.2},true,FRotator(0,0,-FMath::RadiansToDegrees(Angle)));
        Wall({R.X0-.1,(R.Y0+R.Y1)/2,1.25},{.2,Run,5.5});
        Wall({R.X1+.1,(R.Y0+R.Y1)/2,1.25},{.2,Run,5.5});
        const double Terminal=StartZ>0?R.Y0-.1:R.Y1+.1;
        if(TerminalWall) Wall({(R.X0+R.X1)/2,Terminal,1.25},{R.X1-R.X0,.2,5.5});
        // Nonblocking thin visual tread strips, never step collision above the smooth ramp.
        for(int32 I=0;I<12;++I)
        {
            const double T=(I+.5)/12.;
            Box({(R.X0+R.X1)/2,R.Y0+T*Run,StartZ+T*Rise+.015},{R.X1-R.X0,.04,.02},false);
        }
        Actor<ATargetPoint>(Name,{(R.X0+R.X1)*50,(R.Y0+R.Y1)*50,(StartZ+EndZ)*50});
    }
    void Light(int32 Index,double X,double Y,TCHAR Profile)
    {
        if(auto* A=Actor<APointLight>(FString::Printf(TEXT("A04_T%02d"),Index),{X*100,Y*100,250}))
        {
            auto* C=CastChecked<UPointLightComponent>(A->GetLightComponent());
            C->SetMobility(EComponentMobility::Movable); C->SetCastShadows(false);
            C->SetIntensityUnits(ELightUnits::Lumens); C->SetUseTemperature(true);
            C->SetTemperature(Profile=='N'?6500:2200);
            C->SetIntensity(Profile=='N'?1200:Profile=='W'?1000:600);
            C->SetAttenuationRadius(Profile=='N'?850:Profile=='W'?800:600);
        }
    }
    void Mark(const TCHAR* Name,FVector P,FVector Size,const TCHAR* Tag)
    {
        if(FString(Tag).Contains(TEXT("NoCombat")))
        {
            if(auto* A=Actor<ATriggerBox>(Name,(P+FVector(0,0,Size.Z/2))*100))
            {
                A->Tags.Add(Tag);
                auto* C=CastChecked<UBoxComponent>(A->GetCollisionComponent());
                C->SetBoxExtent(Size*50); C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                C->SetGenerateOverlapEvents(false); // Bounds for W4, no invented exclusion behavior.
            }
        }
        else if(auto* A=Actor<ATargetPoint>(Name,P*100)) { A->Tags.Add(Tag); A->SetActorScale3D(Size); }
    }
    void Authoring(const FLHAreaDefinition& Area,bool B1)
    {
        for(const auto& E:Area.Entrances)
        {
            if(auto* A=Actor<ALHEntranceMarker>(TEXT("Entrance_")+E.Id.LocalId.ToString(),E.SafeTransform.GetLocation(),E.SafeTransform.Rotator()))
            { A->EntranceId=E.Id; A->SafeArrivalTransform=E.SafeTransform; A->bSafetyReviewed=LHArrivalReview::IsReviewed(E); }
        }
        for(const auto& P:Area.Portals)
        {
            const bool Entry=P.Source.LocalId==TEXT("Entry");
            const FVector Position=B1?(Entry?FVector(450,-2500,100):FVector(4500,2100,-75))
                :(Entry?FVector(500,-6300,100):FVector(5300,6600,-100));
            if(auto* A=Actor<ALHPortal>(TEXT("Portal_")+P.Source.LocalId.ToString(),Position,FRotator(0,Entry?-90:90,0)))
            { A->PortalId=P.Portal.InstanceId; A->Source=P.Source; A->Destination=P.Destination;
              A->Direction=Entry?ELHPortalDirection::Return:ELHPortalDirection::Descent; }
        }
        if(Area.Spawns.Num()!=(B1?17:25) || Area.Portals.Num()!=2 || Area.Entrances.Num()!=2) bOK=false;
        for(const auto& S:Area.Spawns)
        {
            const FString Species=S.Enemy.Value.ToString();
            const bool Common=Species==TEXT("Enemy.BrownRat") || Species==TEXT("Enemy.Bat") || Species==TEXT("Enemy.GreenSlime");
            const bool B2Only=Species==TEXT("Enemy.GiantBat") || Species==TEXT("Enemy.UndeadBat") || Species==TEXT("Enemy.GiantSpider") || Species==TEXT("Enemy.DungeonBat");
            if((!Common && (B1 || !B2Only)) || !Inside(S.Anchor.GetLocation().X/100,S.Anchor.GetLocation().Y/100))
            { UE_LOG(LogTemp,Error,TEXT("Invalid floor roster/anchor: %s"),*S.Alias.ToString()); bOK=false; continue; }
            if(auto* A=Actor<ALHSpawnMarker>(S.Alias.ToString(),S.Anchor.GetLocation(),S.Anchor.Rotator()))
            {
                A->Area=Area.Id; A->SpawnId=S.SpawnId; A->EnemyDefinitionId=S.Enemy;
                if(S.Enemy.Value==TEXT("Enemy.DungeonBat")) A->Tags.Add(TEXT("LH.Placement.Provisional"));
                if(S.Enemy.Value==TEXT("Enemy.UndeadBat")) A->Tags.Add(TEXT("LH.Provenance.Disputed"));
            }
        }
        Actor<APlayerStart>(TEXT("PlayerStart"),Area.Entrances[0].SafeTransform.GetLocation()+FVector(0,0,100),Area.Entrances[0].SafeTransform.Rotator());
        if(auto* N=Actor<ANavMeshBoundsVolume>(TEXT("NavigationBounds"),{1500,0,100}))
        { auto* C=NewObject<UCubeBuilder>(); C->X=12000; C->Y=15000; C->Z=1200; UActorFactory::CreateBrushForVolumeActor(N,C); }
        if(auto* A=Actor<ASkyLight>(TEXT("NeutralFill"),{0,0,600}))
        {
            auto* C=A->GetLightComponent(); C->SetMobility(EComponentMobility::Movable);
            C->SetCastShadows(false); C->SetIntensity(B1?.5:.4);
            C->bRealTimeCapture=false; C->LowerHemisphereColor=FLinearColor::White;
            // Fixed engine cubemap, no external HDRI. Capture source is recorded in handoff.
            C->SourceType=SLS_SpecifiedCubemap;
            C->Cubemap=LoadObject<UTextureCube>(nullptr,TEXT("/Engine/EngineResources/GrayLightTextureCube.GrayLightTextureCube"));
            if(!C->Cubemap) bOK=false;
        }
        if(auto* A=Actor<APostProcessVolume>(TEXT("ExposureBaseline"),FVector::ZeroVector))
        {
            A->bUnbound=true; auto& S=A->Settings;
            S.bOverride_AutoExposureMinBrightness=true; S.AutoExposureMinBrightness=2;
            S.bOverride_AutoExposureMaxBrightness=true; S.AutoExposureMaxBrightness=2;
            S.bOverride_AutoExposureBias=true; S.AutoExposureBias=0;
            S.bOverride_AutoExposureMethod=true; S.AutoExposureMethod=AEM_Manual;
            S.bOverride_AutoExposureApplyPhysicalCameraExposure=true; S.AutoExposureApplyPhysicalCameraExposure=true;
            S.bOverride_CameraISO=true; S.CameraISO=100;
            S.bOverride_CameraShutterSpeed=true; S.CameraShutterSpeed=1;
            S.bOverride_DepthOfFieldFstop=true; S.DepthOfFieldFstop=2; // Prototype interior EV100 2: log2(2^2 *1).
            S.bOverride_BloomIntensity=true; S.BloomIntensity=0;
            S.bOverride_MotionBlurAmount=true; S.MotionBlurAmount=0;
        }
        World->GetWorldSettings()->DefaultGameMode=ALHGameMode::StaticClass();
    }
    bool CheckRoutes(bool B1)
    {
        // Positive control: an uninitialized physics scene must not report empty sweeps as passes.
        const FVector Probe=B1?FVector(900,900,0):FVector(1100,1000,0);
        FHitResult FloorHit;
        if(!World->GetPhysicsScene() || !World->LineTraceSingleByChannel(FloorHit,Probe+FVector(0,0,250),Probe-FVector(0,0,200),ECC_Pawn))
        { UE_LOG(LogTemp,Error,TEXT("No blocking floor/physics scene for clearance checks")); bOK=false; return false; }
        // Sweep the largest proposed capsule envelope along intended flat routes.
        const FCollisionShape Capsule=FCollisionShape::MakeCapsule(B1?40.f:55.f,B1?75.f:100.f);
        const double Z=B1?80:105;
        auto Check=[&](FVector From,FVector To)
        {
            From.Z=Z; To.Z=Z; FHitResult Hit;
            if(World->SweepSingleByChannel(Hit,From,To,FQuat::Identity,ECC_Pawn,Capsule))
            { UE_LOG(LogTemp,Error,TEXT("Route blocked by %s"),*GetNameSafe(Hit.GetActor())); bOK=false; }
        };
        auto Route=[&](std::initializer_list<FVector> Points)
        {
            bool First=true; FVector Prev;
            for(FVector P:Points) { P*=100; if(!First) Check(Prev,P); Prev=P; First=false; }
        };
        if(B1)
        {
            Route({{4.5,-16.5,0},{9,-16.5,0},{9,9,0},{-21,9,0}});
            Route({{9,9,0},{35,9,0},{35,5,0},{45,5,0},{45,14,0}});
            Route({{35,5,0},{52,5,0},{52,24,0},{35,24,0},{35,5,0}});
        }
        else
        {
            Route({{5,-54.5,0},{10,-54.5,0},{10,-30,0},{3.5,-30,0},{3.5,-16,0},{11,-16,0},{11,10,0}});
            Route({{10,-30,0},{18,-30,0},{18,-16,0},{11,-16,0}});
            Route({{11,10,0},{15,10,0},{15,38,0},{-1,38,0},{-1,56,0},{53,56,0},{53,57.5,0}});
            Route({{11,10,0},{-19,10,0},{-19,-23,0}});
            Route({{-19,10,0},{-14,10,0},{-14,42,0},{-30,42,0}});
            // Explicit forbidden shortcut probes and ring interior.
            for(const FVector P : {FVector(-9,47,0),FVector(11,-23,0),FVector(7,30,0)})
                if(Inside(P.X,P.Y)) bOK=false;
        }
        // Check approach-to-terminal slope clearance above the local ramp, not floor Z=0.
        for(int32 I=0;I<Ramps.Num();++I)
        {
            const auto& R=Ramps[I]; const bool Up=I==0;
            const double Rise=Up?1.2:(B1?1.:1.2);
            const double BeginY=Up?R.Y1+.5:R.Y0-.5, EndY=Up?R.Y0+1:R.Y1-1;
            const double EndZ=Up?Rise*(R.Y1-EndY)/(R.Y1-R.Y0):-Rise*(EndY-R.Y0)/(R.Y1-R.Y0);
            FHitResult Hit;
            const FVector From((R.X0+R.X1)*50,BeginY*100,Z), Mouth((R.X0+R.X1)*50,(Up?R.Y1:R.Y0)*100,Z), To((R.X0+R.X1)*50,EndY*100,Z+EndZ*100);
            if(World->SweepSingleByChannel(Hit,From,Mouth,FQuat::Identity,ECC_Pawn,Capsule)
                || World->SweepSingleByChannel(Hit,Mouth,To,FQuat::Identity,ECC_Pawn,Capsule))
            { UE_LOG(LogTemp,Error,TEXT("Stair route blocked by %s"),*GetNameSafe(Hit.GetActor())); bOK=false; }
        }
        if(bOK) UE_LOG(LogTemp,Display,TEXT("Native floor control, route and stair capsule checks passed for B%d"),B1?1:2);
        return bOK;
    }
    FString Fingerprint() const
    {
        TArray<FString> Rows;
        for(TActorIterator<AActor> It(World);It;++It)
        {
            AActor* A=*It; if(!A->Tags.Contains(TEXT("LH.W3_02.Generated"))) continue;
            FString Row=A->GetName()+TEXT("|")+A->GetClass()->GetName()+TEXT("|")+A->GetActorTransform().ToString();
            for(const FName Tag:A->Tags) Row+=TEXT("|")+Tag.ToString();
            if(auto* P=Cast<ALHPortal>(A)) Row+=P->PortalId.ToString()+P->Source.LocalId.ToString()+P->Destination.Area.Content.Value.ToString()+P->Destination.LocalId.ToString();
            if(auto* P=Cast<ALHSpawnMarker>(A)) Row+=P->SpawnId.ToString()+P->EnemyDefinitionId.Value.ToString();
            Rows.Add(Row);
        }
        Rows.Sort(); return FMD5::HashAnsiString(*FString::Join(Rows,TEXT("\n")));
    }
    void B1()
    {
        Floors={{0,18,-28,-10},{0,18,0,18},{-3,0,3,7},{18,21,13,17},{-32,-10,0,20},{30,54,0,26},
            {7.4,10.6,-10,0},{-10,0,7.4,10.6},{18,30,7.4,10.6}};
        Ramps={{3,6,-26,-20},{43.5,46.5,18,22}}; Geometry();
        // E01 internal enclosure: only low-Y aperture 43.8..46.2.
        Wall({39.9,16,1.25},{.2,12,5.5}); Wall({50.1,16,1.25},{.2,12,5.5});
        Wall({45,22.1,1.25},{10.4,.2,5.5});
        Wall({41.9,9.9,2},{3.8,.2,4}); Wall({48.1,9.9,2},{3.8,.2,4});
        Box({45,9.9,3.5},{2.4,.2,1});
        Stair(Ramps[0],1.2,0,TEXT("B1.S01")); Stair(Ramps[1],0,-1,TEXT("B1.S02"),false);
        Box({9,9,.01},{2,2,.02},false); // flush hub motif
        Mark(TEXT("B1.Safe.Entry"),{9,-19,0},{18,18,3},TEXT("LH.Safety.NoCombat.Required"));
        Mark(TEXT("B1.Safe.C01"),{9,-5,0},{3.2,10,3},TEXT("LH.Safety.NoCombat.Required"));
        Mark(TEXT("B1.Safe.Healers"),{-21,10,0},{22,20,3},TEXT("LH.Safety.NoCombat.Required"));
        Mark(TEXT("B1.Safe.C02"),{-5,9,0},{10,3.2,3},TEXT("LH.Safety.NoCombat.Required"));
        Mark(TEXT("B1.Safe.Enclosure"),{45,16,0},{10,12,3},TEXT("LH.Safety.NoCombat.Required"));
        Box({-12,12,1.7},{.8,.15,.15},false);
        Box({-12,12,1.7},{.15,.15,.8},false);
        Mark(TEXT("B1.NPC.Nevanis"),{-25,6,0},{1,1,1},TEXT("LH.Service.BindingRequired"));
        Mark(TEXT("B1.NPC.Shovanis"),{-25,14,0},{1,1,1},TEXT("LH.Service.BindingRequired"));
        Box({-25,6,.9},{.7,.7,1.8},false); Box({-25,14,.9},{.7,.7,1.8},false);
        Mark(TEXT("B1.Healer.ServiceCue"),{-12,12,0},{1,1,1},TEXT("LH.Landmark.Healer"));
        Light(1,2,-20,'N'); Light(2,7.7,-5,'T'); Light(3,9,.4,'T'); Light(4,.4,12,'T');
        Light(5,17.6,12,'T'); Light(6,-12,12,'W'); Light(7,-31.6,10,'N'); Light(8,24,10.3,'T');
        Light(9,40,10,'W'); Light(10,53.6,7,'T'); Light(11,33,25.6,'T'); Light(12,48.5,18,'N');
    }
    void B2()
    {
        Floors={{0,20,-66,-46},{0,22,-34,-12},{6,16,-6,0},{0,22,0,20},{-30,-8,0,20},
            {-30,-8,-32,-14},{-20,-8,32,46},{-40,-20,38,52},{-10,36,48,64},{48,70,48,68},
            {8.4,11.6,-46,-34},{9.4,12.6,-12,-6},{-8,0,8.4,11.6},{-20.6,-17.4,-14,0},
            {-15.6,-12.4,20,32},{13.4,16.6,20,39.6},{-2.6,16.6,36.4,39.6},{-2.6,.6,36.4,48},
            {36,48,54.4,57.6}};
        Holes={{7,15,-27,-19}}; Ramps={{3.5,6.5,-64,-58},{51.5,54.5,61,67}}; Geometry();
        // Void collision is full height; separate low visible coping on its inside edge.
        Box({7.1,-23,.6},{.2,8,1.2},false); Box({14.9,-23,.6},{.2,8,1.2},false);
        Box({11,-26.9,.6},{8,.2,1.2},false); Box({11,-19.1,.6},{8,.2,1.2},false);
        Stair(Ramps[0],1.2,0,TEXT("B2.S01")); Stair(Ramps[1],0,-1.2,TEXT("B2.S02"));
        Box({11,0,.01},{3,.5,.02},false);
        Mark(TEXT("B2.Safe.Entry"),{10,-56,0},{20,20,3},TEXT("LH.Safety.NoCombat.Required"));
        Mark(TEXT("B2.Safe.C01"),{10,-40,0},{3.2,12,3},TEXT("LH.Safety.NoCombat.Required"));
        Mark(TEXT("B2.Safe.Descent"),{59,58,0},{22,20,3},TEXT("LH.Safety.NoCombat.Required"));
        Mark(TEXT("B2.Safe.C08"),{42,56,0},{12,3.2,3},TEXT("LH.Safety.NoCombat.Required"));
        Box({-7.9,14,1},{.2,.8,2},false); // Branch pier silhouette, outside clear branch mouth.
        Light(1,2,-58,'N'); Light(2,8.7,-40,'T'); Light(3,2,-30,'T'); Light(4,20,-16,'T');
        Light(5,14,-.5,'N'); Light(6,21.6,12,'T'); Light(7,-8,14,'T'); Light(8,-17,19.6,'T');
        Light(9,-29.6,-24,'T'); Light(10,-8.4,35,'T'); Light(11,-39.6,45,'T');
        Light(12,16.3,38,'T'); Light(13,-2.3,38,'T'); Light(14,7,63.6,'T'); Light(15,25,48.4,'T');
        Light(16,36,59,'W'); Light(17,55,61,'N');
    }
};
}
ULHGenerateBasementAMapsCommandlet::ULHGenerateBasementAMapsCommandlet()
{ IsClient=false; IsServer=false; IsEditor=true; LogToConsole=true; }
int32 ULHGenerateBasementAMapsCommandlet::Main(const FString& Params)
{
    using namespace LHBasementAGeneration;
    UStaticMesh* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
    TArray<FString> Errors;
    if(!GEditor || !Cube || !LHWorld::ValidateRegistry(Errors)) return 1;
    // A freshly loaded engine mesh can still be compiling in a headless editor.
    // StaticMeshComponent suppresses physics creation until compilation finishes.
    FStaticMeshCompilingManager::Get().FinishCompilation({Cube});
    for(bool B1:{true,false})
    {
        const FString Package=B1?TEXT("/Game/Lighthaven/Maps/L_TempleB1"):TEXT("/Game/Lighthaven/Maps/L_TempleB2");
        const FString Filename=FPackageName::LongPackageNameToFilename(Package,FPackageName::GetMapPackageExtension());
        if(IFileManager::Get().IsReadOnly(*Filename)) { UE_LOG(LogTemp,Error,TEXT("Read-only map: %s"),*Filename); return 1; }
        FLHAreaId Id; Id.Content.Value=B1?TEXT("Area.TempleB1"):TEXT("Area.TempleB2");
        const auto* Area=LHWorld::FindArea(Id); if(!Area) return 1;
        UWorld* World=GEditor->NewMap(false); if(!World) return 1;
        FBuilder Builder{World,Cube}; if(B1) Builder.B1(); else Builder.B2(); Builder.Authoring(*Area,B1); Builder.CheckRoutes(B1);
        if(!Builder.bOK || !IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename),true) || !FEditorFileUtils::SaveMap(World,Filename)) return 1;
        UE_LOG(LogTemp,Display,TEXT("Generated %s: %d encounter anchors; safety unreviewed"),*Package,Area->Spawns.Num());
        UE_LOG(LogTemp,Display,TEXT("Authored actor identity/transform fingerprint %s: %s"),*Package,*Builder.Fingerprint());
    }
    return 0;
}
