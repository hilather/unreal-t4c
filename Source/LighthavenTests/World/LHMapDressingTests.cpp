#include "Misc/AutomationTest.h"
#include "Visual/LHVisualKit.h"
#include "Visual/LHB1ArtBinding.h"
#include "Visual/LHB1Lighting.h"
#include "World/LHAreaRegistry.h"
#include "World/LHWorldMarkers.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Components/BoxComponent.h"
#include "UObject/Package.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FLHMapDressingTest,"Lighthaven.World.Dressing.Maps",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FLHMapDressingTest::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
    for(const auto& Area:LHWorld::Registry()) { Names.Add(Area.Id.Content.Value.ToString()); Commands.Add(Area.Id.Content.Value.ToString()); }
}
bool FLHMapDressingTest::RunTest(const FString& Parameters)
{
    const auto* Area=LHWorld::Registry().FindByPredicate([&](const FLHAreaDefinition& A){return A.Id.Content.Value.ToString()==Parameters;});
    if(!Area) return false;
    const double LoadStart=FPlatformTime::Seconds();
    auto* Package=LoadPackage(nullptr,*Area->Map.GetLongPackageName(),LOAD_None);
    auto* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
    if(!TestNotNull(TEXT("Generated map required"),World)) return false;
    AddInfo(FString::Printf(TEXT("%s load/rebuild %.3f seconds"),*Parameters,FPlatformTime::Seconds()-LoadStart));
    TArray<ALHVisualPiece*> Pieces;
    TSet<FGuid> Spawns,Portals,NPCs;
    TSet<FName> Entrances;
    struct FNPCBaseline { const TCHAR* Alias; const TCHAR* Guid; FVector Position; double Yaw; };
    const FNPCBaseline BaselineNPCs[] = {
        {TEXT("NPC.BrotherKiran"),TEXT("cad1834f92084c3484f48b9c4c077a7b"),FVector(300,2100,0),-90},
        {TEXT("NPC.Kilhiam"),TEXT("6dfd770dbf874ac09dd2efba40bcd81e"),FVector(300,1400,0),0},
        {TEXT("NPC.Moonrock"),TEXT("5ef65f0ad7cf450ca6554e17c4115287"),FVector(1300,2000,0),180},
        {TEXT("NPC.Samaritan"),TEXT("7aed0467432c4f868c81e65ede50f1b2"),FVector(-200,-300,0),-90},
        {TEXT("NPC.Sigfried"),TEXT("36351285404c489f8accd6b6e3a9d2ab"),FVector(-2500,1800,0),0},
        {TEXT("NPC.Fali"),TEXT("0d1ff670d056496ca68fac87dec830c5"),FVector(-1800,-1900,0),0},
        {TEXT("NPC.Rolph"),TEXT("461bc4749f07451aa01ef2bec941aff0"),FVector(-900,-1900,0),180},
        {TEXT("NPC.Ortanalas"),TEXT("16e51f72d20d4efc8bc725cc47c2cdb7"),FVector(1500,-2900,0),180},
        {TEXT("NPC.JagarKar"),TEXT("4522396925a0480fbafbde5975c3cb30"),FVector(700,-2900,0),90},
        {TEXT("NPC.Kalastor"),TEXT("8cb18c8e2d2c47a48a59a0a99375c1cf"),FVector(1500,-5900,0),180},
        {TEXT("NPC.Murmuntag"),TEXT("6ef21e05308e465cb6add6ce34e0d150"),FVector(-1700,-6200,0),0},
        {TEXT("NPC.Uranos"),TEXT("ea0144a11a54417487447d0c02d9706e"),FVector(-4300,7200,0),0},
        {TEXT("NPC.Iraltok"),TEXT("448469187fbd4606b5767ff3b8b87218"),FVector(-2200,7700,0),180},
        {TEXT("NPC.Nevanis"),TEXT("32a8de01573448aa9fa6de27056614c2"),FVector(-2500,600,0),0},
        {TEXT("NPC.Shovanis"),TEXT("c451f507b1d442788b34a5d097516bc8"),FVector(-2500,1400,0),0},
    };
    int32 Cameras=0;
    for(TActorIterator<AActor> It(World);It;++It)
    {
        if(auto* P=Cast<ALHVisualPiece>(*It))
        {
            TestTrue(TEXT("Mesh excluded from package serialization"),P->GetMesh()->HasAnyFlags(RF_Transient));
            TestTrue(TEXT("Geometry present immediately after load"),P->GetMesh()->GetNumSections()>0);
            if(P->GetImportedMesh()->IsVisible())
            {
                TestEqual(TEXT("Only B1 uses imported art"),P->GetRecipe().Style,ELHVisualStyle::B1Cellar);
                TestEqual(TEXT("Imported dressing collision disabled"),P->GetImportedMesh()->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
                TestFalse(TEXT("Imported dressing nav disabled"),P->GetImportedMesh()->CanEverAffectNavigation());
                TestTrue(TEXT("Imported instances present"),P->GetImportedMesh()->GetInstanceCount()>0);
            }
            // Explicit rebuild proves serialized recipes reconstruct the same assembly.
            const auto Recipe=P->GetRecipe();
            const int32 StyleIndex=LHWorld::Registry().IndexOfByPredicate([&](const FLHAreaDefinition& V){return &V==Area;});
            TestEqual(TEXT("Per-map kit style"),int32(Recipe.Style),StyleIndex); const auto Fingerprint=Recipe.Fingerprint();
            TestTrue(TEXT("Serialized recipe rebuilds"),P->Build(Recipe));
            TestEqual(TEXT("Recipe stable"),P->GetRecipe().Fingerprint(),Fingerprint);
            Pieces.Add(P);
            // Stronger than pad intersection: all dressing is nonblocking, everywhere.
            TestEqual(TEXT("No dressing collision at any pad/route"),P->GetBlockers().Num(),0);
            TestEqual(TEXT("No serialized collision recipe"),Recipe.Collision.Num(),0);
        }
        if(auto* E=Cast<ALHEntranceMarker>(*It))
        {
            const auto* Expected=Area->Entrances.FindByPredicate([&](const FLHEntranceDefinition& V){return LHWorld::SameEntrance(V.Id,E->EntranceId);});
            TestNotNull(TEXT("Canonical entrance ID"),Expected);
            if(Expected) { TestTrue(TEXT("Arrival transform unchanged"),E->SafeArrivalTransform.Equals(Expected->SafeTransform,0)); TestTrue(TEXT("Entrance marker transform unchanged"),E->GetActorTransform().Equals(Expected->SafeTransform,.01)); }
            TestFalse(TEXT("Unique entrance"),Entrances.Contains(E->EntranceId.LocalId)); Entrances.Add(E->EntranceId.LocalId);
        }
        if(auto* S=Cast<ALHSpawnMarker>(*It))
        {
            const auto* Expected=Area->Spawns.FindByPredicate([&](const FLHSpawnAuthoring& V){return V.SpawnId==S->SpawnId;});
            TestNotNull(TEXT("Canonical spawn ID"),Expected);
            if(Expected) { TestEqual(TEXT("Enemy unchanged"),S->EnemyDefinitionId.Value,Expected->Enemy.Value); TestTrue(TEXT("Anchor unchanged"),S->GetActorTransform().Equals(Expected->Anchor,.01)); }
            TestFalse(TEXT("Unique spawn"),Spawns.Contains(S->SpawnId)); Spawns.Add(S->SpawnId);
        }
        if(auto* P=Cast<ALHPortal>(*It))
        {
            TestTrue(TEXT("Canonical portal ID"),Area->Portals.ContainsByPredicate([&](const FLHPortalDefinition& V){return V.Portal.InstanceId==P->PortalId && LHWorld::SameEntrance(V.Source,P->Source) && LHWorld::SameEntrance(V.Destination,P->Destination); }));
            const bool Return=P->Source.LocalId==TEXT("Entry");
            const int32 Index=LHWorld::Registry().IndexOfByPredicate([&](const FLHAreaDefinition& V){return &V==Area;});
            FVector Position(-1000,500,-50); double Yaw=0;
            if(Index==1) { Position=Return?FVector(450,-2500,100):FVector(4500,2100,-75); Yaw=Return?-90:90; }
            if(Index==2) { Position=Return?FVector(500,-6300,100):FVector(5300,6600,-100); Yaw=Return?-90:90; }
            if(Index==3) { Position=Return?FVector(3200,1360,80):FVector(4700,7060,-80); Yaw=90; }
            if(Index==4) { Position=FVector(-210,600,80); Yaw=180; }
            TestTrue(TEXT("Portal transform unchanged"),P->GetActorTransform().Equals(FTransform(FRotator(0,Yaw,0),Position),.01));
            TestEqual(TEXT("Portal direction unchanged"),P->Direction,Return?ELHPortalDirection::Return:ELHPortalDirection::Descent);
            TestFalse(TEXT("Unique portal"),Portals.Contains(P->PortalId)); Portals.Add(P->PortalId);
        }
        if(auto* N=Cast<ALHInteractableMarker>(*It))
        {
            const FNPCBaseline* Expected=nullptr;
            for(const auto& Row:BaselineNPCs) if(N->DefinitionId.Value.ToString().Equals(Row.Alias,ESearchCase::CaseSensitive)) Expected=&Row;
            TestNotNull(TEXT("Baseline NPC alias"),Expected);
            TestTrue(TEXT("NPC area unchanged"),LHWorld::SameArea(N->Area,Area->Id));
            if(Expected)
            {
                FGuid Guid; FGuid::Parse(Expected->Guid,Guid);
                TestEqual(TEXT("NPC identity unchanged"),N->InstanceId,Guid);
                TestTrue(TEXT("NPC transform unchanged"),N->GetActorTransform().Equals(FTransform(FRotator(0,Expected->Yaw,0),Expected->Position),.01));
            }
            TestFalse(TEXT("Unique NPC"),NPCs.Contains(N->InstanceId)); NPCs.Add(N->InstanceId);
        }
        if(It->IsA<ACameraActor>() && It->Tags.Contains(TEXT("LH.Capture"))) ++Cameras;
    }
    const auto Totals=LHVisual::ValidatePlacedSet(Pieces);
    for(const auto& Error:Totals.Errors) AddError(Error);
    TestTrue(TEXT("Dressing present and map piece budget"),Totals.Pieces>=20 && Totals.Pieces<=6000);
    TestTrue(TEXT("Map triangle budget"),Totals.Triangles<=2000000);
    TestTrue(TEXT("Map section budget"),Totals.DrawCalls<=12000);
    TestEqual(TEXT("Three fixed capture views"),Cameras,3);
    TestEqual(TEXT("Entrance inventory unchanged"),Entrances.Num(),Area->Entrances.Num());
    TestEqual(TEXT("Spawn inventory unchanged"),Spawns.Num(),Area->Spawns.Num());
    TestEqual(TEXT("Portal inventory unchanged"),Portals.Num(),Area->Portals.Num());
    TestEqual(TEXT("NPC inventory unchanged"),NPCs.Num(),Area->Spawns.IsEmpty()?13:(Parameters==TEXT("Area.TempleB1")?2:0));
    AddInfo(FString::Printf(TEXT("%s: pieces=%d triangles=%d sections=%d blockers=%d"),*Parameters,Totals.Pieces,Totals.Triangles,Totals.DrawCalls,Totals.CollisionBoxes));
    return true;
}

// Geometry rays deliberately bypass physics: dressing has zero collision.
// Test presentation bounding envelopes and retained/visible static mesh bounds.
// Imported envelopes are intersected with their actual material clipping box.
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FLHCaptureFramingTest,"Lighthaven.World.Dressing.CaptureFraming",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FLHCaptureFramingTest::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
    for(const auto& Area:LHWorld::Registry()) { Names.Add(Area.Id.Content.Value.ToString()); Commands.Add(Area.Id.Content.Value.ToString()); }
}
bool FLHCaptureFramingTest::RunTest(const FString& Parameters)
{
    const auto* Area=LHWorld::Registry().FindByPredicate([&](const FLHAreaDefinition& A){return A.Id.Content.Value.ToString()==Parameters;});
    if(!Area) return false;
    const double LoadStart=FPlatformTime::Seconds();
    auto* Package=LoadPackage(nullptr,*Area->Map.GetLongPackageName(),LOAD_None);
    auto* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
    if(!TestNotNull(TEXT("Generated map required"),World)) return false;
    AddInfo(FString::Printf(TEXT("%s first load/rebuild %.3f seconds"),*Parameters,FPlatformTime::Seconds()-LoadStart));
    struct FSurface { FTransform Transform; FBox Bounds; };
    TArray<FSurface> Surfaces;
    for(TActorIterator<AActor> It(World);It;++It)
    {
        if(auto* P=Cast<ALHVisualPiece>(*It))
        {
            auto* Imported=P->GetImportedMesh();
            LHB1Art::FFit Fit;
            if(Imported->IsVisible() && Imported->GetStaticMesh() && LHB1Art::Resolve(P->GetRecipe(),Fit))
            {
                for(const auto& Instance:Fit.Instances)
                {
                    const FBox Bounds=Imported->GetStaticMesh()->GetBoundingBox().TransformBy(Instance);
                    const FBox Clipped=Bounds.Overlap(Fit.ClipBounds);
                    if(Clipped.IsValid) Surfaces.Add({P->GetActorTransform(),Clipped});
                }
            }
            else for(const auto& B:P->GetRecipe().Geometry)
                Surfaces.Add({FTransform(B.Rotation,B.Center)*P->GetActorTransform(),FBox(-B.Size/2,B.Size/2)});
        }
        if(auto* A=Cast<AStaticMeshActor>(*It))
        {
            auto* C=A->GetStaticMeshComponent();
            if(C->GetStaticMesh() && !A->IsHidden() && (C->IsVisible() || A->Tags.Contains(TEXT("LH.Dressing.RetainedCollider"))))
                Surfaces.Add({C->GetComponentTransform(),C->GetStaticMesh()->GetBoundingBox()});
        }
    }
    int32 Cameras=0;
    for(TActorIterator<ACameraActor> It(World);It;++It)
    {
        if(!It->Tags.Contains(TEXT("LH.Capture"))) continue;
        ++Cameras; int32 Hits=0;
        auto* C=It->GetCameraComponent();
        TestEqual(TEXT("Capture horizontal FOV"),C->FieldOfView,65.f);
        const double HalfWidth=FMath::Tan(FMath::DegreesToRadians(65./2));
        const FVector Start=C->GetComponentLocation();
        for(int32 Y=0;Y<9;++Y) for(int32 X=0;X<16;++X)
        {
            const FVector Direction=C->GetComponentQuat().RotateVector(FVector(1,(2*(X+.5)/16-1)*HalfWidth,(1-2*(Y+.5)/9)*HalfWidth*9/16)).GetSafeNormal();
            const FVector End=Start+Direction*6000;
            for(const auto& Surface:Surfaces)
            {
                const FVector A=Surface.Transform.InverseTransformPosition(Start),B=Surface.Transform.InverseTransformPosition(End);
                if(FMath::LineBoxIntersection(Surface.Bounds,A,B,B-A)) { ++Hits; break; }
            }
        }
        const double Fraction=double(Hits)/144;
        AddInfo(FString::Printf(TEXT("%s %s hits=%d/144 fraction=%.6f position=%s rotation=%s"),*Parameters,*It->GetName(),Hits,Fraction,*Start.ToString(),*C->GetComponentRotation().ToString()));
        TestTrue(TEXT("At least 85 percent view rays hit geometry within 60m"),Fraction>=.85);
    }
    TestEqual(TEXT("Three cameras audited"),Cameras,3);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHB1PerimeterClutterTest,"Lighthaven.World.Dressing.B1PerimeterClutter",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHB1PerimeterClutterTest::RunTest(const FString&)
{
    auto* Package=LoadPackage(nullptr,TEXT("/Game/Lighthaven/Maps/L_TempleB1"),LOAD_None);
    auto* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
    if(!TestNotNull(TEXT("Regenerated B1 required"),World)) return false;
    TArray<FBox> Walls,Floors,Stairs,Occupied;
    TArray<FVector> Protected,Flames,FloorFlames;
    for(TActorIterator<AActor> It(World);It;++It)
    {
        if(auto* E=Cast<ALHEntranceMarker>(*It)) Protected.Add(E->SafeArrivalTransform.GetLocation());
        else if(It->IsA<ALHSpawnMarker>() || It->IsA<ALHPortal>() || It->IsA<ALHInteractableMarker>()
            || It->Tags.Contains(TEXT("LH.Landmark.Healer"))) Protected.Add(It->GetActorLocation());
        auto* A=Cast<AStaticMeshActor>(*It);
        if(!A || !A->GetActorLabel().StartsWith(TEXT("Geometry_"))) continue;
        const FVector Size=A->GetActorScale3D().GetAbs()*100;
        const FBox Box(A->GetActorLocation()-Size/2,A->GetActorLocation()+Size/2);
        if(Size.Z<=20 && Size.X>=5 && Size.Y>=5)
        {
            if(A->GetActorRotation().IsNearlyZero()) Floors.Add(Box);
            else Stairs.Add(A->GetStaticMeshComponent()->Bounds.GetBox().ExpandBy(FVector(200,200,0)));
        }
        if(Size.Z>=80 && FMath::Min(Size.X,Size.Y)<=40 && A->GetActorRotation().IsNearlyZero()
            && Box.Min.Z<=.1 && Box.Max.Z<=120.1) Walls.Add(Box);
    }
    auto Intersects2D=[](const FBox& A,const FBox& B)
    {return A.Min.X<B.Max.X && A.Max.X>B.Min.X && A.Min.Y<B.Max.Y && A.Max.Y>B.Min.Y;};
    for(TActorIterator<ALHVisualPiece> It(World);It;++It)
        if(auto* Light=It->FindComponentByClass<ULHB1TorchLightComponent>())
        {
            Flames.Add(Light->GetComponentLocation());
            if(It->Tags.Contains(TEXT("LH.B1.FloorFixture"))) FloorFlames.Add(Light->GetComponentLocation());
        }
    TestTrue(TEXT("Actual fixture flames exist for clearance checks"),Flames.Num()>0);
    int32 Clutter=0;
    TSet<FName> Kinds;
    for(TActorIterator<ALHVisualPiece> It(World);It;++It)
    {
        if(!It->Tags.Contains(TEXT("LH.B1.PerimeterClutter"))) continue;
        ++Clutter;
        const auto& Recipe=It->GetRecipe();
        Kinds.Add(Recipe.Id);
        const FString Id=Recipe.Id.ToString();
        const bool Long=Id.EndsWith(TEXT("Table")) || Id.EndsWith(TEXT("Bench"));
        if(Id.EndsWith(TEXT("Debris")) || Id.EndsWith(TEXT("Bench")))
        {
            double Height=0;
            for(const auto& Box:Recipe.Geometry) Height=FMath::Max(Height,Box.Center.Z+Box.Size.Z/2);
            TestTrue(TEXT("Reduced wall-flame clearance applies only to silhouettes below60cm"),Height<60);
        }
        const bool AlongX=FMath::IsNearlyZero(It->GetActorRotation().Yaw);
        const FVector Half=AlongX?FVector(Long?88:55,55,0):FVector(55,Long?88:55,0);
        const FVector Position=It->GetActorLocation();
        const FBox Foot(Position-Half,Position+Half);
        TestTrue(TEXT("Clutter stays at a wall face, outside walkable middle"),Walls.ContainsByPredicate([&](const FBox& W)
        {
            const bool X=W.GetSize().X>=W.GetSize().Y;
            const double Along=X?Position.X:Position.Y,Low=X?W.Min.X:W.Min.Y,High=X?W.Max.X:W.Max.Y;
            const double Normal=X?Position.Y:Position.X;
            const double FaceLow=X?W.Min.Y:W.Min.X,FaceHigh=X?W.Max.Y:W.Max.X;
            return Along>=Low+150 && Along<=High-150
                && FMath::Min(FMath::Abs(Normal-FaceLow),FMath::Abs(Normal-FaceHigh))<=101;
        }));
        bool BroadRoom=false;
        for(const FBox& W:Walls)
        {
            const bool X=W.GetSize().X>=W.GetSize().Y;
            const double Along=X?Position.X:Position.Y,Low=X?W.Min.X:W.Min.Y,High=X?W.Max.X:W.Max.Y;
            const double Normal=X?Position.Y:Position.X;
            const double FaceLow=X?W.Min.Y:W.Min.X,FaceHigh=X?W.Max.Y:W.Max.X;
            if(Along<Low+150 || Along>High-150 || FMath::Min(FMath::Abs(Normal-FaceLow),FMath::Abs(Normal-FaceHigh))>101) continue;
            const double Sign=Normal> (FaceLow+FaceHigh)/2?1.:-1.;
            const FVector Inward=X?FVector(0,Sign,0):FVector(Sign,0,0);
            const FVector Tangent=X?FVector(1,0,0):FVector(0,1,0);
            const FVector End=Position+Inward*600;
            const FBox Strip(Foot.Min.ComponentMin(End-Half),Foot.Max.ComponentMax(End+Half));
            bool Supported=!Walls.ContainsByPredicate([&](const FBox& B){return Intersects2D(Strip,B);});
            for(double Depth=100;Depth<=600;Depth+=100)
            for(double Side:{-1.,0.,1.})
            {
                const FVector Sample=Position+Inward*Depth+Tangent*(Side*(X?Half.X:Half.Y));
                Supported &= Floors.ContainsByPredicate([&](const FBox& F)
                {return Sample.X>=F.Min.X && Sample.X<=F.Max.X && Sample.Y>=F.Min.Y && Sample.Y<=F.Max.Y && FMath::Abs(F.Max.Z)<=.1;});
            }
            BroadRoom |= Supported;
        }
        TestTrue(TEXT("Clutter faces600cm of continuous room depth with no partition crossed; narrow corridors excluded"),BroadRoom);
        for(const FVector& Flame:Flames)
        {
            const FVector Nearest(FMath::Clamp(Flame.X,Foot.Min.X,Foot.Max.X),FMath::Clamp(Flame.Y,Foot.Min.Y,Foot.Max.Y),0);
            const bool Low=Id.EndsWith(TEXT("Debris")) || Id.EndsWith(TEXT("Bench"));
            const double Clearance=Low && !FloorFlames.Contains(Flame)?40.:120.;
            TestTrue(TEXT("Tall clutter and floor bowls stay120cm clear; low clutter stays40cm clear of wall flame projection"),
                FVector::DistSquared2D(Flame,Nearest)>=FMath::Square(Clearance));
        }
        TestFalse(TEXT("No clutter on stairs or stair approaches"),Stairs.ContainsByPredicate([&](const FBox& B){return Intersects2D(Foot,B);}));
        TestFalse(TEXT("No clutter intersects walls"),Walls.ContainsByPredicate([&](const FBox& B){return Intersects2D(Foot,B);}));
        TestFalse(TEXT("Clusters do not overlap each other"),Occupied.ContainsByPredicate([&](const FBox& B){return Intersects2D(Foot,B);}));
        for(const FVector& P:Protected)
        {
            const FVector Nearest(FMath::Clamp(P.X,Foot.Min.X,Foot.Max.X),FMath::Clamp(P.Y,Foot.Min.Y,Foot.Max.Y),0);
            TestTrue(TEXT("Entire footprint clear of arrivals, portals, spawns and services"),FVector::DistSquared2D(P,Nearest)>=FMath::Square(350.));
        }
        for(double X:{Foot.Min.X,Position.X,Foot.Max.X})
        for(double Y:{Foot.Min.Y,Position.Y,Foot.Max.Y})
            TestTrue(TEXT("Whole clutter footprint has flat floor support"),Floors.ContainsByPredicate([&](const FBox& F)
            {return X>=F.Min.X && X<=F.Max.X && Y>=F.Min.Y && Y<=F.Max.Y && FMath::Abs(F.Max.Z)<=.1;}));
        TestEqual(TEXT("Visual clutter collision recipe empty"),Recipe.Collision.Num(),0);
        TestEqual(TEXT("Visual clutter has zero blockers"),It->GetBlockers().Num(),0);
        TestEqual(TEXT("Visual clutter mesh collision disabled"),It->GetMesh()->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
        TestFalse(TEXT("Visual clutter mesh has no navigation contribution"),It->GetMesh()->CanEverAffectNavigation());
        Occupied.Add(Foot);
    }
    TestTrue(TEXT("B1 contains multiple perimeter clusters"),Clutter>=12);
    TestTrue(TEXT("At least three existing kit kinds provide varied silhouettes"),Kinds.Num()>=3);
    TestTrue(TEXT("Clutter stays within bounded presentation budget"),Clutter<=90);
    AddInfo(FString::Printf(TEXT("B1 perimeter clutter: %d pieces, %d kinds"),Clutter,Kinds.Num()));
    return true;
}
