#include "Misc/AutomationTest.h"
#include "Visual/LHEnvArtBinding.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"
#include "World/LHAreaRegistry.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHEnvArtFitTest,"Lighthaven.Visual.EnvArt.StyleTable",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHEnvArtFitTest::RunTest(const FString&)
{
    for(const auto Style:{ELHVisualStyle::Church,ELHVisualStyle::B1Cellar,ELHVisualStyle::B2Damp,ELHVisualStyle::B3Crypt,ELHVisualStyle::B4Ritual})
    {
        TestFalse(TEXT("Cook folder is explicit"),LHEnvArt::Folder(Style).IsEmpty());
        for(const auto& Name:LHEnvArt::AssetNames(Style))
        {
            if(Name==TEXT("Stair600x120Descending")) continue;
            const FString Prefix=Name==TEXT("Carpet") || Name==TEXT("Pillar") || Name==TEXT("DoorLeafPreview")?TEXT("Presentation.Environment.Church."):
                Name==TEXT("ArchBoss500")?TEXT("Presentation.Environment.Basement."):TEXT("Presentation.Environment.Shared.");
            FLHVisualRecipe R; LHEnvArt::FFit Fit;
            TestTrue(*Name,LHVisual::MakeRecipe(FName(*(Prefix+Name)),Style,R));
            TestTrue(*Name,LHEnvArt::Resolve(R,Fit));
            TestEqual(TEXT("Export name matches recipe"),Fit.AssetName,Name);
            TestTrue(TEXT("Has fitted instances"),Fit.Instances.Num()>0);
            const FString Folder=LHEnvArt::MeshFolder(Style,Name);
            const FString Path=TEXT("/Game/Lighthaven/Art/Env/")+Folder+TEXT("/SM_")+Name+TEXT(".SM_")+Name;
            auto* Mesh=LoadObject<UStaticMesh>(nullptr,*Path);
            if(TestNotNull(*(LHEnvArt::Folder(Style)+TEXT("/")+Name),Mesh))
            {
                const FBox Box=Mesh->GetBoundingBox();
                if(Name==TEXT("Wall400")) TestTrue(TEXT("Wall pivot and centimeter envelope"),Box.Min.Equals({0,-20,0},.2) && Box.Max.Equals({400,0,400},.2));
                if(Name==TEXT("Floor400")) TestTrue(TEXT("Floor pivot and centimeter envelope"),Box.Min.Equals({0,0,-20},.2) && Box.Max.Equals({400,400,0},.2));
                if(Name==TEXT("Altar")) TestTrue(TEXT("Altar ground pivot"),Box.Min.Equals({-100,-80,0},.2) && Box.Max.Equals({100,80,120},.2));
                if(Name==TEXT("Pillar")) TestTrue(TEXT("Pillar ground pivot"),Box.Min.Equals({-30,-30,0},.2) && Box.Max.Equals({30,30,400},.2));
                if(Name==TEXT("DoorLeafPreview")) TestTrue(TEXT("Door hinge pivot"),Box.Min.Equals({0,-5,0},.2) && Box.Max.Equals({160,5,300},.2));
                if(Name==TEXT("ArchBoss500")) TestTrue(TEXT("Boss arch pivot"),Box.Min.Equals({-300,-20,0},.2) && Box.Max.Equals({300,0,400},.2));
            }
        }
        FLHVisualRecipe InvalidWall;
        LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Wall400"),Style,InvalidWall);
        InvalidWall.Geometry[0].Size.Y*=2;
        LHEnvArt::FFit InvalidFit;
        TestFalse(TEXT("Unsupported thickness keeps procedural fallback"),LHEnvArt::Resolve(InvalidWall,InvalidFit));
        FLHVisualRecipe Missing; Missing.Style=Style; Missing.Id=TEXT("Presentation.Environment.Shared.NotExported");
        LHEnvArt::FFit Fit;
        TestFalse(TEXT("Missing recipe keeps procedural fallback"),LHEnvArt::Resolve(Missing,Fit));
    }
    FLHVisualRecipe B1; LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Wall400"),ELHVisualStyle::B1Cellar,B1);
    LHEnvArt::FFit Before,After;
    TestTrue(TEXT("Legacy fit"),LHB1Art::Resolve(B1,Before));
    TestTrue(TEXT("Generalized fit"),LHEnvArt::Resolve(B1,After));
    TestEqual(TEXT("B1 instances preserved"),Before.Instances.Num(),After.Instances.Num());
    TestTrue(TEXT("B1 pivot preserved"),Before.Instances[0].Equals(After.Instances[0]));
    const auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    auto* World=UWorld::CreateWorld(EWorldType::Game,false,MakeUniqueObjectName(GetTransientPackage(),UWorld::StaticClass(),TEXT("EnvArtBounds")),GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    for(const auto Style:{ELHVisualStyle::Church,ELHVisualStyle::B2Damp,ELHVisualStyle::B3Crypt,ELHVisualStyle::B4Ritual})
    {
        auto* Piece=LHVisual::SpawnProp(World,TEXT("Presentation.Environment.Shared.Wall400"),FTransform::Identity,Style);
        if(!TestNotNull(TEXT("Wall actor"),Piece)) continue;
        auto Recipe=Piece->GetRecipe(); Recipe.Collision.Reset();
        for(auto& Box:Recipe.Geometry) { Box.Center=(Box.Center-FVector(200,-10,200))*FVector(1.25,1,.125); Box.Size*=FVector(1.25,1,.125); }
        TestTrue(TEXT("Cutaway build"),Piece->Build(Recipe));
        const auto Bounds=Piece->GetComponentsBoundingBox(true);
        TestTrue(TEXT("Cropped lintel bounds retain doorway center"),Bounds.GetCenter().Equals(FVector::ZeroVector,.2));
        TestTrue(TEXT("Whole tiles do not enlarge cropped lintel bounds"),Bounds.GetSize().Equals(FVector(500,20,50),.2));
        Recipe.Id=TEXT("Presentation.Environment.Shared.Torch");
        TestTrue(TEXT("Missing imported piece builds saved geometry"),Piece->Build(Recipe));
        TestFalse(TEXT("Missing imported piece is hidden"),Piece->GetImportedMesh()->IsVisible());
        TestTrue(TEXT("Missing imported piece uses procedural mesh"),Piece->GetMesh()->IsVisible());
        Piece->Destroy();
        auto* Shared=LHVisual::SpawnProp(World,TEXT("Presentation.Environment.Shared.Sconce"),FTransform::Identity,Style);
        if(TestNotNull(TEXT("Shared geometry actor"),Shared) && TestNotNull(TEXT("Shared mesh imported"),Shared->GetImportedMesh()->GetStaticMesh().Get()))
        {
            TestTrue(TEXT("Shared package remains in B1"),Shared->GetImportedMesh()->GetStaticMesh()->GetPathName().StartsWith(TEXT("/Game/Lighthaven/Art/Env/B1/")));
            for(int32 Slot=0;Slot<Shared->GetImportedMesh()->GetNumMaterials();++Slot)
            {
                FString Family=Shared->GetImportedMesh()->GetStaticMesh()->GetMaterial(Slot)->GetName(); Family.RemoveFromStart(TEXT("MI_B1_"));
                const auto* Instance=Cast<UMaterialInstanceDynamic>(Shared->GetImportedMesh()->GetMaterial(Slot));
                if(TestNotNull(TEXT("Style material instance"),Instance)) TestEqual(TEXT("Shared geometry receives requested style material"),Instance->Parent->GetName(),TEXT("MI_")+LHEnvArt::Folder(Style)+TEXT("_")+Family);
            }
            // Null references reproduce a kit that could not be loaded without
            // renaming packages used by other tests or the cook process.
            auto* Property=FindFProperty<FMapProperty>(ALHVisualPiece::StaticClass(),TEXT("EnvAssets"));
            if(TestNotNull(TEXT("Mesh reference table"),Property))
            {
                auto* References=Property->ContainerPtrToValuePtr<TMap<FString,TObjectPtr<UStaticMesh>>>(Shared);
                const FString Key=LHEnvArt::Folder(Style)+TEXT("/SM_Sconce");
                auto* Reference=References->Find(Key);
                if(TestNotNull(TEXT("Known piece mesh reference"),Reference))
                {
                    const auto SavedMesh=*Reference; *Reference=nullptr;
                    TestTrue(TEXT("Missing loaded mesh still builds"),Shared->Build(Shared->GetRecipe()));
                    TestFalse(TEXT("Missing loaded mesh hides imported renderer"),Shared->GetImportedMesh()->IsVisible());
                    TestTrue(TEXT("Missing loaded mesh uses procedural renderer"),Shared->GetMesh()->IsVisible());
                    *Reference=SavedMesh;
                }
            }
            auto* MaterialsProperty=FindFProperty<FArrayProperty>(ALHVisualPiece::StaticClass(),TEXT("EnvMaterials"));
            if(TestNotNull(TEXT("Style material references"),MaterialsProperty))
            {
                auto* References=MaterialsProperty->ContainerPtrToValuePtr<TArray<TObjectPtr<UMaterialInterface>>>(Shared);
                References->Reset();
                TestTrue(TEXT("Missing style materials still builds"),Shared->Build(Shared->GetRecipe()));
                TestFalse(TEXT("Shared B1 mesh does not substitute a missing style palette"),Shared->GetImportedMesh()->IsVisible());
                TestTrue(TEXT("Missing style materials uses procedural renderer"),Shared->GetMesh()->IsVisible());
            }
        }
        if(Shared) Shared->Destroy();
    }
    World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
    return true;
}
// Audit all fifteen cameras against the imported style envelopes and clipping.
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FLHEnvCaptureFramingTest,"Lighthaven.Visual.EnvArt.CaptureFraming",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FLHEnvCaptureFramingTest::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
    for(const auto& Area:LHWorld::Registry()) { Names.Add(Area.Id.Content.Value.ToString()); Commands.Add(Area.Id.Content.Value.ToString()); }
}
bool FLHEnvCaptureFramingTest::RunTest(const FString& Parameters)
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
            LHEnvArt::FFit Fit;
            if(Imported->IsVisible() && Imported->GetStaticMesh() && LHEnvArt::Resolve(P->GetRecipe(),Fit))
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

#endif
