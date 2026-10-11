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
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCreatureImportedAssets,"Lighthaven.Visual.Monsters.CreatureImportedAssets",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHCreatureImportedAssets::RunTest(const FString&)
{
    const TCHAR* Kinds[]={TEXT("rat"),TEXT("bat"),TEXT("slime"),TEXT("goblin"),TEXT("giant_spider"),TEXT("balork"),TEXT("goblin_warrior"),TEXT("atrocity"),TEXT("dungeon_bat"),TEXT("giant_bat"),TEXT("undead_bat")};
    const FVector Maximum[]={FVector(90,28,25),FVector(45,80,145),FVector(90,90,40),FVector(110,85,185),FVector(150,170,65),FVector(200,440,310),FVector(120,105,210),FVector(160,180,190),FVector(55,100,155),FVector(85,170,190),FVector(60,110,165)};
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
        // Use the same authored full-rig rest bounds consumed by build-art.sh.
        // Missing or malformed evidence is a failure, never a skipped bounds check.
        const FString ValidationPath=FPaths::ProjectDir()/TEXT("artsource/creatures/output")/(FString(Kinds[I])+TEXT("-validation.json"));
        FString Validation;
        if(TestTrue(*ValidationPath,FFileHelper::LoadFileToString(Validation,*ValidationPath)))
        {
            // The token reader is header-only: no unowned module dependency change.
            const auto Reader=TJsonReaderFactory<>::Create(Validation);
            EJsonNotation Token;
            int32 Depth=0,RestDepth=-1;
            bool HasMinimum=false,HasMaximum=false,Valid=true;
            FVector Minimum,RestMaximum;
            while(Reader->ReadNext(Token))
            {
                if(Token==EJsonNotation::Error) { Valid=false; break; }
                if(Token==EJsonNotation::ObjectStart)
                {
                    ++Depth;
                    if(Depth==2 && Reader->GetIdentifier()==TEXT("rest_bounds")) RestDepth=Depth;
                }
                else if(Token==EJsonNotation::ObjectEnd)
                {
                    if(Depth==RestDepth) RestDepth=-1;
                    --Depth;
                }
                else if(Token==EJsonNotation::ArrayStart)
                {
                    const FString Field=Reader->GetIdentifier();
                    if(Depth!=RestDepth || (Field!=TEXT("min_cm") && Field!=TEXT("max_cm")))
                    {
                        if(!Reader->SkipArray()) { Valid=false; break; }
                        continue;
                    }
                    FVector Value;
                    int32 Count=0;
                    while(Reader->ReadNext(Token) && Token!=EJsonNotation::ArrayEnd)
                    {
                        if(Token!=EJsonNotation::Number || Count>=3 || !FMath::IsFinite(Reader->GetValueAsNumber())) { Valid=false; break; }
                        Value[Count++]=Reader->GetValueAsNumber();
                    }
                    if(!Valid || Token!=EJsonNotation::ArrayEnd || Count!=3) { Valid=false; break; }
                    if(Field==TEXT("min_cm")) { Minimum=Value; HasMinimum=true; }
                    else { RestMaximum=Value; HasMaximum=true; }
                }
            }
            if(TestTrue(TEXT("Valid build JSON with finite three-coordinate rest bounds"),Valid && Depth==0 && HasMinimum && HasMaximum))
            {
                TestTrue(TEXT("Exact authored rest minimum including socket weapon"),RestBox.Min.Equals(Minimum,.5));
                TestTrue(TEXT("Exact authored rest maximum including socket weapon"),RestBox.Max.Equals(RestMaximum,.5));
            }
        }
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
