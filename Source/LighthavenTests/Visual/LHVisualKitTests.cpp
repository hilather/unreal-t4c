#include "Misc/AutomationTest.h"
#include "Visual/LHVisualKit.h"
#include "ProceduralMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Engine/Texture.h"
#include "Engine/Engine.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace LHVisualTestsPrivate
{
struct FWorld
{
    UWorld* World=nullptr;
    FWorld()
    {
        const auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
        World=UWorld::CreateWorld(EWorldType::Game,false,MakeUniqueObjectName(GetTransientPackage(),UWorld::StaticClass(),TEXT("VisualKitTest")),GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
        GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    }
    ~FWorld() { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); }
};
FName Id(const TCHAR* S) { return FName(FString(TEXT("Presentation.Environment."))+S); }
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHVisualCatalogTest,"Lighthaven.Visual.CatalogBuildCollisionBudgets",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHVisualCatalogTest::RunTest(const FString&)
{
    LHVisualTestsPrivate::FWorld Host;
    TArray<ALHVisualPiece*> All;
    TestEqual(TEXT("Stable catalog count"),LHVisual::PieceIds().Num(),26);
    for(int32 Style=0;Style<5;++Style) for(FName Id:LHVisual::PieceIds())
    {
        const FTransform T(FVector(All.Num()*1000.f,0,0));
        auto* P=LHVisual::SpawnProp(Host.World,Id,T,ELHVisualStyle(Style));
        if(!TestNotNull(*Id.ToString(),P)) continue;
        All.Add(P); const auto& R=P->GetRecipe();
        const bool Decor=Id.ToString().EndsWith(TEXT("Torch")) || Id.ToString().EndsWith(TEXT("Sconce")) || Id.ToString().EndsWith(TEXT("Debris"));
        TestEqual(TEXT("Decor never blocks; all solid pieces do"),P->GetBlockers().IsEmpty(),Decor);
        TestEqual(TEXT("Render has no collision"),P->GetMesh()->GetCollisionProfileName(),FName(TEXT("NoCollision")));
        for(int32 I=0;I<P->GetBlockers().Num();++I)
        {
            auto* B=P->GetBlockers()[I].Get();
            TestEqual(TEXT("Solid profile"),B->GetCollisionProfileName(),FName(TEXT("BlockAll")));
            TestEqual(TEXT("Blocks pawn"),B->GetCollisionResponseToChannel(ECC_Pawn),ECR_Block);
            TestEqual(TEXT("Blocks visibility"),B->GetCollisionResponseToChannel(ECC_Visibility),ECR_Block);
            TestTrue(TEXT("Extent"),B->GetUnscaledBoxExtent().Equals(R.Collision[I].Size/2));
        }
        for(int32 I=0;I<P->GetMesh()->GetNumSections();++I)
        {
            const auto* Section=P->GetMesh()->GetProcMeshSection(I); if(!Section || Section->ProcIndexBuffer.IsEmpty()) continue;
            auto* M=Cast<UMaterialInstanceDynamic>(P->GetMesh()->GetMaterial(I));
            if(!TestNotNull(TEXT("Dynamic engine material"),M)) continue;
            FLinearColor Color;
            TestTrue(TEXT("Color is exposed"),M->GetVectorParameterValue(FMaterialParameterInfo(TEXT("Color")),Color));
            TestTrue(TEXT("Color parameter matches recipe"),Color.Equals(R.Colors[I]));
            const bool ImportedFlame=R.Style==ELHVisualStyle::B1Cellar &&
                Id==LHVisualTestsPrivate::Id(TEXT("Shared.Torch")) && I==1;
            if(ImportedFlame)
            {
                TestEqual(TEXT("B1 flame uses imported emissive parent"),M->Parent->GetPathName(),
                    FString(TEXT("/Game/Lighthaven/Art/Env/B1/MI_B1_flame.MI_B1_flame")));
                float Emission=0;
                TestTrue(TEXT("Imported flame exposes emission"),M->Parent->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Flame")),Emission));
                TestTrue(TEXT("Imported flame is emissive"),Emission>0);
                // Imported master uses ORM G for roughness, not the MID's unused scalar override.
                UTexture* ORM=nullptr;
                TestTrue(TEXT("Imported parent exposes ORM"),M->Parent->GetTextureParameterValue(FMaterialParameterInfo(TEXT("ORM")),ORM));
                if(TestNotNull(TEXT("Imported roughness texture is bound"),ORM))
                {
                    TestFalse(TEXT("ORM is linear"),bool(ORM->SRGB));
                    TestEqual(TEXT("ORM uses mask compression"),ORM->CompressionSettings.GetValue(),TC_Masks);
                }
            }
            else
            {
                TArray<FMaterialParameterInfo> Parameters; TArray<FGuid> ParameterIds;
                M->Parent->GetAllScalarParameterInfo(Parameters,ParameterIds);
                TestTrue(TEXT("Roughness exists in engine parent"),Parameters.ContainsByPredicate([](const FMaterialParameterInfo& Info){ return Info.Name==TEXT("Roughness"); }));
                float Roughness=-1;
                TestTrue(TEXT("Engine Roughness is exposed"),M->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Roughness")),Roughness));
                TestEqual(TEXT("Roughness matches recipe"),Roughness,R.Roughness[I]);
            }
        }
    }
    const auto Totals=LHVisual::ValidatePlacedSet(All);
    TestEqual(TEXT("All style/piece combinations"),Totals.Pieces,130);
    TestTrue(TEXT("Measured budgets/collision pass"),Totals.Errors.IsEmpty());
    AddInfo(FString::Printf(TEXT("Placed set: %d pieces, %d triangles, %d base sections, %d collision boxes"),Totals.Pieces,Totals.Triangles,Totals.DrawCalls,Totals.CollisionBoxes));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHVisualDeterminismTest,"Lighthaven.Visual.DeterminismAndRebuild",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHVisualDeterminismTest::RunTest(const FString&)
{
    LHVisualTestsPrivate::FWorld Host;
    for(int32 Style=0;Style<5;++Style) for(FName Id:LHVisual::PieceIds())
    {
        auto* A=LHVisual::SpawnProp(Host.World,Id,FTransform::Identity,ELHVisualStyle(Style));
        auto* B=LHVisual::SpawnProp(Host.World,Id,FTransform(FVector(1000,0,0)),ELHVisualStyle(Style));
        if(!TestNotNull(TEXT("First"),A) || !TestNotNull(TEXT("Repeat"),B)) continue;
        TestEqual(TEXT("Recipe fingerprint"),A->GetRecipe().Fingerprint(),B->GetRecipe().Fingerprint());
        for(int32 S=0;S<A->GetMesh()->GetNumSections();++S)
        {
            const auto* X=A->GetMesh()->GetProcMeshSection(S); const auto* Y=B->GetMesh()->GetProcMeshSection(S);
            if(!X || !Y) { TestTrue(TEXT("Section holes identical"),X==Y); continue; }
            TestTrue(TEXT("Exact topology"),X->ProcIndexBuffer==Y->ProcIndexBuffer);
            for(int32 I=0;I<X->ProcIndexBuffer.Num();I+=3)
            {
                const auto& P0=X->ProcVertexBuffer[X->ProcIndexBuffer[I]];
                const auto& P1=X->ProcVertexBuffer[X->ProcIndexBuffer[I+1]];
                const auto& P2=X->ProcVertexBuffer[X->ProcIndexBuffer[I+2]];
                const FVector Cross=FVector::CrossProduct(P1.Position-P0.Position,P2.Position-P0.Position);
                TestTrue(TEXT("Nondegenerate clockwise front facing outward normal"),FVector::DotProduct(Cross,P0.Normal)<0);
            }
            TestEqual(TEXT("Vertex count"),X->ProcVertexBuffer.Num(),Y->ProcVertexBuffer.Num());
            for(int32 V=0;V<X->ProcVertexBuffer.Num();++V)
            {
                TestTrue(TEXT("Exact positions"),X->ProcVertexBuffer[V].Position==Y->ProcVertexBuffer[V].Position);
                TestTrue(TEXT("Exact normals"),X->ProcVertexBuffer[V].Normal==Y->ProcVertexBuffer[V].Normal);
                TestTrue(TEXT("Exact UV"),X->ProcVertexBuffer[V].UV0==Y->ProcVertexBuffer[V].UV0);
            }
        }
        FLHVisualRecipe R=A->GetRecipe(); const int32 Count=A->GetBlockers().Num();
        TestTrue(TEXT("Rebuild succeeds"),A->Build(R)); TestEqual(TEXT("No duplicate blockers"),A->GetBlockers().Num(),Count);
        auto T=LHVisual::ValidatePlacedSet({A}); TestTrue(TEXT("Rebuilt budget"),T.Errors.IsEmpty());
        A->Destroy(); B->Destroy();
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHVisualAssemblyTest,"Lighthaven.Visual.WallRunClearanceAndInvalidInputs",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHVisualAssemblyTest::RunTest(const FString&)
{
    using namespace LHVisualTestsPrivate;
    FWorld Host;
    auto Run=LHVisual::SpawnWallRun(Host.World,{20,30,0},{20,880,0},ELHVisualStyle::B2Damp);
    TestEqual(TEXT("850 cm run splits in three"),Run.Num(),3);
    if(Run.Num()==3)
    {
        TestTrue(TEXT("Rotation follows +Y"),Run[0]->GetActorRotation().Equals(FRotator(0,90,0)));
        TestTrue(TEXT("Last 50 cm infill"),Run[2]->GetRecipe().Collision[0].Size.Equals(FVector(50,20,400)));
        TestTrue(TEXT("Final pivot"),Run[2]->GetActorLocation().Equals(FVector(20,830,0)));
        TestTrue(TEXT("Run budget"),LHVisual::ValidatePlacedSet(Run).Errors.IsEmpty());
    }
    TestTrue(TEXT("Vertical invalid"),LHVisual::SpawnWallRun(Host.World,{0,0,0},{0,0,100},ELHVisualStyle::Church).IsEmpty());
    TestNull(TEXT("Unknown piece"),LHVisual::SpawnProp(Host.World,TEXT("Invalid"),FTransform::Identity));
    TestNull(TEXT("Scale cannot resize clearance"),LHVisual::SpawnProp(Host.World,Id(TEXT("Shared.Door240")),FTransform(FQuat::Identity,FVector::ZeroVector,FVector(2))));
    for(const TCHAR* S:{TEXT("Shared.Arch240"),TEXT("Shared.Arch320"),TEXT("Basement.ArchBoss500")})
    {
        FLHVisualRecipe R; LHVisual::MakeRecipe(Id(S),ELHVisualStyle::B4Ritual,R);
        const bool Boss=FString(S).Contains(TEXT("Boss")); const float W=Boss?500:FString(S).EndsWith(TEXT("240"))?240:320,H=Boss?360:300;
        const FBox Clear(FVector(-W/2+.01f,-20,0),FVector(W/2-.01f,0,H-.01f));
        for(const auto& B:R.Geometry) TestFalse(TEXT("Arch geometry preserves rectangular aperture"),Clear.Intersect(FBox(B.Center-B.Size/2,B.Center+B.Size/2)));
        for(const auto& B:R.Collision) TestFalse(TEXT("Frame blockers preserve aperture"),Clear.Intersect(FBox(B.Center-B.Size/2,B.Center+B.Size/2)));
    }
    auto* P=LHVisual::SpawnProp(Host.World,Id(TEXT("Shared.Crate")),FTransform::Identity);
    if(P)
    {
        FLHVisualRecipe Bad=P->GetRecipe(); const uint32 Before=Bad.Fingerprint(); Bad.DrawBudget=0;
        TestFalse(TEXT("Reject overbudget atomically"),P->Build(Bad)); TestEqual(TEXT("Old build retained"),P->GetRecipe().Fingerprint(),Before);
        TestFalse(TEXT("Duplicate references rejected"),LHVisual::ValidatePlacedSet({P,P}).Errors.IsEmpty());
        P->GetBlockers()[0]->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
        TestFalse(TEXT("Changed blocking response detected"),LHVisual::ValidatePlacedSet({P}).Errors.IsEmpty());
    }
    TestFalse(TEXT("Null set rejected"),LHVisual::ValidatePlacedSet({nullptr}).Errors.IsEmpty());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHVisualStairTest,"Lighthaven.Visual.SignedStairsPhysicsAndRecipeSerialization",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHVisualStairTest::RunTest(const FString&)
{
    using namespace LHVisualTestsPrivate;
    FWorld Host;
    const TCHAR* Profiles[]={TEXT("Shared.Stair400x100"),TEXT("Shared.Stair600x120"),TEXT("Shared.Stair250x80"),TEXT("Shared.Stair300x80")};
    const float Runs[]={400,600,250,300}, Rises[]={100,120,80,80};
    int32 Piece=0;
    for(int32 I=0;I<4;++I) for(bool Descending:{false,true})
    {
        const float Rise=Rises[I]*(Descending?-1:1);
        const FVector Origin(Piece++*1000.f,0,0);
        auto* A=LHVisual::SpawnProp(Host.World,Id(Profiles[I]),FTransform(Origin),ELHVisualStyle::B4Ritual,Descending);
        if(!TestNotNull(TEXT("Signed stair builds"),A)) continue;
        for(const auto& C:A->GetBlockers()) C->RecreatePhysicsState();
        FHitResult Hit;
        const FVector Center=Origin+FVector(Runs[I]/2,0,0);
        TestTrue(TEXT("Smooth slab has real query collision"),Host.World->LineTraceSingleByChannel(Hit,Center+FVector(0,0,1000),Center-FVector(0,0,1000),ECC_Visibility));
        TestTrue(TEXT("Walking surface at half signed rise"),FMath::IsNearlyEqual(Hit.ImpactPoint.Z,Rise/2,.1f));
        TestEqual(TEXT("Single smooth blocker"),A->GetBlockers().Num(),1);
        FLHVisualRecipe Source=A->GetRecipe(), Restored;
        TArray<uint8> Bytes;
        FMemoryWriter Writer(Bytes);
        FObjectAndNameAsStringProxyArchive WriteArchive(Writer,false);
        FLHVisualRecipe::StaticStruct()->SerializeItem(WriteArchive,&Source,nullptr);
        FMemoryReader Reader(Bytes);
        FObjectAndNameAsStringProxyArchive ReadArchive(Reader,false);
        FLHVisualRecipe::StaticStruct()->SerializeItem(ReadArchive,&Restored,nullptr);
        TestEqual(TEXT("Serialized recipe retains every input"),Source.Fingerprint(),Restored.Fingerprint());
        TestTrue(TEXT("Serialized recipe rebuilds"),A->Build(Restored));
        TestTrue(TEXT("Rebuilt stairs within budgets"),LHVisual::ValidatePlacedSet({A}).Errors.IsEmpty());
    }
    return true;
}
#endif
