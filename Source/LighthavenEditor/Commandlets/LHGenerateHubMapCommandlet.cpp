#include "LHGenerateHubMapCommandlet.h"
#include "Editor.h"
#include "Framework/LHArrivalReview.h"
#include "FileHelpers.h"
#include "ActorFactories/ActorFactory.h"
#include "Builders/CubeBuilder.h"
#include "World/LHAreaRegistry.h"
#include "World/LHWorldMarkers.h"
#include "Framework/LHGameMode.h"
#include "Framework/LHCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureCube.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/PostProcessVolume.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "EngineUtils.h"
#include "Misc/FileHelper.h"

// Unique namespace: editor module is unity-built. Every metric is Prototype.
namespace LHHubGenerator
{
struct FPolygon { TArray<FVector2D> Points; };
struct FWall { bool bX; double Fixed, Lo, Hi; TArray<double> Doors; };
struct FHub
{
    UWorld* World;
    UStaticMesh* Meshes[4]; // cube, sphere, cylinder, cone
    TMap<FString, UMaterial*> Materials;
    TArray<FPolygon> Footprints;
    TArray<FWall> Walls;
    bool bOK = true;
    int32 BoundaryCount = 0, WallCount = 0;

    template<class T> T* Actor(const FString& Name, FVector P, FRotator R = FRotator::ZeroRotator)
    {
        FActorSpawnParameters S; S.Name = FName(*Name.Replace(TEXT("."),TEXT("_")));
        S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        T* A = World->SpawnActor<T>(P, R, S);
        if (!A) { bOK = false; return nullptr; }
        A->SetActorLabel(Name);
        A->Tags.Append({TEXT("LH.Hub.Generated"), TEXT("LH.Geometry.Reconstructed"), TEXT("LH.Provenance.Prototype")});
        return A;
    }
    UMaterial* Material(const FString& Hex)
    {
        if (auto* Found = Materials.Find(Hex)) return *Found;
        UMaterial* M = NewObject<UMaterial>(World, *FString::Printf(TEXT("M_Hub_%s"), *Hex), RF_Public);
        auto* C = NewObject<UMaterialExpressionConstant3Vector>(M);
        C->Constant = FLinearColor::FromSRGBColor(FColor::FromHex(Hex));
        M->GetExpressionCollection().AddExpression(C);
        M->GetEditorOnlyData()->BaseColor.Expression = C;
        auto* Rough = NewObject<UMaterialExpressionConstant>(M); Rough->R = .85f;
        M->GetExpressionCollection().AddExpression(Rough);
        M->GetEditorOnlyData()->Roughness.Expression = Rough;
        M->PostEditChange(); Materials.Add(Hex, M); return M;
    }
    AStaticMeshActor* Shape(const FString& Name, FVector P, FVector Size, const FString& Hex,
                           bool Collision = true, FRotator R = FRotator::ZeroRotator, int32 Mesh = 0)
    {
        auto* A = Actor<AStaticMeshActor>(Name, P, R);
        if (!A) return nullptr;
        auto* C = A->GetStaticMeshComponent(); C->SetMobility(EComponentMobility::Static);
        C->SetStaticMesh(Meshes[Mesh]); C->SetMaterial(0, Material(Hex));
        C->SetCollisionProfileName(Collision ? TEXT("BlockAll") : TEXT("NoCollision"));
        C->SetGenerateOverlapEvents(false); C->SetCanEverAffectNavigation(Collision);
        A->SetActorScale3D(Size / 100.0); return A;
    }
    void Fade(const FString& Name, FVector Center, FVector Size)
    {
        auto* A = Actor<AActor>(Name + TEXT(".FadeVolume"), Center);
        if (!A) return;
        auto* B = NewObject<UBoxComponent>(A, TEXT("FadeBounds"));
        A->SetRootComponent(B); A->AddInstanceComponent(B); B->SetBoxExtent(Size / 2);
        B->SetCollisionProfileName(TEXT("NoCollision")); B->SetCanEverAffectNavigation(false); B->RegisterComponent(); A->SetActorLocation(Center);
        A->Tags.Append({TEXT("LH.Fade.AuthoringVolume"), FName(*Name), TEXT("LH.Fade.CapCm.80")});
    }
    void Rect(const FString& Name, double X0, double X1, double Y0, double Y1, bool Floor = true)
    {
        Footprints.Add({{{X0,Y0},{X1,Y0},{X1,Y1},{X0,Y1}}});
        if (Floor) Shape(Name + TEXT(".Floor"), FVector((X0+X1)*50,(Y0+Y1)*50,-10), FVector((X1-X0)*100,(Y1-Y0)*100,20), TEXT("77766D"));
    }
    void Room(const FString& Name, double X0, double X1, double Y0, double Y1,
              TArray<double> West, TArray<double> East, TArray<double> South, TArray<double> North, bool Floor = true)
    {
        Rect(Name,X0,X1,Y0,Y1,Floor);
        Walls.Add({true,X0-.1,Y0,Y1,West}); Walls.Add({true,X1+.1,Y0,Y1,East});
        Walls.Add({false,Y0-.1,X0,X1,South}); Walls.Add({false,Y1+.1,X0,X1,North});
        // Stable temporary hard cutaway; fade metadata lets presentation replace it later.
        auto* Roof = Shape(Name+TEXT(".Roof"), FVector((X0+X1)*50,(Y0+Y1)*50,410), FVector((X1-X0)*100,(Y1-Y0)*100,20), TEXT("4C5356"),false);
        if (Roof) { Roof->SetActorHiddenInGame(true); Roof->GetStaticMeshComponent()->SetVisibility(false); Roof->GetStaticMeshComponent()->SetCastShadow(false); Roof->Tags.Add(TEXT("LH.Fade.Roof")); }
        Fade(Name,FVector((X0+X1)*50,(Y0+Y1)*50,300),FVector((X1-X0)*100,(Y1-Y0)*100,600));
    }
    void Route(const FString& Name, TArray<FVector2D> P)
    {
        for (int32 I=0; I<P.Num(); ++I)
        {
            Rect(Name+FString::Printf(TEXT(".Turn%d"),I),P[I].X-2,P[I].X+2,P[I].Y-2,P[I].Y+2);
            if (I==0) continue;
            FVector2D D=P[I]-P[I-1], N=FVector2D(-D.Y,D.X).GetSafeNormal()*2;
            Footprints.Add({{P[I-1]-N,P[I]-N,P[I]+N,P[I-1]+N}});
            auto Mid=(P[I]+P[I-1])*.5;
            Shape(Name+FString::Printf(TEXT(".Strip%d"),I),FVector(Mid.X*100,Mid.Y*100,-10),FVector(D.Size()*100,400,20),TEXT("B2A58A"),true,FRotator(0,FMath::RadiansToDegrees(FMath::Atan2(D.Y,D.X)),0));
        }
    }
    static double Cross(FVector2D A,FVector2D B) { return A.X*B.Y-A.Y*B.X; }
    bool Inside(FVector2D P) const
    {
        for (const auto& Poly:Footprints)
        {
            bool In=true;
            for(int32 I=0;I<Poly.Points.Num();++I)
                if(Cross(Poly.Points[(I+1)%Poly.Points.Num()]-Poly.Points[I],P-Poly.Points[I]) < -1.e-6) { In=false; break; }
            if(In) return true;
        }
        return false;
    }
    void Boundary()
    {
        // Clip each convex footprint edge against every other footprint. Keep only
        // union exterior edges, so crossings/doors never acquire a stray fence.
        TSet<FString> Seen;
        for(const auto& Poly:Footprints) for(int32 I=0;I<Poly.Points.Num();++I)
        {
            FVector2D A=Poly.Points[I], D=Poly.Points[(I+1)%Poly.Points.Num()]-A;
            TArray<double> Cuts{0,1};
            for(const auto& Other:Footprints) for(int32 J=0;J<Other.Points.Num();++J)
            {
                FVector2D B=Other.Points[J], E=Other.Points[(J+1)%Other.Points.Num()]-B;
                double Den=Cross(D,E);
                if(FMath::Abs(Den)>1.e-8)
                {
                    double T=Cross(B-A,E)/Den, U=Cross(B-A,D)/Den;
                    if(T>0 && T<1 && U>=0 && U<=1) Cuts.Add(T);
                }
                else if(FMath::Abs(Cross(B-A,D))<1.e-6)
                    for(auto V:{B,B+E}) { double T=FVector2D::DotProduct(V-A,D)/D.SizeSquared(); if(T>0 && T<1) Cuts.Add(T); }
            }
            Cuts.Sort(); FVector2D Out=FVector2D(D.Y,-D.X).GetSafeNormal();
            for(int32 K=1;K<Cuts.Num();++K)
            {
                double T0=Cuts[K-1],T1=Cuts[K]; if((T1-T0)*D.Size()<.001) continue;
                auto Mid=A+D*((T0+T1)*.5); if(Inside(Mid+Out*.001)) continue;
                auto Start=A+D*T0, End=A+D*T1;
                FString Key=FString::Printf(TEXT("%.4f,%.4f:%.4f,%.4f"),Start.X,Start.Y,End.X,End.Y);
                if(Seen.Contains(Key)) continue; Seen.Add(Key);
                Shape(FString::Printf(TEXT("District.Edge.%03d"),BoundaryCount++),FVector((Mid.X+Out.X*.1)*100,(Mid.Y+Out.Y*.1)*100,50),FVector((End-Start).Size()*100+2,20,100),TEXT("78634B"),true,FRotator(0,FMath::RadiansToDegrees(FMath::Atan2(D.Y,D.X)),0));
                auto* Under=Shape(FString::Printf(TEXT("District.EdgeUnder.%03d"),BoundaryCount),FVector((Mid.X+Out.X*.1)*100,(Mid.Y+Out.Y*.1)*100,-50),FVector((End-Start).Size()*100+2,20,100),TEXT("78634B"));
                if(Under) { Under->GetStaticMeshComponent()->SetVisibility(false); Under->GetStaticMeshComponent()->SetCastShadow(false); }
            }
        }
    }
    void BuildWalls()
    {
        // Group collinear shared partitions; cut a single set of door apertures.
        TArray<FWall> Groups;
        for(const auto& W:Walls)
        {
            auto* G=Groups.FindByPredicate([&](const FWall& V){return V.bX==W.bX && FMath::IsNearlyEqual(V.Fixed,W.Fixed);});
            if(G) { G->Doors.Append(W.Doors); } else Groups.Add(W);
        }
        for(const auto& G:Groups)
        {
            TArray<double> Cuts;
            for(const auto& W:Walls) if(W.bX==G.bX && FMath::IsNearlyEqual(W.Fixed,G.Fixed)) { Cuts.Add(W.Lo); Cuts.Add(W.Hi); }
            for(double Door:G.Doors) { Cuts.Add(Door-1.6); Cuts.Add(Door+1.6); }
            Cuts.Sort();
            for(int32 I=1;I<Cuts.Num();++I)
            {
                double Lo=Cuts[I-1],Hi=Cuts[I],Mid=(Lo+Hi)/2; if(Hi-Lo<.001) continue;
                bool Covered= Walls.ContainsByPredicate([&](const FWall& W){return W.bX==G.bX && FMath::IsNearlyEqual(W.Fixed,G.Fixed) && Mid>=W.Lo && Mid<=W.Hi;});
                if(!Covered) continue;
                bool Door=G.Doors.ContainsByPredicate([&](double V){return FMath::Abs(Mid-V)<1.6;});
                FVector P=G.bX?FVector(G.Fixed*100,Mid*100,200):FVector(Mid*100,G.Fixed*100,200);
                FVector Size=G.bX?FVector(20,(Hi-Lo)*100,400):FVector((Hi-Lo)*100,20,400);
                FString Name=FString::Printf(TEXT("Temple.Wall.%03d"),WallCount++);
                // Separate collision from presentation: upper render is permanently cut.
                auto* Collider=Shape(Name+TEXT(".Collision"),P,Size,TEXT("78634B"));
                if(Collider) { Collider->GetStaticMeshComponent()->SetVisibility(false); Collider->GetStaticMeshComponent()->SetCastShadow(false); }
                if(Door) { if(Collider) { Collider->SetActorLocation(P+FVector(0,0,150)); Collider->SetActorScale3D(FVector(Size.X,Size.Y,100)/100); } }
                else Shape(Name+TEXT(".Cap"),FVector(P.X,P.Y,40),FVector(Size.X,Size.Y,80),TEXT("78634B"),false);
                Fade(Name,P,Size);
            }
        }
    }
    void Piece(AActor* Owner, const FString& Name, int32 Mesh, FVector P, FVector Size, const FString& Hex, FRotator R=FRotator::ZeroRotator)
    {
        FTransform T=Owner->GetActorTransform();
        auto* A=Shape(Owner->GetActorLabel()+TEXT(".")+Name,T.TransformPosition(P),Size,Hex,false,(T.GetRotation()*R.Quaternion()).Rotator(),Mesh);
        if(A) A->AttachToActor(Owner,FAttachmentTransformRules::KeepWorldTransform);
    }
    void Rod(AActor* Owner,const FString& Name,int32 Mesh,double Diameter,FVector Start,FVector End,const FString& Hex)
    {
        FVector D=End-Start;
        Piece(Owner,Name,Mesh,(Start+End)/2,FVector(Diameter,Diameter,D.Size()),Hex,FQuat::FindBetweenNormals(FVector::UpVector,D.GetSafeNormal()).Rotator());
    }
    void NPC(const FString& Alias,const FString& Guid,FVector P,double Yaw,int32 Recipe);
    void Populate();
};

void FHub::NPC(const FString& Alias,const FString& Guid,FVector P,double Yaw,int32 Recipe)
{
    auto* A=Actor<ALHInteractableMarker>(Alias,P,FRotator(0,Yaw,0)); if(!A) return;
    A->Area.Content.Value=TEXT("Area.LighthavenTempleDistrict"); A->DefinitionId.Value=FName(*Alias);
    if(!FGuid::Parse(Guid,A->InstanceId)) { bOK=false; return; }
    A->Tags.Add(FName(*(TEXT("Presentation.")+Alias)));
    const TCHAR* Cloth[]={TEXT("B5A58A"),TEXT("C0AC86"),TEXT("5E6973"),TEXT("927451"),TEXT("62634A"),TEXT("5E6973"),TEXT("535451"),TEXT("B5A58A"),TEXT("5E6973"),TEXT("62634A"),TEXT("806850"),TEXT("666453"),TEXT("733D35")};
    Piece(A,TEXT("boot_1"),0,FVector(5,-12,9),FVector(24,16,18),TEXT("55402C"));
    Rod(A,TEXT("leg_1"),2,14,FVector(0,-12,18),FVector(0,-12,76),Cloth[Recipe]);
    Rod(A,TEXT("sleeve_1"),2,16,FVector(0,-29,103),FVector(0,-29,141),Cloth[Recipe]);
    Piece(A,TEXT("hand_1"),1,FVector(2,-29,95),FVector(14,12,18),TEXT("8B5A40"));
    Piece(A,TEXT("boot1"),0,FVector(5,12,9),FVector(24,16,18),TEXT("55402C"));
    Rod(A,TEXT("leg1"),2,14,FVector(0,12,18),FVector(0,12,76),Cloth[Recipe]);
    Rod(A,TEXT("sleeve1"),2,16,FVector(0,29,103),FVector(0,29,141),Cloth[Recipe]);
    Piece(A,TEXT("hand1"),1,FVector(2,29,95),FVector(14,12,18),TEXT("8B5A40"));
    Piece(A,TEXT("pelvis"),1,FVector(0,0,81),FVector(30,34,28),Cloth[Recipe]);
    Rod(A,TEXT("tunic_hem"),3,50,FVector(0,0,76),FVector(0,0,111),Cloth[Recipe]);
    Piece(A,TEXT("torso"),0,FVector(0,0,118),FVector(28,46,58),Cloth[Recipe]);
    Piece(A,TEXT("belt"),0,FVector(0,0,92),FVector(30,48,8),TEXT("55402C"));
    Piece(A,TEXT("face"),1,FVector(0,0,161),FVector(28,26,28),TEXT("8B5A40"));
    Rod(A,TEXT("BD8E72"),3,8,FVector(13,0,160),FVector(20,0,160),TEXT("8B5A40"));
    Piece(A,TEXT("hair_cap"),1,FVector(-1,0,170),FVector(30,28,12),TEXT("30251D"));
    switch(Recipe) {
    case 0:
        Piece(A,TEXT("book"),0,FVector(26,0,132),FVector(35,46,8),TEXT("C0AC86"));
        Piece(A,TEXT("spine"),0,FVector(26,0,134),FVector(37,5,10),TEXT("55402C"));
        break;
    case 1:
        Piece(A,TEXT("sun"),1,FVector(25,0,143),FVector(26,26,26),TEXT("C0AC86"));
        Piece(A,TEXT("sun_ray_Y_1"),0,FVector(25,-20,143),FVector(9,14,6),TEXT("C0AC86"));
        Piece(A,TEXT("sun_ray_X_1"),0,FVector(8,0,143),FVector(14,9,6),TEXT("C0AC86"));
        Piece(A,TEXT("sun_ray_Y1"),0,FVector(25,20,143),FVector(9,14,6),TEXT("C0AC86"));
        Piece(A,TEXT("sun_ray_X1"),0,FVector(42,0,143),FVector(14,9,6),TEXT("C0AC86"));
        break;
    case 2:
        Piece(A,TEXT("tablet_1"),0,FVector(25,-12,139),FVector(32,14,12),TEXT("A9C4CC"));
        Piece(A,TEXT("tablet1"),0,FVector(25,12,139),FVector(32,14,12),TEXT("A9C4CC"));
        break;
    case 3:
        Piece(A,TEXT("satchel"),1,FVector(-6,34,82),FVector(30,28,36),TEXT("C0AC86"));
        Rod(A,TEXT("rolled_bag_top"),2,12,FVector(-17,34,99),FVector(7,34,99),TEXT("55402C"));
        break;
    case 4:
        Rod(A,TEXT("bow_lower"),2,8,FVector(0,-42,35),FVector(22,-42,105),TEXT("55402C"));
        Rod(A,TEXT("bow_upper"),2,8,FVector(22,-42,105),FVector(0,-42,175),TEXT("55402C"));
        Rod(A,TEXT("quiver"),2,18,FVector(-25,22,88),FVector(-25,22,143),TEXT("55402C"));
        Rod(A,TEXT("arrow17"),2,4,FVector(-25,17,95),FVector(-25,17,160),TEXT("C0AC86"));
        Rod(A,TEXT("arrow27"),2,4,FVector(-25,27,95),FVector(-25,27,160),TEXT("C0AC86"));
        break;
    case 5:
        Piece(A,TEXT("bottle"),1,FVector(27,0,125),FVector(32,32,36),TEXT("A9C4CC"));
        Rod(A,TEXT("bottle_neck"),2,12,FVector(27,0,139),FVector(27,0,156),TEXT("C0AC86"));
        break;
    case 6:
        Piece(A,TEXT("bib"),0,FVector(19,0,119),FVector(8,58,55),TEXT("959083"));
        Piece(A,TEXT("raised_rim"),0,FVector(19,0,149),FVector(12,62,10),TEXT("C0AC86"));
        break;
    case 7:
        Rod(A,TEXT("training_bar_1"),2,12,FVector(27,-17,97),FVector(27,-17,165),TEXT("C0AC86"));
        Rod(A,TEXT("training_bar1"),2,12,FVector(27,17,97),FVector(27,17,165),TEXT("C0AC86"));
        break;
    case 8:
        Piece(A,TEXT("shoulder_tab_1"),0,FVector(0,-24,150),FVector(25,14,8),TEXT("C0AC86"));
        Piece(A,TEXT("shoulder_tab1"),0,FVector(0,24,150),FVector(25,14,8),TEXT("C0AC86"));
        break;
    case 9:
        Piece(A,TEXT("zigzag_bar0"),0,FVector(12,-12,141),FVector(16,32,6),TEXT("A9C4CC"));
        Piece(A,TEXT("zigzag_bar1"),0,FVector(22,0,146),FVector(16,32,6),TEXT("A9C4CC"));
        Piece(A,TEXT("zigzag_bar2"),0,FVector(32,12,151),FVector(16,32,6),TEXT("A9C4CC"));
        break;
    case 10:
        Rod(A,TEXT("round_plate"),2,44,FVector(22,0,126),FVector(22,0,135),TEXT("C0AC86"));
        Piece(A,TEXT("boss"),0,FVector(22,0,140),FVector(18,18,10),TEXT("55402C"));
        break;
    case 11:
        Piece(A,TEXT("cairn0"),0,FVector(25,0,122),FVector(38,38,13),TEXT("959083"));
        Piece(A,TEXT("cairn1"),0,FVector(25,0,135),FVector(28,28,13),TEXT("959083"));
        Piece(A,TEXT("cairn2"),0,FVector(25,0,148),FVector(18,18,13),TEXT("959083"));
        break;
    case 12:
        Piece(A,TEXT("flame_base"),0,FVector(25,0,120),FVector(30,30,6),TEXT("C0AC86"));
        Rod(A,TEXT("flame"),3,30,FVector(25,0,123),FVector(25,0,172),TEXT("A33A29"));
        break;
    default: bOK=false; break;
    }
}
void FHub::Populate()
{
    Room(TEXT("Temple.Nave"),0,16,0,24,{5},{},{8},{});
    Room(TEXT("Temple.DungeonWing"),-12,-.2,0,10,{}, {5},{},{},false);
    // Flat wing excludes the entire ramp footprint, no collision cap over descent.
    Shape(TEXT("Wing.FlatEast"),FVector(-410,500,-10),FVector(780,1000,20),TEXT("77766D"));
    Shape(TEXT("Wing.FlatSouth"),FVector(-1000,175,-10),FVector(400,350,20),TEXT("77766D"));
    Shape(TEXT("Wing.FlatNorth"),FVector(-1000,825,-10),FVector(400,350,20),TEXT("77766D"));
    const double Angle=FMath::RadiansToDegrees(FMath::Atan(.25));
    Shape(TEXT("Temple.Descent.Ramp"),FVector(-997.5746,500,-59.7014),FVector(FMath::Sqrt(170000.0),300,20),TEXT("959083"),true,FRotator(Angle,0,0));
    for(int32 I=0;I<8;++I)
        Shape(FString::Printf(TEXT("Temple.Descent.Tread%d"),I),FVector(-825-I*50,500,-6.25-I*12.5+1),FVector(50,300,2),TEXT("959083"),false);
    Rect(TEXT("Temple.Forecourt"),-6,20,-12,-.2);
    Room(TEXT("Temple.SigfriedShop"),-30,-14,8,24,{},{},{-22},{});
    Room(TEXT("Temple.FaliRolphShop"),-22,-6,-24,-12,{},{},{},{-14});
    Room(TEXT("Temple.TrainingHouse"),4,20,-34,-22,{},{},{},{12});
    Room(TEXT("Temple.KalastorHouse"),4,20,-64,-52,{},{},{},{12});
    Rect(TEXT("Temple.MurmuntagSquare"),-22,-6,-70,-54);
    Rect(TEXT("Temple.MageApron"),-40,-16,54,59.8);
    Room(TEXT("Temple.MageVestibule"),-48,-16,60,66,{},{},{-33},{-33});
    Room(TEXT("Temple.MageHall"),-36,-30,66.2,84.2,{72},{77},{-33},{});
    Room(TEXT("Temple.UranosRoom"),-48,-36.2,66.2,78,{},{72},{},{});
    Room(TEXT("Temple.IraltokRoom"),-29.8,-16,70.2,84.2,{77},{},{},{});
    Rect(TEXT("Threshold.NaveWing"),-.2,0,3.4,6.6);
    Rect(TEXT("Threshold.NaveCourt"),6.4,9.6,-.2,0);
    Rect(TEXT("Threshold.MageHall"),-34.6,-31.4,66,66.2);
    Rect(TEXT("Threshold.Uranos"),-36.2,-36,70.4,73.6);
    Rect(TEXT("Threshold.Iraltok"),-30,-29.8,75.4,78.6);
    Route(TEXT("Temple.WestLane"),{{-4,-8},{-26,-8},{-26,4},{-22,4},{-22,8}});
    Route(TEXT("Temple.FaliSpur"),{{-14,-8},{-14,-12}});
    Route(TEXT("Temple.TrainingLane"),{{12,-12},{12,-22}});
    Route(TEXT("Temple.SouthLane"),{{-2,-12},{-2,-48}});
    Route(TEXT("Temple.KalastorSpur"),{{-2,-48},{12,-48},{12,-52}});
    Route(TEXT("Temple.MurmuntagSpur"),{{-2,-44},{-14,-44},{-14,-54}});
    Route(TEXT("Temple.ShoreLane"),{{20,-6},{24,-6},{24,28}});
    Route(TEXT("Temple.MageCauseway"),{{24,28},{24,36},{16,42},{0,42},{-16,50},{-30,50},{-33,56}});
    Route(TEXT("Temple.MageDoorApproach"),{{-33,56},{-33,60}});
    BuildWalls(); Boundary();
    Shape(TEXT("Scenic.Water"),FVector(-1000,4200,-50),FVector(10000,10000,10),TEXT("526F79"),false);
    // Visible closures on unbuilt outer-island branches; no travel actors there.
    Shape(TEXT("Scenic.OuterIslandBoundary"),FVector(-1600,5330,70),FVector(400,20,20),TEXT("43352A"));
    Shape(TEXT("Temple.RedAisle"),FVector(800,1050,.5),FVector(400,2100,1),TEXT("733D35"),false);
    for(int X:{250,1350}) for(int Y:{1100,1600})
        Shape(FString::Printf(TEXT("Temple.Pew.%d.%d"),X,Y),FVector(X,Y,50),FVector(300,200,100),TEXT("43352A"));
    Shape(TEXT("Temple.AltarPlinth"),FVector(800,2250,10),FVector(400,100,20),TEXT("78634B"));
    Shape(TEXT("Temple.AltarTable"),FVector(800,2250,100),FVector(300,80,20),TEXT("43352A"));
    // Roof silhouette retained for editor inspection, hard-cut at runtime baseline.
    for(int Side:{-1,1})
    {
        auto* Roof=Shape(FString::Printf(TEXT("Temple.Nave.RoofSlope.%d"),Side),FVector(800+Side*400,1200,500),FVector(FMath::Sqrt(680000.0),2400,20),TEXT("4C5356"),false,FRotator(-Side*Angle,0,0));
        if(Roof) { Roof->SetActorHiddenInGame(true); Roof->GetStaticMeshComponent()->SetVisibility(false); Roof->GetStaticMeshComponent()->SetCastShadow(false); Roof->Tags.Add(TEXT("LH.Fade.Roof")); }
    }
    const auto* Area=LHWorld::FindArea(FLHAreaId{FLHContentId{TEXT("Area.LighthavenTempleDistrict")}});
    if(!Area || Area->Entrances.Num()!=2 || Area->Portals.Num()!=1) { bOK=false; return; }
    for(const auto& E:Area->Entrances)
    {
        auto* A=Actor<ALHEntranceMarker>(E.Id.LocalId.ToString(),E.SafeTransform.GetLocation(),E.SafeTransform.Rotator());
        if(A) { A->EntranceId=E.Id; A->SafeArrivalTransform=E.SafeTransform; A->bSafetyReviewed=LHArrivalReview::IsReviewed(E); }
    }
    const auto& P=Area->Portals[0];
    auto* Portal=Actor<ALHPortal>(TEXT("Temple.Descent.Departure"),FVector(-1000,500,-50));
    if(Portal)
    {
        Portal->PortalId=P.Portal.InstanceId; Portal->Source=P.Source; Portal->Destination=P.Destination; Portal->Direction=ELHPortalDirection::Descent;
        auto* Footprint=NewObject<UBoxComponent>(Portal,TEXT("InteractionFootprint"));
        Portal->AddInstanceComponent(Footprint); Footprint->SetupAttachment(Portal->GetRootComponent());
        Footprint->SetBoxExtent(FVector(50,150,150)); Footprint->SetCollisionProfileName(TEXT("NoCollision"));
        Footprint->SetCanEverAffectNavigation(false); Footprint->RegisterComponent();
        Portal->Tags.Add(TEXT("LH.Interaction.ExplicitOnly"));
    }
    // A-03 double-bar down arrow on flat approach, pointing toward the ramp.
    for(int I=0;I<2;++I) Shape(FString::Printf(TEXT("Temple.Descent.ArrowBar%d"),I),FVector(-750+I*25,500,2),FVector(6,60,4),TEXT("E9BF79"),false);
    for(int Side:{-1,1}) Shape(FString::Printf(TEXT("Temple.Descent.ArrowHead%d"),Side),FVector(-770,500+Side*15,2),FVector(45,6,4),TEXT("E9BF79"),false,FRotator(0,Side*45,0));
    const double HH=GetDefault<ALHCharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    Actor<APlayerStart>(TEXT("Temple.SafeSpawn.PlayerStart"),FVector(800,500,HH),FRotator(0,90,0));
    NPC(TEXT("NPC.BrotherKiran"),TEXT("cad1834f92084c3484f48b9c4c077a7b"),FVector(300,2100,0),-90,0);
    NPC(TEXT("NPC.Kilhiam"),TEXT("6dfd770dbf874ac09dd2efba40bcd81e"),FVector(300,1400,0),0,1);
    NPC(TEXT("NPC.Moonrock"),TEXT("5ef65f0ad7cf450ca6554e17c4115287"),FVector(1300,2000,0),180,2);
    NPC(TEXT("NPC.Samaritan"),TEXT("7aed0467432c4f868c81e65ede50f1b2"),FVector(-200,-300,0),-90,3);
    NPC(TEXT("NPC.Sigfried"),TEXT("36351285404c489f8accd6b6e3a9d2ab"),FVector(-2500,1800,0),0,4);
    NPC(TEXT("NPC.Fali"),TEXT("0d1ff670d056496ca68fac87dec830c5"),FVector(-1800,-1900,0),0,5);
    NPC(TEXT("NPC.Rolph"),TEXT("461bc4749f07451aa01ef2bec941aff0"),FVector(-900,-1900,0),180,6);
    NPC(TEXT("NPC.Ortanalas"),TEXT("16e51f72d20d4efc8bc725cc47c2cdb7"),FVector(1500,-2900,0),180,7);
    NPC(TEXT("NPC.JagarKar"),TEXT("4522396925a0480fbafbde5975c3cb30"),FVector(700,-2900,0),90,8);
    NPC(TEXT("NPC.Kalastor"),TEXT("8cb18c8e2d2c47a48a59a0a99375c1cf"),FVector(1500,-5900,0),180,9);
    NPC(TEXT("NPC.Murmuntag"),TEXT("6ef21e05308e465cb6add6ce34e0d150"),FVector(-1700,-6200,0),0,10);
    NPC(TEXT("NPC.Uranos"),TEXT("ea0144a11a54417487447d0c02d9706e"),FVector(-4300,7200,0),0,11);
    NPC(TEXT("NPC.Iraltok"),TEXT("448469187fbd4606b5767ff3b8b87218"),FVector(-2200,7700,0),180,12);
    struct FLight { const TCHAR* Id; double X,Y,Z; char Profile; };
    const FLight Lights[]={
        {TEXT("E01"),10,-.3,2.5,'T'},{TEXT("E02"),-20,7.7,2.5,'T'},
        {TEXT("E03"),-12,-11.7,2.5,'T'},{TEXT("E04"),14,-21.7,2.5,'T'},
        {TEXT("E05"),14,-51.7,2.5,'T'},{TEXT("E06"),-21,-62,2.5,'T'},
        {TEXT("E07"),-31,59.7,2.5,'T'},{TEXT("I01"),8,22.5,3.5,'W'},
        {TEXT("I02"),.4,14,2.5,'T'},{TEXT("I03"),15.6,20,2.5,'T'},
        {TEXT("I04"),-.5,5,2.5,'T'},{TEXT("I05"),-8,2.2,2.5,'N'},
        {TEXT("I06"),-29.6,18,2.5,'T'},{TEXT("I07"),-14,-23.6,2.5,'T'},
        {TEXT("I08"),12,-33.6,2.5,'T'},{TEXT("I09"),12,-63.6,2.5,'T'},
        {TEXT("I10"),-29,65.6,2.5,'T'},{TEXT("I11"),-35.6,81,2.5,'N'},
        {TEXT("I12"),-47.6,72,2.5,'T'},{TEXT("I13"),-16.4,77,2.5,'T'}};
    for(const auto& L:Lights) if(auto* A=Actor<APointLight>(FString(TEXT("Hub.Light."))+L.Id,FVector(L.X*100,L.Y*100,L.Z*100)))
    {
        auto* C=A->PointLightComponent.Get(); C->SetMobility(EComponentMobility::Movable);
        C->SetIntensityUnits(ELightUnits::Lumens); C->SetIntensity(L.Profile=='N'?1200:L.Profile=='W'?1000:600);
        C->SetAttenuationRadius(L.Profile=='N'?850:L.Profile=='W'?800:600);
        C->SetUseTemperature(true); C->SetTemperature(L.Profile=='N'?6500:2300); C->SetCastShadows(false);
    }
    if(auto* A=Actor<ADirectionalLight>(TEXT("Hub.Sun"),FVector(0,0,1500),FRotator(-45,45,0)))
    {
        auto* C=CastChecked<UDirectionalLightComponent>(A->GetLightComponent()); C->SetMobility(EComponentMobility::Movable);
        C->SetIntensity(3000); C->SetUseTemperature(true); C->SetTemperature(6500); C->SetAtmosphereSunLight(true);
    }
    Actor<ASkyAtmosphere>(TEXT("Hub.SkyAtmosphere"),FVector::ZeroVector);
    if(auto* A=Actor<ASkyLight>(TEXT("Hub.SkyFill"),FVector(0,0,1000)))
    {
        auto* C=A->GetLightComponent(); C->SetMobility(EComponentMobility::Movable); C->SetIntensity(.7f);
        C->SetCastShadows(false); C->SetRealTimeCaptureEnabled(false);
        auto* Cube=NewObject<UTextureCube>(World,TEXT("HubNeutralCubemap"),RF_Public);
        TArray<FColor> Pixels; Pixels.Init(FColor(128,128,128,255),16*16*6);
        Cube->Source.Init(16,16,6,1,TSF_BGRA8,reinterpret_cast<const uint8*>(Pixels.GetData()));
        Cube->SRGB=true; Cube->CompressionSettings=TC_Default; Cube->PostEditChange();
        C->SourceType=SLS_SpecifiedCubemap; C->SetCubemap(Cube);
        C->bLowerHemisphereIsBlack=true; C->SetLowerHemisphereColor(FLinearColor::FromSRGBColor(FColor(128,128,128)));
    }
    if(auto* PP=Actor<APostProcessVolume>(TEXT("Hub.FixedExposure"),FVector::ZeroVector))
    {
        PP->bUnbound=true;
        PP->Settings.bOverride_AutoExposureMinBrightness=true; PP->Settings.AutoExposureMinBrightness=6;
        PP->Settings.bOverride_AutoExposureMaxBrightness=true; PP->Settings.AutoExposureMaxBrightness=6;
        PP->Settings.bOverride_AutoExposureBias=true; PP->Settings.AutoExposureBias=0;
        PP->Settings.bOverride_BloomIntensity=true; PP->Settings.BloomIntensity=0;
        PP->Settings.bOverride_MotionBlurAmount=true; PP->Settings.MotionBlurAmount=0;
    }
    if(auto* Nav=Actor<ANavMeshBoundsVolume>(TEXT("Hub.NavigationBounds"),FVector(-1000,800,400)))
    {
        auto* Builder=NewObject<UCubeBuilder>(); Builder->X=8000; Builder->Y=17000; Builder->Z=1400;
        UActorFactory::CreateBrushForVolumeActor(Nav,Builder);
    }
    World->GetWorldSettings()->DefaultGameMode=ALHGameMode::StaticClass();
}
}
ULHGenerateHubMapCommandlet::ULHGenerateHubMapCommandlet()
{
    IsClient=false; IsServer=false; IsEditor=true; LogToConsole=true;
}
int32 ULHGenerateHubMapCommandlet::Main(const FString& Params)
{
    const FString Package=TEXT("/Game/Lighthaven/Maps/L_LighthavenTempleDistrict");
    const FString Filename=FPackageName::LongPackageNameToFilename(Package,FPackageName::GetMapPackageExtension());
    if(IFileManager::Get().FileExists(*Filename) && IFileManager::Get().IsReadOnly(*Filename))
    { UE_LOG(LogTemp,Error,TEXT("Read-only LFS map: %s"),*Filename); return 1; }
    if(!GEditor) return 1;
    LHHubGenerator::FHub Hub{};
    const TCHAR* Paths[]={TEXT("/Engine/BasicShapes/Cube.Cube"),TEXT("/Engine/BasicShapes/Sphere.Sphere"),TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),TEXT("/Engine/BasicShapes/Cone.Cone")};
    for(int32 I=0;I<4;++I) { Hub.Meshes[I]=LoadObject<UStaticMesh>(nullptr,Paths[I]); if(!Hub.Meshes[I]) return 1; }
    TArray<FString> Errors;
    if(!LHWorld::ValidateRegistry(Errors)) { for(const auto& E:Errors) UE_LOG(LogTemp,Error,TEXT("%s"),*E); return 1; }
    Hub.World=GEditor->NewMap(false); if(!Hub.World) return 1;
    Hub.Populate();
    TArray<FString> Manifest;
    TSet<FGuid> Ids;
    for(TActorIterator<AActor> It(Hub.World);It;++It)
    {
        AActor* A=*It; if(!A->Tags.Contains(TEXT("LH.Hub.Generated"))) continue;
        FString Identity;
        if(auto* N=Cast<ALHInteractableMarker>(A))
        {
            if(!N->InstanceId.IsValid() || Ids.Contains(N->InstanceId)) Hub.bOK=false;
            Ids.Add(N->InstanceId); Identity=N->InstanceId.ToString()+TEXT("/")+N->DefinitionId.Value.ToString();
        }
        if(auto* P=Cast<ALHPortal>(A)) Identity=P->PortalId.ToString()+TEXT("/")+P->Destination.LocalId.ToString();
        if(auto* E=Cast<ALHEntranceMarker>(A)) Identity=E->EntranceId.LocalId.ToString()+TEXT("/")+E->SafeArrivalTransform.ToString();
        if(auto* Box=Cast<UBoxComponent>(A->GetRootComponent())) Identity+=TEXT("/Bounds=")+Box->GetUnscaledBoxExtent().ToString();
        TArray<FString> Tags; for(FName Tag:A->Tags) Tags.Add(Tag.ToString()); Tags.Sort();
        Manifest.Add(A->GetActorLabel()+TEXT("\t")+A->GetClass()->GetName()+TEXT("\t")+A->GetActorTransform().ToString()+TEXT("\t")+Identity+TEXT("\t")+FString::Join(Tags,TEXT(",")));
    }
    Manifest.Sort();
    if(!FFileHelper::SaveStringToFile(FString::Join(Manifest,TEXT("\n"))+TEXT("\n"),*(FPaths::ProjectSavedDir()/TEXT("HubLayout.tsv")))) return 1;
    if(!Hub.bOK || !IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename),true) || !FEditorFileUtils::SaveMap(Hub.World,Filename)) return 1;
    UE_LOG(LogTemp,Display,TEXT("Generated %s: 13 NPCs, 2 unreviewed entrances, 1 registry portal, 20 local lights, %d boundary segments, %d wall segments"),*Package,Hub.BoundaryCount,Hub.WallCount);
    return 0;
}
