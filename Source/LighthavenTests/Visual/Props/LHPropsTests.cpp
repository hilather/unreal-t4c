#include "Misc/AutomationTest.h"
#include "Visual/Props/LHPropsCatalog.h"
#include "Data/Items/LHItemCatalog.h"
#include "Abilities/LHAbilityCatalog.h"
#include "Engine/Texture2D.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHPropsAssetsTest,"Lighthaven.Visual.Props.CatalogAssets",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHPropsAssetsTest::RunTest(const FString&)
{
    TestEqual(TEXT("All item definitions mapped"),ULHPropsCatalog::ItemIds().Num(),LHItemData::Catalog().Num());
    for(const auto& Row:LHItemData::Catalog())
    {
        TestTrue(*Row.Id.Value.ToString(),ULHPropsCatalog::ItemIds().Contains(Row.Id.Value));
        TestNotNull(*(Row.Id.Value.ToString()+TEXT(" mesh")),ULHPropsCatalog::Mesh(Row.Id.Value));
        TestNotNull(*(Row.Id.Value.ToString()+TEXT(" icon")),ULHPropsCatalog::Icon(Row.Id.Value));
    }
    TestEqual(TEXT("All ability definitions mapped"),ULHPropsCatalog::AbilityIds().Num(),LHAbilities::Catalog().Num());
    for(const auto& Row:LHAbilities::Catalog()) TestTrue(*Row.Id.Value.ToString(),ULHPropsCatalog::AbilityIds().Contains(Row.Id.Value));
    for(FName Id:ULHPropsCatalog::AbilityIds())
    {
        TestNotNull(*(Id.ToString()+TEXT(" icon")),ULHPropsCatalog::Icon(Id));
        const int32 Expected=(Id=="Spell.FireDart" || Id=="Spell.StoneShard" || Id=="Attack.Melee.Basic" || Id=="Attack.Ranged.Bow")?2:1;
        TestEqual(*(Id.ToString()+TEXT(" effects")),ULHPropsCatalog::Effects(Id).Num(),Expected);
    }
    for(FName Id:{FName("LootSack"),FName("CorpseLootBadge"),FName("ChestClosed"),FName("ChestOpen")}) TestNotNull(*Id.ToString(),ULHPropsCatalog::Mesh(Id));
    TestNull(TEXT("Unknown mesh falls back"),ULHPropsCatalog::Mesh("Unknown"));
    TestNull(TEXT("Unknown icon falls back"),ULHPropsCatalog::Icon("Unknown"));
    TestEqual(TEXT("Unknown effects fall back"),ULHPropsCatalog::Effects("Unknown").Num(),0);
    for(FName Id:ULHPropsCatalog::ItemIds()) if(auto* T=ULHPropsCatalog::Icon(Id))
    {
        TestTrue(TEXT("UI sRGB"),T->SRGB); TestTrue(TEXT("No streaming"),T->NeverStream);
        TestEqual(TEXT("UI group"),static_cast<int32>(T->LODGroup),static_cast<int32>(TEXTUREGROUP_UI)); TestFalse(TEXT("Retain alpha"),T->CompressionNoAlpha);
    }
    return true;
}
#endif
