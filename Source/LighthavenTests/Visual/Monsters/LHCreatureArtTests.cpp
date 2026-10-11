#include "Misc/AutomationTest.h"
#include "Visual/Monsters/LHMonsterVisual.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCreatureArtCatalog,"Lighthaven.Visual.Monsters.CreatureArtCatalog",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHCreatureArtCatalog::RunTest(const FString&)
{
    const TCHAR* Ids[]={TEXT("BrownRat"),TEXT("Bat"),TEXT("GreenSlime"),TEXT("Goblin"),TEXT("GiantSpider"),TEXT("Balork"),TEXT("GoblinWarrior"),TEXT("Atrocity"),TEXT("DungeonBat"),TEXT("GiantBat"),TEXT("UndeadBat")};
    TSet<FString> Unique;
    for(const TCHAR* Id:Ids)
    {
        const FName Definition(*(FString(TEXT("Enemy."))+Id));
        const FString Art=ULHMonsterVisual::ArtId(Definition);
        TestFalse(TEXT("Roster has import folder"),Art.IsEmpty());
        TestTrue(TEXT("Roster retains fallback recipe"),ULHMonsterVisual::Known(Definition));
        Unique.Add(Art);
    }
    TestEqual(TEXT("Eleven distinct bindings"),Unique.Num(),11);
    TestTrue(TEXT("Unknown has no imported binding"),ULHMonsterVisual::ArtId(TEXT("Enemy.Unregistered")).IsEmpty());
    return true;
}

