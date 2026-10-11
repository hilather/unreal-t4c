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
    const auto Backing=LHB1Art::SolidBacking(R,Fit);
    TestEqual(TEXT("Cutaway has one closed masonry core"),Backing.Num(),1);
    if(Backing.Num()==1)
    {
        AddInfo(FString::Printf(TEXT("Actual wall core center=%s size=%s fitMin=%s fitMax=%s"),*Backing[0].Center.ToString(),*Backing[0].Size.ToString(),*Fit.ClipBounds.Min.ToString(),*Fit.ClipBounds.Max.ToString()));
        TestTrue(TEXT("Mortar core recedes beneath coping at ends and top"),Backing[0].Center.Equals(Fit.ClipBounds.GetCenter()) &&
            FMath::IsNearlyEqual(Backing[0].Size.X,997.) && FMath::IsNearlyEqual(Backing[0].Size.Z,117.));
        TestTrue(TEXT("Broad faces inset behind imported stone"),Backing[0].Size.Y<Fit.ClipBounds.GetSize().Y);
    }
    const auto CheckCapOverlaps=[&](const TArray<FLHVisualBox>& Stones)
    {
        for(int32 A=0;A<Stones.Num();++A) for(int32 B=A+1;B<Stones.Num();++B)
        {
            const FVector AMin=Stones[A].Center-Stones[A].Size/2, AMax=Stones[A].Center+Stones[A].Size/2;
            const FVector BMin=Stones[B].Center-Stones[B].Size/2, BMax=Stones[B].Center+Stones[B].Size/2;
            const FVector Overlap(FMath::Min(AMax.X,BMax.X)-FMath::Max(AMin.X,BMin.X),
                FMath::Min(AMax.Y,BMax.Y)-FMath::Max(AMin.Y,BMin.Y),
                FMath::Min(AMax.Z,BMax.Z)-FMath::Max(AMin.Z,BMin.Z));
            TestFalse(*FString::Printf(TEXT("Coping stones %d/%d cannot overlap in volume or create coplanar corner faces"),A,B),
                Overlap.X>.0001 && Overlap.Y>.0001 && Overlap.Z>.0001);
        }
    };
    const auto Caps=LHB1Art::MasonryCaps(R,Fit);
    CheckCapOverlaps(Caps);
    TestTrue(TEXT("Cutaway coping has individually sized stones"),Caps.Num()>12);
    bool DifferentLengths=false, HasJoint=false;
    double LastTopEnd=Fit.ClipBounds.Min.X;
    for(const auto& B:Caps)
    {
        const FVector Min=B.Center-B.Size/2, Max=B.Center+B.Size/2;
        TestTrue(TEXT("Cap never grows exact fitted silhouette"),
            Fit.ClipBounds.IsInsideOrOn(Min) && Fit.ClipBounds.IsInsideOrOn(Max));
        TestTrue(TEXT("Cap closes full masonry thickness"),FMath::IsNearlyEqual(B.Size.Y,20.));
        if(FMath::IsNearlyEqual(Max.Z,Fit.ClipBounds.Max.Z) && B.Size.Z<=6.01)
        {
            HasJoint|=Min.X>LastTopEnd+.1;
            DifferentLengths|=!FMath::IsNearlyEqual(B.Size.X,Caps[0].Size.X,.1);
            LastTopEnd=Max.X;
        }
    }
    TestTrue(TEXT("Coping uses varied stone lengths"),DifferentLengths);
    TestTrue(TEXT("Coping leaves visible recessed mortar joints"),HasJoint);
    TestTrue(TEXT("Last coping stone reaches exact crop endpoint"),FMath::IsNearlyEqual(LastTopEnd,Fit.ClipBounds.Max.X));
    FBox Combined(ForceInit);
    for(const auto& B:Backing) Combined+=FBox(B.Center-B.Size/2,B.Center+B.Size/2);
    for(const auto& B:Caps) Combined+=FBox(B.Center-B.Size/2,B.Center+B.Size/2);
    TestTrue(TEXT("Core and coping together retain exact fitted envelope"),Combined.Min.Equals(Fit.ClipBounds.Min) && Combined.Max.Equals(Fit.ClipBounds.Max));
    if(Backing.Num()==1) TestTrue(TEXT("Joints expose a recessed mortar bed"),
        FMath::IsNearlyEqual(Backing[0].Center.Z+Backing[0].Size.Z/2,Fit.ClipBounds.Max.Z-1.5));
    const uint32 Unchanged=R.Fingerprint(); LHB1Art::SolidBacking(R,Fit); LHB1Art::MasonryCaps(R,Fit);
    TestEqual(TEXT("Backing never mutates gameplay recipe"),R.Fingerprint(),Unchanged);
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
    LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Arch240"),ELHVisualStyle::B1Cellar,R);
    TestTrue(TEXT("Arch resolves"),LHB1Art::Resolve(R,Fit));
    const auto ArchBacking=LHB1Art::SolidBacking(R,Fit);
    TestEqual(TEXT("Arch closed uprights and crown"),ArchBacking.Num(),3);
    if(ArchBacking.Num()==3) TestTrue(TEXT("Crown does not fill segmental aperture"),
        FMath::IsNearlyEqual(ArchBacking[2].Center.Z-ArchBacking[2].Size.Z/2,357.));
    for(const TCHAR* Name:{TEXT("Arch240"),TEXT("Arch320")})
    {
        LHVisual::MakeRecipe(FName(*(FString(TEXT("Presentation.Environment.Shared."))+Name)),ELHVisualStyle::B1Cellar,R);
        LHB1Art::Resolve(R,Fit);
        const auto ArchCaps=LHB1Art::MasonryCaps(R,Fit);
        CheckCapOverlaps(ArchCaps);
        const auto RecessedArch=LHB1Art::SolidBacking(R,Fit);
        if(TestEqual(TEXT("Arch retains three closed backing regions"),RecessedArch.Num(),3))
        {
            for(const auto& B:RecessedArch) AddInfo(FString::Printf(TEXT("%s actual arch core center=%s size=%s"),Name,*B.Center.ToString(),*B.Size.ToString()));
            TestTrue(TEXT("Arch core top recedes below coping joints"),FMath::IsNearlyEqual(RecessedArch[2].Center.Z+RecessedArch[2].Size.Z/2,398.5));
            TestTrue(TEXT("Arch jamb outer beds recede while inner reveals remain exact"),
                FMath::IsNearlyEqual(RecessedArch[0].Center.X-RecessedArch[0].Size.X/2,-198.5) &&
                FMath::IsNearlyEqual(RecessedArch[1].Center.X+RecessedArch[1].Size.X/2,198.5) &&
                FMath::IsNearlyEqual(RecessedArch[0].Center.X+RecessedArch[0].Size.X/2,R.Geometry[0].Center.X+R.Geometry[0].Size.X/2) &&
                FMath::IsNearlyEqual(RecessedArch[1].Center.X-RecessedArch[1].Size.X/2,R.Geometry[1].Center.X-R.Geometry[1].Size.X/2));
        }
        TestTrue(TEXT("Arch has segmented crown and outer jamb caps"),ArchCaps.Num()>8);
        for(const auto& B:ArchCaps)
        {
            const FVector Min=B.Center-B.Size/2, Max=B.Center+B.Size/2;
            TestTrue(TEXT("Arch cap stays in masonry envelope"),Min.X>=-200. && Max.X<=200. && Min.Z>=0. && Max.Z<=400.);
            TestTrue(TEXT("Caps never enter segmental aperture"),Min.Z>=357. || Max.X<=R.Geometry[0].Center.X+R.Geometry[0].Size.X/2 ||
                Min.X>=R.Geometry[1].Center.X-R.Geometry[1].Size.X/2);
        }
        R.Style=ELHVisualStyle::B2Damp;
        TestEqual(TEXT("Other styles receive no B1 caps"),LHB1Art::MasonryCaps(R,Fit).Num(),0);
    }
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
        if(const auto* Cap=Piece->GetArtSupport()->GetProcMeshSection(0))
            for(int32 Base=0;Base+3<Cap->ProcVertexBuffer.Num();Base+=4)
            {
                const FVector U=(Cap->ProcVertexBuffer[Base+1].Position-Cap->ProcVertexBuffer[Base].Position).GetSafeNormal();
                const FVector V=(Cap->ProcVertexBuffer[Base+3].Position-Cap->ProcVertexBuffer[Base].Position).GetSafeNormal();
                for(int32 Corner=0;Corner<4;++Corner)
                {
                    const auto& Vertex=Cap->ProcVertexBuffer[Base+Corner];
                    const FVector T=Vertex.Tangent.TangentX;
                    TestTrue(TEXT("Masonry tangent follows increasing UV U"),T.Equals(U,.0001));
                    TestTrue(TEXT("Masonry tangent has unit length and is orthogonal to normal"),
                        FMath::IsNearlyEqual(T.Size(),1.,.0001) && FMath::Abs(FVector::DotProduct(T,Vertex.Normal))<.0001);
                    const FVector Bitangent=FVector::CrossProduct(Vertex.Normal,T)*(Vertex.Tangent.bFlipTangentY?-1.:1.);
                    TestTrue(TEXT("Masonry tangent handedness follows increasing UV V"),Bitangent.Equals(V,.0001));
                }
            }
        TestEqual(TEXT("Masonry support has no collision"),Piece->GetArtSupport()->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
        TestFalse(TEXT("Masonry support has no navigation"),Piece->GetArtSupport()->CanEverAffectNavigation());
        if(Name==TEXT("Wall400") || Name.StartsWith(TEXT("Arch")))
        {
            const auto* Support=Piece->GetArtSupport()->GetProcMeshSection(0);
            if(TestNotNull(TEXT("Masonry coping generated"),Support) && Support->ProcVertexBuffer.Num()>24)
            {
                const FColor Core=Support->ProcVertexBuffer[0].Color;
                bool Varied=false;
                for(const auto& Vertex:Support->ProcVertexBuffer)
                {
                    Varied|=Vertex.Color!=Core;
                    TestTrue(TEXT("Masonry colors retain warm channel order"),Vertex.Color.R>=Vertex.Color.G && Vertex.Color.G>=Vertex.Color.B);
                }
                TestTrue(TEXT("Masonry courses carry individual stone tones"),Varied);
            }
        }
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
    auto* Floor=LHVisual::SpawnProp(World,TEXT("Presentation.Environment.Shared.Floor400"),FTransform::Identity,ELHVisualStyle::B1Cellar);
    auto* Fixture=LHVisual::SpawnProp(World,TEXT("Presentation.Environment.Shared.Sconce"),FTransform(FVector(200,200,250)),ELHVisualStyle::B1Cellar);
    if(TestNotNull(TEXT("Stand fixture"),Fixture) && TestNotNull(TEXT("Stand floor"),Floor))
    {
        const auto Copy=Fixture->GetRecipe();
        TestTrue(TEXT("Refresh after floor collision registered"),Fixture->Build(Copy));
        const auto* Support=Fixture->GetArtSupport()->GetProcMeshSection(0);
        if(TestNotNull(TEXT("Unsupported fixture receives stand"),Support))
        {
            TestTrue(TEXT("Stand touches floor and fixture bracket"),FMath::IsNearlyEqual(Support->SectionLocalBox.Min.Z,-250.,.1) &&
                FMath::IsNearlyEqual(Support->SectionLocalBox.Max.Z,-8.,.1));
        }
        TestEqual(TEXT("Stand carries zero collision"),Fixture->GetArtSupport()->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
        TestFalse(TEXT("Stand cannot affect route nav"),Fixture->GetArtSupport()->CanEverAffectNavigation());
        TestEqual(TEXT("Stand leaves recipe fingerprint unchanged"),Fixture->GetRecipe().Fingerprint(),Copy.Fingerprint());
        TestTrue(TEXT("Stand preserves placement"),Fixture->GetActorLocation().Equals(FVector(200,200,250)));
        // A retained full-height wall can contain the stand ray origin although its
        // visible cutaway is lower. Ignore that penetration and reach actual ground.
        auto* Wall=LHVisual::SpawnProp(World,TEXT("Presentation.Environment.Shared.Wall400"),
            FTransform(FVector(0,235,0)),ELHVisualStyle::B1Cellar);
        if(TestNotNull(TEXT("Tall hidden wall intersects fixture support ray"),Wall))
        {
            TestTrue(TEXT("Refresh stand through overlapping wall"),Fixture->Build(Copy));
            const auto* Grounded=Fixture->GetArtSupport()->GetProcMeshSection(0);
            if(TestNotNull(TEXT("Stand survives penetrating wall hit"),Grounded))
                TestTrue(TEXT("Penetrating wall is rejected in favour of real floor"),
                    FMath::IsNearlyEqual(Grounded->SectionLocalBox.Min.Z,-250.,.1));
            TestTrue(TEXT("Wall blocker retained"),Wall->GetBlockers().Num()==1);
            Wall->Destroy();
        }
        Fixture->Destroy(); Floor->Destroy();
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
