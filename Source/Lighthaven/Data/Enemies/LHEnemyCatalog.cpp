#include "Data/Enemies/LHEnemyCatalog.h"
#include "Data/LHDataProvenance.h"
namespace LHEnemyCatalogPrivate
{
constexpr const TCHAR* Monster=TEXT("https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html");
constexpr const TCHAR* Drops=TEXT("https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html");
constexpr const TCHAR* RuntimeMissing=TEXT("R-03 missing attack rating/speed and arena/script; rules-ledger Enemy runtime interpretation: authored runtime adapter, replace after source research and G4 balance review.");
FLHEnemyCatalogRow Make(const TCHAR* Name,int Level,int HP,int X1,int X50,int X100,int Min,int Max,int GoldMin,int GoldMax,int R,int HH,bool DamageDisputed,bool LootDisputed,std::initializer_list<const TCHAR*> Items)
{
    using namespace LHData; FLHEnemyCatalogRow E;
    E.Id.Value=FName(FString(TEXT("Enemy."))+Name); E.PresentationId.Value=FName(FString(TEXT("Presentation.Enemy."))+Name);
    E.Level=Integer(Level,Classic(TEXT("Level"),Monster)); E.Health=Number(HP,Classic(TEXT("Health"),Monster));
    const int XP[]={X1,X50,X100}; for (int I=0;I<3;++I) E.ExperienceColumns[I]=Integer(XP[I],Classic(*FString::Printf(TEXT("XP.Column%d"),I),Monster));
    auto Damage=Classic(TEXT("Damage"),Monster,TEXT("Selected classic min/max; live conflicts for Rat/Bat/Slime/Undead Bat retained in definition status. Runtime explicitly selects classic, per Matt/R-03."));
    E.MinimumDamage=Number(Min,Damage); E.MaximumDamage=Number(Max,Damage);
    if (DamageDisputed) { E.MinimumDamage.Provenance.Status=ELHProvenanceStatus::Disputed; E.MaximumDamage.Provenance.Status=ELHProvenanceStatus::Disputed; }
    E.GoldMinimum=Integer(GoldMin,Live(TEXT("Gold.Minimum"),TEXT("https://www.t4cbible.com/monster1")));
    E.GoldMaximum=Integer(GoldMax,Live(TEXT("Gold.Maximum"),TEXT("https://www.t4cbible.com/monster1")));
    const bool Boss=E.Id.Value==TEXT("Enemy.Balork");
    if (Boss) E.GoldMinimum=E.GoldMaximum=PInteger(5,TEXT("Gold"),TEXT("R-03 Balork gold missing; minimal fallback5; replace when researched."));
    E.LootMembership=Classic(TEXT("Loot.Membership"),Drops,TEXT("Classic listed items selected. Live-only Heal Pot/Atrocity Iron Key excluded; rarity conflicts retained; dash means no listed item, not proof of no loot."));
    if (LootDisputed) E.LootMembership.Status=ELHProvenanceStatus::Disputed;
    auto& S=E.Runtime; S.ContentId=E.Id; S.PresentationId=E.PresentationId; S.bBoss=Boss; S.MaxHealth=E.Health;
    S.CapsuleRadiusCm=PNumber(R,TEXT("Capsule.Radius"),TEXT("A-01 species capsule2026-10-08 adopted W4-02; authored geometry, replace only after clearance review."));
    S.CapsuleHalfHeightCm=PNumber(HH,TEXT("Capsule.HalfHeight"),TEXT("A-01 species capsule2026-10-08 adopted W4-02; authored geometry, replace only after clearance review."));
    S.Accuracy=PNumber(10,TEXT("Accuracy"),RuntimeMissing);
    S.Avoidance=PNumber(10,TEXT("Avoidance"),RuntimeMissing);
    S.Armor=PNumber(0,TEXT("Armor"),RuntimeMissing); S.Resistance=PNumber(0,TEXT("Resistance"),RuntimeMissing);
    S.DamageBonus=PNumber(0,TEXT("DamageBonus"),RuntimeMissing); S.MaxMana=PNumber(0,TEXT("MaxMana"),RuntimeMissing);
    auto& A=S.Attack; A.Combat=LH::Rules::MakeStage1PrototypeCombat();
    for (auto* V:{&A.Combat.HitBase,&A.Combat.AccuracyScale,&A.Combat.AvoidanceScale,&A.Combat.MinimumChance,&A.Combat.MaximumChance,&A.Combat.ArmorScale,&A.Combat.ResistanceScale,&A.Combat.MinimumDamage,&A.Combat.DamageQuantum})
        V->Provenance=Prototype(TEXT("Combat.Formula"),TEXT("rules-ledger Physical resolution missing formula; R-03 damage distribution missing: shared Stage1 prototype ratio/flat armor model, replace when sourced."));
    A.RequirementPolicy.Basis=LH::Rules::EAttributeBasis::Base; A.RequirementPolicy.BasisProvenance=Prototype(TEXT("Requirements.Basis"),RuntimeMissing);
    A.RequirementPolicy.BowRequiresQuiver=PInteger(1,TEXT("Requirements.Quiver"),RuntimeMissing);
    auto& B=A.Eligibility.MinimumAttributes; B.Strength=B.Endurance=B.Agility=B.Intelligence=B.Wisdom=PInteger(0,TEXT("Requirements.Attributes"),RuntimeMissing);
    A.Eligibility.MinimumLevel=PInteger(0,TEXT("Requirements.Level"),RuntimeMissing); S.Requirements.Level=Level;
    A.ManaCost=PNumber(0,TEXT("Attack.ManaCost"),RuntimeMissing);
    A.CooldownSeconds=PNumber(2,TEXT("Attack.Cooldown"),TEXT("R-03 attack speed missing: one attack/2000ms; replace after source/play review."));
    A.ImpactSeconds=PNumber(1,TEXT("Attack.Impact"),RuntimeMissing);
    A.RangeCm=PNumber(Boss?350:200,TEXT("Attack.Range"),RuntimeMissing);
    A.WeaponMinimum=Number(Min,Damage); A.WeaponMaximum=Number(Max,Damage);
    const double Values[]={800,1600,180,3,3,.2,50,1000};
    checkf(LHAI::AllowedParameterKeys().Num()==UE_ARRAY_COUNT(Values),TEXT("AI allowlist changed: review W4-02 tuning"));
    for (int I=0;I<LHAI::AllowedParameterKeys().Num();++I)
    {
        FLHMechanicalField F; F.Key=LHAI::AllowedParameterKeys()[I].Name;
        F.Value=PNumber(Values[I],*F.Key.ToString(),I==7?TEXT("R-03 respawn safety distance missing:1000cm; W4-04 same policy; replace after safety review."):RuntimeMissing);
        E.AIParameters.Add(F);
    }
    FString Error; checkf(LHAI::ApplyParameters(E.AIParameters,S,Error),TEXT("%s"),*Error);
    auto& W=E.Reward; W.bBoss=Boss;
    W.Experience=PInteger(X1,TEXT("Reward.Experience"),TEXT("R-03 unconditional XP semantics missing: first printed XP column selected; replace after semantics research."));
    W.GoldMin=W.GoldMax=PInteger((GoldMin+GoldMax)/2,TEXT("Reward.Gold"),TEXT("R-03 gold distribution missing: guaranteed floor-midpoint of Bible range; replace after distribution research."));
    if (Boss) W.GoldMin=W.GoldMax=E.GoldMinimum;
    W.ItemDropChance=PNumber(.1,TEXT("Reward.ItemDropChance"),TEXT("R-03 drop odds missing:10% overall item event, uniform one listed item; replace when sourced."));
    W.RespawnSeconds=Boss?Number(900,Live(TEXT("RespawnSeconds"),TEXT("https://www.t4cbible.com/monster1"))):PNumber(120,TEXT("RespawnSeconds"),TEXT("R-03 ordinary respawn missing:120s; replace after source/play review."));
    W.SafetyDistanceCm=S.SpawnSafetyDistanceCm;
    for (auto Item:Items)
    {
        FLHLootEntry L; L.Item.Value=FName(FString(TEXT("Item."))+Item); L.Provenance=E.LootMembership;
        L.MinimumQuantity=L.MaximumQuantity=PInteger(1,TEXT("Loot.Quantity"),TEXT("R-03 quantity missing:one listed item; replace when sourced."));
        L.Weight=PNumber(1,TEXT("Loot.Weight"),TEXT("R-03 simultaneous drops/distribution missing:uniform one classic item; replace when sourced.")); W.Loot.Add(L);
    }
    return E;
}
}
const TArray<FLHEnemyCatalogRow>& LHEnemyData::Catalog()
{
    using LHEnemyCatalogPrivate::Make;
    static const TArray<FLHEnemyCatalogRow> Rows={
        Make(TEXT("BrownRat"),1,27,45,42,42,4,5,1,5,25,25,true,false,{TEXT("Torch")}),
        Make(TEXT("Bat"),1,27,42,37,32,4,5,1,5,25,75,true,false,{TEXT("Torch"),TEXT("LightHeal")}),
        Make(TEXT("DungeonBat"),2,41,75,69,68,3,7,3,11,30,80,false,false,{}),
        Make(TEXT("GreenSlime"),2,41,74,69,68,4,7,3,11,40,40,true,false,{}),
        Make(TEXT("GiantBat"),3,55,107,94,93,4,8,5,16,45,100,false,false,{}),
        Make(TEXT("UndeadBat"),3,55,94,93,93,4,7,5,16,30,85,true,false,{TEXT("DecayingBatWings")}),
        Make(TEXT("GiantSpider"),4,69,146,124,122,4,10,7,22,55,55,false,false,{TEXT("Torch"),TEXT("LightHeal")}),
        Make(TEXT("Goblin"),5,84,192,161,157,5,12,8,27,35,70,false,true,{TEXT("GoblinLeatherArmor"),TEXT("LightHeal"),TEXT("IronRing")}),
        Make(TEXT("GoblinWarrior"),12,199,706,527,499,10,23,21,66,40,75,false,true,{TEXT("GoblinBlade"),TEXT("IronKey"),TEXT("LightHeal")}),
        Make(TEXT("Atrocity"),5,84,231,161,157,5,12,8,27,65,100,false,true,{TEXT("LightHeal")}),
        Make(TEXT("Balork"),15,508,2025,1553,1452,13,29,5,5,100,155,false,false,{TEXT("FlowingBlackRobe"),TEXT("LightHeal")})};
    return Rows;
}
const FLHEnemyCatalogRow* LHEnemyData::Find(const FLHContentId& Id)
{ return Catalog().FindByPredicate([&](const auto& R){return R.Id.Value==Id.Value;}); }
