#include "Misc/AutomationTest.h"
#include "Visual/LHB1ArtBinding.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHB1ArtFitTest,"Lighthaven.Visual.B1Art.FitAndFallback",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHB1ArtFitTest::RunTest(const FString&)
{
    FLHVisualRecipe R; LHB1Art::FFit Fit;
    const FName Wall(TEXT("Presentation.Environment.Shared.Wall400"));
    LHVisual::MakeRecipe(Wall,ELHVisualStyle::B1Cellar,R);
    TestTrue(TEXT("B1 wall resolves"),LHB1Art::Resolve(R,Fit));
    TestEqual(TEXT("Canonical single tile"),Fit.Instances.Num(),1);
    TestTrue(TEXT("Canonical pivot"),Fit.Instances[0].Equals(FTransform::Identity));
    const FVector Scale(2.5,1,.3), Pivot(200,-10,200);
    for(auto& B:R.Geometry) { B.Center=(B.Center-Pivot)*Scale; B.Size*=Scale; }
    TestTrue(TEXT("Saved cutaway recipe tiles"),LHB1Art::Resolve(R,Fit));
    TestEqual(TEXT("1000cm wall has three unscaled tiles"),Fit.Instances.Num(),3);
    TestTrue(TEXT("Cutaway bounds exactly recentered"),Fit.ClipBounds.Min.Equals(FVector(-500,-10,-60)) && Fit.ClipBounds.Max.Equals(FVector(500,10,60)));
    for(const auto& T:Fit.Instances) TestTrue(TEXT("UV density never stretched"),T.GetScale3D().Equals(FVector::OneVector));
    for(auto Style:{ELHVisualStyle::Church,ELHVisualStyle::B2Damp,ELHVisualStyle::B3Crypt,ELHVisualStyle::B4Ritual})
    { R.Style=Style; TestFalse(TEXT("Other styles fall back"),LHB1Art::Resolve(R,Fit)); }
    LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Stair600x120"),ELHVisualStyle::B1Cellar,R,true);
    TestTrue(TEXT("Descending variant"),LHB1Art::Resolve(R,Fit));
    TestEqual(TEXT("Signed asset name"),Fit.AssetName,FString(TEXT("Stair600x120Descending")));
    TestTrue(TEXT("Unfitted descending pivot"),Fit.Instances[0].Equals(FTransform::Identity,.01));
    const FRotator Slope(FMath::RadiansToDegrees(FMath::Atan2(-100.,400.)),0,0);
    R.Geometry[0].Center=-Slope.RotateVector(FVector::UpVector)*10;
    R.Geometry[0].Rotation=Slope; R.Geometry[0].Size=FVector(FMath::Sqrt(170000.),300,20);
    TestTrue(TEXT("Short fitted descending stair crops nearest mesh"),LHB1Art::Resolve(R,Fit));
    TestTrue(TEXT("Short stair retains unit mesh scale"),Fit.Instances[0].GetScale3D().Equals(FVector::OneVector));
    const FVector End=Fit.Instances[0].TransformPosition(FVector(600,0,-120));
    TestTrue(TEXT("Rotated path slope matches saved collider"),FMath::IsNearlyEqual((End.Z-Fit.Instances[0].GetLocation().Z)/(End.X-Fit.Instances[0].GetLocation().X),-.25,.0001));
    LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Door240"),ELHVisualStyle::B1Cellar,R);
    TestFalse(TEXT("Unexported IDs fall back"),LHB1Art::Resolve(R,Fit));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHB1ArtAssetsTest,"Lighthaven.Visual.B1Art.ImportedBoundsPivotsAndLifecycle",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHB1ArtAssetsTest::RunTest(const FString&)
{
    struct FExpected { const TCHAR* Name; FVector Min,Max; };
    const FExpected Bounds[] = {
        {TEXT("Wall400"),{0,-20,0},{400,0,400}}, {TEXT("Floor400"),{0,0,-20},{400,400,0}},
        {TEXT("Stair600x120"),{0,-150,-19.6116},{603.9223,150,120}},
        {TEXT("Stair600x120Descending"),{-3.9223,-150,-139.6116},{600,150,0}},
        {TEXT("Arch240"),{-200,-20,0},{200,0,400}}, {TEXT("Arch320"),{-200,-20,0},{200,0,400}},
        {TEXT("Sconce"),{-12.5,-6,-20},{12.5,34,26}}, {TEXT("Barrel"),{-31,-31,0},{31,31,100}},
        {TEXT("Crate"),{-41,-41,0},{41,41,80}}, {TEXT("Debris"),{-44,-23.526,0},{49.885,24.324,12}},
        {TEXT("Table"),{-80,-45,0},{80,45,80}}, {TEXT("Bench"),{-80,-22.5,0},{80,22.5,45}}
    };
    const auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    auto* World=UWorld::CreateWorld(EWorldType::Game,false,MakeUniqueObjectName(GetTransientPackage(),UWorld::StaticClass(),TEXT("B1ArtTest")),GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    for(const auto& B:Bounds)
    {
        const FString Name(B.Name), Path=TEXT("/Game/Lighthaven/Art/Env/B1/SM_")+Name+TEXT(".SM_")+Name;
        auto* Mesh=LoadObject<UStaticMesh>(nullptr,*Path);
        if(!TestNotNull(*Name,Mesh)) continue;
        const FBox Actual=Mesh->GetBoundingBox();
        TestTrue(*(Name+TEXT(" min cm/pivot")),Actual.Min.Equals(B.Min,.2));
        TestTrue(*(Name+TEXT(" max cm/pivot")),Actual.Max.Equals(B.Max,.2));
        FString IdName=Name; const bool Descending=IdName.RemoveFromEnd(TEXT("Descending"));
        auto* Piece=LHVisual::SpawnProp(World,FName(*(TEXT("Presentation.Environment.Shared.")+IdName)),FTransform::Identity,ELHVisualStyle::B1Cellar,Descending);
        if(!TestNotNull(TEXT("Build imported piece"),Piece)) continue;
        TestEqual(TEXT("Correct asset bound"),Piece->GetImportedMesh()->GetStaticMesh().Get(),Mesh);
        TestTrue(TEXT("Imported visible"),Piece->GetImportedMesh()->IsVisible());
        TestFalse(TEXT("Fallback hidden"),Piece->GetMesh()->IsVisible());
        TestEqual(TEXT("Render has no collision"),Piece->GetImportedMesh()->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
        TestFalse(TEXT("Render has no navigation"),Piece->GetImportedMesh()->CanEverAffectNavigation());
        const auto Copy=Piece->GetRecipe();
        TestTrue(TEXT("Rebuild"),Piece->Build(Copy));
        TestEqual(TEXT("No duplicate instances"),Piece->GetImportedMesh()->GetInstanceCount(),1);
        FLHVisualRecipe Other=Copy; Other.Style=ELHVisualStyle::B2Damp;
        TestTrue(TEXT("Other style rebuild"),Piece->Build(Other));
        TestFalse(TEXT("Other style imported hidden"),Piece->GetImportedMesh()->IsVisible());
        TestTrue(TEXT("Other style procedural fallback visible"),Piece->GetMesh()->IsVisible());
        Piece->Destroy();
    }
    for(const TCHAR* Family:{TEXT("stone"),TEXT("timber"),TEXT("iron")})
        for(const TCHAR* Suffix:{TEXT("basecolor"),TEXT("normal"),TEXT("orm")})
        {
            const FString Name=FString(TEXT("T_"))+Family+TEXT("_")+Suffix;
            const FString Path=TEXT("/Game/Lighthaven/Art/Env/B1/Textures/")+Name+TEXT(".")+Name;
            auto* Texture=LoadObject<UTexture2D>(nullptr,*Path);
            if(!TestNotNull(*Name,Texture)) continue;
            const bool Normal=FString(Suffix)==TEXT("normal"), ORM=FString(Suffix)==TEXT("orm");
            TestEqual(TEXT("Texture color space"),bool(Texture->SRGB),!(Normal||ORM));
            if(Normal) { TestEqual(TEXT("Normal compression"),Texture->CompressionSettings.GetValue(),TC_Normalmap); TestTrue(TEXT("OpenGL green converted"),bool(Texture->bFlipGreenChannel)); }
            if(ORM) TestEqual(TEXT("Linear ORM mask compression"),Texture->CompressionSettings.GetValue(),TC_Masks);
        }
    World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
    return true;
}
#endif
