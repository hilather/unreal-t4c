#include "Data/Items/LHItemCatalog.h"
#include "Data/LHDataProvenance.h"
namespace LHItemCatalogPrivate
{
FLHItemCatalogRow Item(const TCHAR* Name)
{
    using namespace LHData; FLHItemCatalogRow R; R.Id.Value=FName(FString(TEXT("Item."))+Name); R.Character.Id=R.Id;
    R.Membership=Classic(TEXT("Item.Membership"),TEXT("https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html"));
    R.Character.StackLimit=Integer(1,Provenance(TEXT("StackLimit"),TEXT(""),TEXT("2026-10-09"),ELHProvenanceStatus::Modernized,TEXT("Slice inventory policy: individual instances, no inferred historical stacking.")));
    R.CarryPolicy=Policy(TEXT("CarryPolicy"),TEXT("UnresearchedLootExemptFromCapacity"),Provenance(TEXT("CarryPolicy"),TEXT(""),TEXT("2026-10-09"),ELHProvenanceStatus::Modernized,TEXT("Explicit adapter permits unknown loot to be carried while encumbrance remains unresolved. No equip/sell authorization. Replace exemption when researched.")));
    R.Character.Weight=Number(0,R.CarryPolicy.Provenance);
    return R;
}
void ZeroModifiers(FLHCharacterItemDefinition& C)
{
    using namespace LHData;
    auto P=Provenance(TEXT("Modifiers"),TEXT(""),TEXT("2026-10-09"),ELHProvenanceStatus::Modernized,TEXT("Slice adapter: only explicitly registered effects apply; no inferred historical bonus."));
    auto& M=C.Modifier; M.Attributes.Strength=M.Attributes.Endurance=M.Attributes.Agility=M.Attributes.Intelligence=M.Attributes.Wisdom=Integer(0,P);
    M.Health=M.Mana=M.Accuracy=M.Avoidance=M.DamageBonus=M.Armor=M.Capacity=Number(0,P);
}
void Requirements(FLHCharacterItemDefinition& C,const TCHAR* Page)
{
    using namespace LHData; auto P=FString(Page).EndsWith(TEXT("/Armor"))?Live(TEXT("Eligibility"),Page):Classic(TEXT("Eligibility"),Page);
    auto& B=C.Eligibility.MinimumAttributes; B.Strength=B.Endurance=B.Agility=B.Intelligence=B.Wisdom=Integer(0,P);
    C.Eligibility.MinimumLevel=Integer(0,Provenance(TEXT("Eligibility.MinimumLevel"),TEXT(""),TEXT("2026-10-09"),ELHProvenanceStatus::Modernized,TEXT("Slice no additional level restriction; source has no listed level requirement.")));
}
}
const TArray<FLHItemCatalogRow>& LHItemData::Catalog()
{
    static const TArray<FLHItemCatalogRow> Rows=[]
    {
        using namespace LHData; using namespace LHItemCatalogPrivate; TArray<FLHItemCatalogRow> Out;
        for (const TCHAR* Name:{TEXT("RustedDirk"),TEXT("AshwoodFlatbow"),TEXT("WoodenArrows"),TEXT("PotionOfMana"),TEXT("ClothVest"),TEXT("ClothPants"),TEXT("Torch"),TEXT("LightHeal"),TEXT("DecayingBatWings"),TEXT("GoblinLeatherArmor"),TEXT("IronRing"),TEXT("GoblinBlade"),TEXT("IronKey"),TEXT("FlowingBlackRobe")})
        {
            auto R=Item(Name); auto& C=R.Character; const FString N(Name);
            if (N==TEXT("RustedDirk") || N==TEXT("AshwoodFlatbow") || N==TEXT("WoodenArrows"))
            {
                const bool Dirk=N==TEXT("RustedDirk"), Quiver=N==TEXT("WoodenArrows");
                const TCHAR* Page=Dirk?TEXT("https://www.t4cbible.com/Weapon"):TEXT("https://www.t4cbible.com/traders");
                R.Membership=Live(TEXT("Item.Membership"),Page); C.Weight=Number(Dirk?3:Quiver?3:7,Classic(TEXT("Encumbrance"),Page));
                R.BuyGold=Integer(Quiver?100:29,Classic(TEXT("BuyGold"),Page));
                if (Dirk) R.SellGold=Integer(9,Classic(TEXT("SellGold"),Page));
                C.Slot=Quiver?ELHEquipmentSlot::Quiver:ELHEquipmentSlot::MainHand; C.bBow=!Dirk&&!Quiver;
                R.bEquipable=true; Requirements(C,Page); ZeroModifiers(C);
                auto& W=R.Combat.Weapon; R.Combat.bQuiver=Quiver; R.Combat.bWeapon=!Quiver; W.bBow=C.bBow;
                if (Quiver) W.QuiverDamageBonus=Number(1,Classic(TEXT("QuiverDamageBonus"),Page));
                else
                {
                    W.MinimumDamage=Number(1,Classic(TEXT("Damage.Minimum"),Page)); W.MaximumDamage=Number(Dirk?4:3,Classic(TEXT("Damage.Maximum"),Page));
                    W.RangeCm=PNumber(Dirk?200:900,TEXT("Weapon.Range"),TEXT("rules-ledger physical range missing; authored Stage1 reach; replace after source/play review."));
                    if (C.bBow) { FLHContentId Q; Q.Value=TEXT("Item.WoodenArrows"); C.CompatibleQuivers.Add(Q); W.CompatibleQuivers=C.CompatibleQuivers; }
                }
            }
            else if (N==TEXT("PotionOfMana"))
            {
                R.Membership=Live(TEXT("Item.Membership"),TEXT("https://www.t4cbible.com/Potions"));
                C.Weight=Number(2,Live(TEXT("Encumbrance"),TEXT("https://www.t4cbible.com/Potions")));
                R.BuyGold=Integer(50,Live(TEXT("BuyGold"),TEXT("https://www.t4cbible.com/Items")));
                R.Combat.bConsumable=true; R.Combat.Consumable.ManaRestore=Number(25,Live(TEXT("ManaRestore"),TEXT("https://www.t4cbible.com/Potions")));
            }
            else if (N==TEXT("ClothVest") || N==TEXT("ClothPants"))
            {
                const TCHAR* Page=TEXT("https://www.t4cbible.com/Armor"); R.Membership=Live(TEXT("Item.Membership"),Page);
                C.Weight=Number(3,Live(TEXT("Encumbrance"),Page)); C.Slot=N==TEXT("ClothVest")?ELHEquipmentSlot::Torso:ELHEquipmentSlot::Legs;
                R.bEquipable=true; Requirements(C,Page); ZeroModifiers(C); C.Modifier.Armor=Number(0,Live(TEXT("Armor"),Page)); C.Modifier.Avoidance=Number(0,Live(TEXT("Dodge"),Page));
            }
            if (N==TEXT("RustedDirk") || N==TEXT("AshwoodFlatbow") || N==TEXT("WoodenArrows") || N==TEXT("PotionOfMana") || N==TEXT("ClothVest") || N==TEXT("ClothPants"))
            { R.Encumbrance=C.Weight; R.CarryPolicy=Policy(TEXT("CarryPolicy"),TEXT("SourceEncumbrance"),C.Weight.Provenance); }
            Out.Add(MoveTemp(R));
        } return Out;
    }(); return Rows;
}
const FLHItemCatalogRow* LHItemData::Find(const FLHContentId& Id)
{ return Catalog().FindByPredicate([&](const auto& R){return R.Id.Value==Id.Value;}); }
const TArray<FLHStarterGrant>& LHItemData::StartingKit()
{
    static const TArray<FLHStarterGrant> Grants=[]
    {
        TArray<FLHStarterGrant> Out;
        for (const TCHAR* Name:{TEXT("Item.RustedDirk"),TEXT("Item.ClothVest"),TEXT("Item.ClothPants"),TEXT("Item.Torch")})
        { FLHStarterGrant G; G.Item.Value=Name; G.Quantity=LHData::PInteger(G.Item.Value==TEXT("Item.Torch")?3:1,TEXT("Starter.Quantity"),TEXT("R-03 starting kit missing:Dirk1 vest1 pants1 torches3; replace when sourced.")); Out.Add(G); }
        return Out;
    }(); return Grants;
}
