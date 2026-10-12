#include "Visual/Props/LHPropsCatalog.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
ULHPropsCatalog::ULHPropsCatalog()
{
    static ConstructorHelpers::FObjectFinderOptional<ULHPropsAssets> Imported(TEXT("/Game/Lighthaven/Art/Props/DA_Props.DA_Props"));
    Assets=Imported.Get();
}
UTexture2D* ULHPropsCatalog::Icon(FName Id)
{
    const auto* A=GetDefault<ULHPropsCatalog>()->Assets.Get();
    if(A) if(const auto* V=A->Icons.Find(Id)) return V->Get();
    return nullptr;
}
UStaticMesh* ULHPropsCatalog::Mesh(FName Id)
{
    const auto* A=GetDefault<ULHPropsCatalog>()->Assets.Get();
    if(A) if(const auto* V=A->Meshes.Find(Id)) return V->Get();
    return nullptr;
}
const TArray<FName>& ULHPropsCatalog::ItemIds()
{
    static const TArray<FName> Ids={"Item.RustedDirk","Item.AshwoodFlatbow","Item.WoodenArrows","Item.Torch","Item.PotionOfMana","Item.ClothVest","Item.ClothPants","Item.LightHeal","Item.DecayingBatWings","Item.GoblinLeatherArmor","Item.IronRing","Item.GoblinBlade","Item.IronKey","Item.FlowingBlackRobe"}; return Ids;
}
const TArray<FName>& ULHPropsCatalog::AbilityIds()
{
    static const TArray<FName> Ids={"Spell.Light","Spell.FireDart","Spell.HealLight","Spell.StoneShard","Spell.DustDevil","Attack.Melee.Basic","Attack.Ranged.Bow"}; return Ids;
}
TArray<UStaticMesh*> ULHPropsCatalog::Effects(FName Id)
{
    static const TMap<FName,TArray<FName>> Names={{"Spell.Light",{"Light"}},{"Spell.FireDart",{"FireDart","FireImpact"}},{"Spell.HealLight",{"HealLight"}},{"Spell.StoneShard",{"StoneShard","StoneShatter"}},{"Spell.DustDevil",{"DustDevil"}},{"Attack.Melee.Basic",{"MeleeSpark","SlashArc"}},{"Attack.Ranged.Bow",{"ArrowFlight","ImpactPuff"}}};
    TArray<UStaticMesh*> Out; if(const auto* Keys=Names.Find(Id)) for(FName Key:*Keys) if(auto* M=Mesh(Key)) Out.Add(M); return Out;
}