#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "StaticMeshResources.h"
#include "Animation/AnimSequence.h"
#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCreatureImportedAssets,"Lighthaven.Visual.Monsters.CreatureImportedAssets",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHCreatureImportedAssets::RunTest(const FString&)
{
    const TCHAR* Kinds[]={TEXT("rat"),TEXT("bat"),TEXT("slime"),TEXT("goblin"),TEXT("giant_spider"),TEXT("balork"),TEXT("goblin_warrior"),TEXT("atrocity"),TEXT("dungeon_bat"),TEXT("giant_bat"),TEXT("undead_bat")};
    const FVector Maximum[]={FVector(90,28,25),FVector(45,80,145),FVector(90,90,40),FVector(110,85,185),FVector(150,170,65),FVector(200,440,310),FVector(120,105,210),FVector(160,180,190),FVector(55,100,155),FVector(85,170,190),FVector(60,110,165)};
    // Authored W5 creature validation JSON rest bounds (cm); presentation evidence, not mechanics.
    const FVector Minimum[]={FVector(-66.009,-12.219,0.191),FVector(-20.775,-36.408,91.867),FVector(-42.802,-41.075,0.0),FVector(-26.4,-40.596,1.848),FVector(-60.774,-81.669,0.257),FVector(-96.245,-212.159,0.1),FVector(-26.4,-43.0,1.848),FVector(-51.07,-79.984,0.759),FVector(-23.89,-45.256,94.792),FVector(-35.692,-79.4,106.643),FVector(-27.168,-50.351,98.504)};
    const FVector RestMaximum[]={FVector(20.004,12.201,24.745),FVector(15.1,36.41,122.594),FVector(44.099,44.669,21.114),FVector(43.108,40.596,184.0),FVector(79.072,81.671,62.9),FVector(60.201,212.159,302.571),FVector(43.108,43.0,209.0),FVector(79.298,79.988,189.056),FVector(16.912,45.256,128.147),FVector(25.67,79.405,156.481),FVector(19.479,50.355,133.781)};
    for(int32 I=0;I<11;++I)
    {
        const FString Root=FString(TEXT("/Game/Lighthaven/Art/Creatures/"))+Kinds[I]+TEXT("/");
        const FString Path=Root+TEXT("SK_")+Kinds[I];
        if(!FPackageName::DoesPackageExist(Path)) { AddError(TEXT("Missing imported creature: ")+Path); continue; }
        auto* Mesh=LoadObject<USkeletalMesh>(nullptr,*Path);
        if(!TestNotNull(TEXT("Skeletal mesh"),Mesh)) continue;
        TestEqual(TEXT("One authored LOD"),Mesh->GetLODNum(),1);
        TestTrue(TEXT("Retained source for cold-cache rebuild"),Mesh->HasMeshDescription(0));
        for(const TCHAR* Channel:{TEXT("base"),TEXT("normal"),TEXT("orm")})
        {
            auto* Texture=LoadObject<UTexture2D>(nullptr,*(Root+TEXT("T_")+Channel));
            if(TestNotNull(TEXT("Creature texture"),Texture))
            {
                const int32 Limit=I==5 && FString(Channel)==TEXT("base")?1024:512;
                TestTrue(TEXT("Gameplay-distance texture budget"),Texture->Source.GetSizeX()<=Limit && Texture->Source.GetSizeY()<=Limit);
                TestTrue(TEXT("Atlas has no alpha compression overhead"),Texture->CompressionNoAlpha);
                TestEqual(TEXT("Texture color space"),Texture->SRGB,FString(Channel)==TEXT("base"));
                TestEqual(TEXT("Texture compression"),Texture->CompressionSettings,
                    FString(Channel)==TEXT("normal")?TC_Normalmap:FString(Channel)==TEXT("orm")?TC_Masks:TC_Default);
            }
        }
        TestTrue(TEXT("Body has material slots"),Mesh->GetMaterials().Num()>0);
        for(const auto& Slot:Mesh->GetMaterials())
            TestTrue(TEXT("Persisted creature material binding"),Slot.MaterialInterface &&
                Slot.MaterialInterface->GetPathName()==Root+TEXT("MI_Creature.MI_Creature"));
        const auto Bounds=Mesh->GetBounds();
        const FVector Size=Bounds.BoxExtent*2;
        TestTrue(TEXT("Finite positive centimetre bounds"),!Size.ContainsNaN() && Size.GetMin()>0);
        TestTrue(TEXT("Within source rest ruler"),Size.X<=Maximum[I].X+1 && Size.Y<=Maximum[I].Y+1 && Size.Z<=Maximum[I].Z+1);
        FBox RestBox=Bounds.GetBox();
        if(I==3 || I==5 || I==6)
        {
            auto* Weapon=LoadObject<UStaticMesh>(nullptr,*(Root+TEXT("SM_Weapon")));
            const auto* Socket=Mesh->FindSocket(TEXT("WeaponSocket"));
            if(TestNotNull(TEXT("Separate rigid weapon"),Weapon) && TestNotNull(TEXT("Named weapon socket"),Socket))
            {
                TestEqual(TEXT("Socket targets authored bone"),Socket->BoneName,I==5?FName(TEXT("Weapon_Main")):FName(TEXT("Weapon_R")));
                const FMatrix Pose=Mesh->GetComposedRefPoseMatrix(Socket->BoneName);
                const auto* Render=Weapon->GetRenderData();
                if(TestNotNull(TEXT("Weapon render data"),Render) && TestTrue(TEXT("Weapon LOD"),Render->LODResources.Num()>0))
                {
                    const auto& Vertices=Render->LODResources[0].VertexBuffers.PositionVertexBuffer;
                    for(uint32 V=0;V<Vertices.GetNumVertices();++V)
                        RestBox+=FVector(Pose.TransformPosition(FVector(Vertices.VertexPosition(V))));
                }
            }
        }
        TestTrue(TEXT("Exact authored rest minimum including socket weapon"),RestBox.Min.Equals(Minimum[I],.5));
        TestTrue(TEXT("Exact authored rest maximum including socket weapon"),RestBox.Max.Equals(RestMaximum[I],.5));
        for(const TCHAR* Action:{TEXT("idle"),TEXT("move"),TEXT("attack"),TEXT("hit"),TEXT("death")})
        {
            const FString ClipPath=Root+TEXT("A_")+Action;
            auto* Clip=FPackageName::DoesPackageExist(ClipPath)?LoadObject<UAnimSequence>(nullptr,*ClipPath):nullptr;
            if(TestNotNull(*ClipPath,Clip)) TestTrue(TEXT("Compatible skeleton"),Clip->GetSkeleton()==Mesh->GetSkeleton());
        }
    }
    TArray<FString> Packages;
    IFileManager::Get().FindFilesRecursive(Packages,*(FPaths::ProjectContentDir()/TEXT("Lighthaven/Art/Creatures")),TEXT("*.uasset"),true,false);
    int64 Bytes=0;
    for(const FString& Package:Packages) Bytes+=IFileManager::Get().FileSize(*Package);
    TestTrue(TEXT("Imported creature packages fit 25,000,000 bytes"),Bytes>0 && Bytes<=25000000);
    return true;
}
