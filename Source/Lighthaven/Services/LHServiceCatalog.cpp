#include "Services/LHServiceCatalog.h"
namespace LHServices
{
FLHInteger PrototypeInteger(int64 V,const TCHAR* Note) { FLHInteger I; I.Value=V; I.Resolution=ELHValueResolution::Resolved; I.Provenance=LHAbilities::PrototypeNumber(V,Note).Provenance; return I; }
const TArray<FLHServiceOffer>& Catalog()
{
    static const TArray<FLHServiceOffer> Rows=[] {
        TArray<FLHServiceOffer> R;
        auto Add=[&](const TCHAR* N,const TCHAR* S,ELHServiceKind K,int G,const TCHAR* Area) {
            FLHServiceOffer O; O.Npc.Value=FName(*FString::Printf(TEXT("NPC.%s"),N)); O.Subject.Value=S;
            O.Id.Value=FName(*FString::Printf(TEXT("Offer.%s.%s"),N,S)); O.Kind=K; O.Area=Area;
            O.Gold=PrototypeInteger(G,TEXT("R03 missing training transaction units: gold per rank; replace after source/play review"));
            O.SkillPoints=PrototypeInteger(K==ELHServiceKind::TrainSkill?1:0,TEXT("R03 missing skill point conversion: one point per rank; vendor uses no points"));
            O.RankCap=PrototypeInteger(100,TEXT("R03 disputed authentic cap: Prototype slice cap 100"));
            if(K==ELHServiceKind::BuyItem) { O.Gold.Provenance.Status=ELHProvenanceStatus::Confirmed; O.Gold.Provenance.SourceUrl=G==50?TEXT("https://www.t4cbible.com/Items"):TEXT("https://www.t4cbible.com/traders"); O.Gold.Provenance.RetrievedDate=TEXT("2026-10-09"); O.Gold.Provenance.SourceBaseline=TEXT("R03 live Bible; version unstated"); O.Gold.Provenance.Notes=TEXT("Documentary buy price; not verified in play"); }
            if(K==ELHServiceKind::LearnSpell) { const auto* A=LHAbilities::Find(O.Subject); check(A); O.Gold=A->LearningGold; O.SkillPoints=A->LearningSkillPoints; }
            R.Add(O);
        };
        const TCHAR* Town=TEXT("TempleDistrict"), *Tower=TEXT("MageTower"), *B1=TEXT("B1");
        Add(TEXT("Kilhiam"),TEXT("Spell.Light"),ELHServiceKind::LearnSpell,0,Town);
        Add(TEXT("Moonrock"),TEXT("Spell.HealLight"),ELHServiceKind::LearnSpell,0,Town);
        Add(TEXT("Iraltok"),TEXT("Spell.FireDart"),ELHServiceKind::LearnSpell,0,Tower);
        Add(TEXT("Uranos"),TEXT("Spell.StoneShard"),ELHServiceKind::LearnSpell,0,Tower);
        Add(TEXT("Shovanis"),TEXT("Spell.DustDevil"),ELHServiceKind::LearnSpell,0,B1);
        for(const TCHAR* N:{TEXT("Murmuntag"),TEXT("Ortanalas")}) Add(N,TEXT("Skill.Attack"),ELHServiceKind::TrainSkill,10,Town);
        Add(TEXT("Kalastor"),TEXT("Skill.Dodge"),ELHServiceKind::TrainSkill,10,Town);
        for(const TCHAR* N:{TEXT("Kalastor"),TEXT("Ortanalas")}) Add(N,TEXT("Skill.Archery"),ELHServiceKind::TrainSkill,15,Town);
        Add(TEXT("Sigfried"),TEXT("Item.AshwoodFlatbow"),ELHServiceKind::BuyItem,29,Town);
        Add(TEXT("Sigfried"),TEXT("Item.WoodenArrows"),ELHServiceKind::BuyItem,100,Town);
        Add(TEXT("Fali"),TEXT("Item.PotionOfMana"),ELHServiceKind::BuyItem,50,Town);
        return R;
    }(); return Rows;
}
FLHInteger SellPrice(const FLHContentId& Id)
{
    if(Id.Value!=TEXT("Item.RustedDirk")) return {};
    auto I=PrototypeInteger(9,TEXT("Documentary sell price; not verified in play")); I.Provenance.Status=ELHProvenanceStatus::Confirmed;
    I.Provenance.SourceUrl=TEXT("https://www.t4cbible.com/Weapon"); I.Provenance.RetrievedDate=TEXT("2026-10-07"); I.Provenance.SourceBaseline=TEXT("Retained rules-ledger Bible evidence; version unstated"); return I;
}
}
