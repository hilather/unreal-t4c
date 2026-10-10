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
#include "Animation/AnimSequence.h"
#include "Misc/PackageName.h"
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
        const auto Bounds=Mesh->GetBounds();
        const FVector Size=Bounds.BoxExtent*2;
        TestTrue(TEXT("Finite positive centimetre bounds"),!Size.ContainsNaN() && Size.GetMin()>0);
        TestTrue(TEXT("Within source rest ruler"),Size.X<=Maximum[I].X+1 && Size.Y<=Maximum[I].Y+1 && Size.Z<=Maximum[I].Z+1);
        for(const TCHAR* Action:{TEXT("idle"),TEXT("move"),TEXT("attack"),TEXT("hit"),TEXT("death")})
        {
            const FString ClipPath=Root+TEXT("A_")+Action;
            auto* Clip=FPackageName::DoesPackageExist(ClipPath)?LoadObject<UAnimSequence>(nullptr,*ClipPath):nullptr;
            if(TestNotNull(*ClipPath,Clip)) TestTrue(TEXT("Compatible skeleton"),Clip->GetSkeleton()==Mesh->GetSkeleton());
        }
    }
    return true;
}
