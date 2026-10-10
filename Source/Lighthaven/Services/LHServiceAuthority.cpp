#include "Services/LHServiceAuthority.h"
namespace LHServiceAuthorityPrivate
{
// Modernized authorizes authored policy only, never historical costs or balances.
bool Ready(const FLHInteger& I, bool AuthoredPolicy=false)
{
    return I.Resolution==ELHValueResolution::Resolved && I.Value>=0 &&
        (I.Provenance.Status==ELHProvenanceStatus::Confirmed ||
         I.Provenance.Status==ELHProvenanceStatus::VerifiedT4C ||
         I.Provenance.Status==ELHProvenanceStatus::Prototype ||
         (AuthoredPolicy && I.Provenance.Status==ELHProvenanceStatus::Modernized));
}
bool Same(const FLHEntityId& A,const FLHEntityId& B) { return A.RunId==B.RunId && A.Area.Content.Value==B.Area.Content.Value && A.InstanceId==B.InstanceId; }
const FLHServiceOffer* Find(const FLHServiceContext& C,ELHServiceKind K,FName Subject,bool ById=false) { return LHServices::Catalog().FindByPredicate([&](const auto& O){return O.Npc.Value==C.Npc.Value && O.Kind==K && (ById?O.Id.Value:O.Subject.Value)==Subject;}); }
ELHCommandReason Common(const FLHServiceContext& C,const FLHSaveSnapshot& S,const FLHEntityId& Target)
{
    if(!C.Profile) return ELHCommandReason::UnresolvedRules;
    if(!Target.InstanceId.IsValid() || !Target.RunId.IsValid() || Target.Area.Content.Value.IsNone() || !Same(Target,C.Entity) || Target.RunId!=S.World.RunId) return ELHCommandReason::NotFound;
    if(C.bBusy) return ELHCommandReason::Busy;
    if(S.Character.CurrentHealth.Resolution!=ELHValueResolution::Resolved || !FMath::IsFinite(S.Character.CurrentHealth.Value)) return ELHCommandReason::UnresolvedRules;
    if(S.Character.CurrentHealth.Value<=0) return ELHCommandReason::InvalidLifeState;
    // R03 NPC distance missing: Prototype 250 cm, same spatial convention as portal interactions.
    if(!FMath::IsFinite(C.DistanceCm) || C.DistanceCm<0 || C.DistanceCm>250) return ELHCommandReason::OutOfRange;
    if(!C.bLineOfSight) return ELHCommandReason::Obstructed;
    return ELHCommandReason::None;
}
ELHCommandReason Cost(FLHSaveSnapshot& S,const FLHServiceOffer& O,int64 N)
{
    if(!Ready(O.Gold) || !Ready(O.SkillPoints) || !Ready(S.Character.Gold) || !Ready(S.Character.UnspentSkillPoints)) return ELHCommandReason::UnresolvedRules;
    if(N<=0 || (O.Gold.Value && N>MAX_int64/O.Gold.Value) || (O.SkillPoints.Value && N>MAX_int64/O.SkillPoints.Value)) return ELHCommandReason::InvalidRequest;
    const int64 G=N*O.Gold.Value,P=N*O.SkillPoints.Value;
    if(S.Character.Gold.Value<G) return ELHCommandReason::InsufficientGold;
    if(S.Character.UnspentSkillPoints.Value<P) return ELHCommandReason::InsufficientPoints;
    S.Character.Gold.Value-=G; S.Character.UnspentSkillPoints.Value-=P; return ELHCommandReason::None;
}
ELHCommandReason Eligible(const FLHSaveSnapshot& S,const FLHEligibility& E)
{
    const auto& B=S.Character.BaseAttributes;
    for(const auto* V:{&B.Strength,&B.Endurance,&B.Agility,&B.Intelligence,&B.Wisdom,&S.Character.EarnedLevel}) if(!Ready(*V)) return ELHCommandReason::UnresolvedRules;
    LH::Rules::FRequirementInput I; I.Base={B.Strength.Value,B.Endurance.Value,B.Agility.Value,B.Intelligence.Value,B.Wisdom.Value}; I.Effective=I.Base; I.Level=S.Character.EarnedLevel.Value; I.Skills=S.Character.LearnedSkills; I.Spells=S.Character.LearnedSpells;
    LH::Rules::FRequirementPolicy P; P.Basis=LH::Rules::EAttributeBasis::Base; P.BasisProvenance=LHServices::PrototypeInteger(0,TEXT("R03 missing requirement basis: use base attributes")).Provenance;
    P.BowRequiresQuiver=LHServices::PrototypeInteger(0,TEXT("Learning does not require equipment"));
    auto R=LH::Rules::CheckRequirements(P,E,I); if(!R.Diagnostic.IsAccepted()) return R.Diagnostic.Reason==LH::Rules::EReason::Ineligible?ELHCommandReason::Ineligible:ELHCommandReason::UnresolvedRules;
    return R.Value?ELHCommandReason::None:ELHCommandReason::Ineligible;
}
ELHCommandReason Commit(const FLHServiceContext& C,FLHSaveSnapshot& S,FLHSaveSnapshot&& Next)
{
    FLHCharacterAuthority A; if(!A.Initialize(*C.Profile,Next.Session.RequestEpoch,0)) return ELHCommandReason::UnresolvedRules;
    auto R=A.Import(Next); if(R==ELHCommandReason::None) S=MoveTemp(Next); return R;
}
}
namespace LHServices
{
ELHCommandReason Execute(const FLHServiceContext& C,FLHSaveSnapshot& S,const FLHTrainSkillRequest& R)
{
    using namespace LHServiceAuthorityPrivate;
    auto Reason=Common(C,S,R.Trainer); if(Reason!=ELHCommandReason::None) return Reason;
    if(!Ready(R.Points) || R.Points.Value<=0) return ELHCommandReason::InvalidRequest;
    const auto* O=Find(C,ELHServiceKind::TrainSkill,R.Skill.Value); if(!O) return ELHCommandReason::NotFound;
    auto N=S; auto* Skill=N.Character.LearnedSkills.FindByPredicate([&](const auto& K){return K.Skill.Value==R.Skill.Value;});
    if(!Ready(O->RankCap) || (Skill && !Ready(Skill->TrainedValue))) return ELHCommandReason::UnresolvedRules;
    const int64 Rank=Skill?Skill->TrainedValue.Value:0;
    if(Rank>O->RankCap.Value || R.Points.Value>O->RankCap.Value-Rank) return ELHCommandReason::Ineligible;
    Reason=Cost(N,*O,R.Points.Value); if(Reason!=ELHCommandReason::None) return Reason;
    if(Skill) Skill->TrainedValue.Value+=R.Points.Value;
    else { if(N.Character.LearnedSkills.Num()>=256) return ELHCommandReason::InvalidRequest; FLHLearnedSkill K; K.Skill=R.Skill; K.TrainedValue=PrototypeInteger(R.Points.Value,TEXT("Prototype one point per rank")); N.Character.LearnedSkills.Add(K); }
    return Commit(C,S,MoveTemp(N));
}
ELHCommandReason Execute(const FLHServiceContext& C,FLHSaveSnapshot& S,const FLHLearnSpellRequest& R)
{
    using namespace LHServiceAuthorityPrivate;
    auto Reason=Common(C,S,R.Trainer); if(Reason!=ELHCommandReason::None) return Reason;
    const auto* O=Find(C,ELHServiceKind::LearnSpell,R.Spell.Value); if(!O) return ELHCommandReason::NotFound;
    const auto* A=C.AbilityLookup?C.AbilityLookup(R.Spell):nullptr; if(!A) return ELHCommandReason::UnresolvedRules;
    if(S.Character.LearnedSpells.ContainsByPredicate([&](const auto& K){return K.Value==R.Spell.Value;})) return ELHCommandReason::Ineligible;
    Reason=Eligible(S,A->Eligibility); if(Reason!=ELHCommandReason::None) return Reason;
    auto N=S; FLHServiceOffer Price=*O; Price.Gold=A->LearningGold; Price.SkillPoints=A->LearningSkillPoints;
    Reason=Cost(N,Price,1); if(Reason!=ELHCommandReason::None) return Reason;
    if(N.Character.LearnedSpells.Num()>=256) return ELHCommandReason::InvalidRequest;
    N.Character.LearnedSpells.Add(R.Spell); return Commit(C,S,MoveTemp(N));
}
ELHCommandReason Execute(const FLHServiceContext& C,FLHSaveSnapshot& S,const FLHBuyItemRequest& R)
{
    using namespace LHServiceAuthorityPrivate;
    auto Reason=Common(C,S,R.Vendor); if(Reason!=ELHCommandReason::None) return Reason;
    if(!Ready(R.Quantity) || R.Quantity.Value<=0) return ELHCommandReason::InvalidRequest;
    const auto* O=Find(C,ELHServiceKind::BuyItem,R.Offer.Value,true); if(!O) return ELHCommandReason::NotFound;
    const auto* D=C.ItemLookup?C.ItemLookup(O->Subject):C.Profile->Items.FindByPredicate([&](const auto& I){return I.Id.Value==O->Subject.Value;});
    if(!D || D->Id.Value!=O->Subject.Value || !Ready(D->StackLimit,true) || D->StackLimit.Value<=0 || !Ready(C.Profile->InventorySlots,true)) return ELHCommandReason::UnresolvedRules;
    const int64 Slots=R.Quantity.Value/D->StackLimit.Value+(R.Quantity.Value%D->StackLimit.Value!=0);
    if(Slots>512-S.Character.Inventory.Num() || Slots>C.Profile->InventorySlots.Value-S.Character.Inventory.Num()) return ELHCommandReason::InventoryFull;
    auto N=S; Reason=Cost(N,*O,R.Quantity.Value); if(Reason!=ELHCommandReason::None) return Reason;
    int64 Remaining=R.Quantity.Value;
    while(Remaining>0) { FLHItemInstance I; I.Definition=O->Subject; I.Id.RunId=S.World.RunId; I.Id.Area=C.Entity.Area; I.Id.InstanceId=FGuid::NewGuid(); I.Quantity=PrototypeInteger(FMath::Min(Remaining,D->StackLimit.Value),TEXT("Purchase quantity; unlimited stock owner policy")); Remaining-=I.Quantity.Value; N.Character.Inventory.Add(I); }
    return Commit(C,S,MoveTemp(N));
}
ELHCommandReason Execute(const FLHServiceContext& C,FLHSaveSnapshot& S,const FLHSellItemRequest& R)
{
    using namespace LHServiceAuthorityPrivate;
    auto Reason=Common(C,S,R.Vendor); if(Reason!=ELHCommandReason::None) return Reason;
    // Only actual stocked vendors accept sales; trainers and unresolved Rolph do not.
    if(!Catalog().ContainsByPredicate([&](const auto& O){return O.Npc.Value==C.Npc.Value && O.Kind==ELHServiceKind::BuyItem;})) return ELHCommandReason::NotFound;
    if(!Ready(R.Quantity) || R.Quantity.Value<=0) return ELHCommandReason::InvalidRequest;
    const int32 Index=S.Character.Inventory.IndexOfByPredicate([&](const auto& I){return Same(I.Id,R.Item);}); if(Index==INDEX_NONE) return ELHCommandReason::NotFound;
    if(S.Character.Equipment.ContainsByPredicate([&](const auto& E){return Same(E.Item,R.Item);})) return ELHCommandReason::InvalidEquipment;
    const auto& Item=S.Character.Inventory[Index]; auto Price=SellPrice(Item.Definition);
    if(!Ready(Price) || !Ready(Item.Quantity) || !Ready(S.Character.Gold)) return ELHCommandReason::UnresolvedRules;
    if(R.Quantity.Value>Item.Quantity.Value || (Price.Value && R.Quantity.Value>MAX_int64/Price.Value)) return ELHCommandReason::InvalidRequest;
    const int64 Gain=Price.Value*R.Quantity.Value; if(S.Character.Gold.Value>MAX_int64-Gain) return ELHCommandReason::InvalidRequest;
    auto N=S; N.Character.Gold.Value+=Gain; N.Character.Inventory[Index].Quantity.Value-=R.Quantity.Value;
    if(!N.Character.Inventory[Index].Quantity.Value) N.Character.Inventory.RemoveAt(Index);
    return Commit(C,S,MoveTemp(N));
}
TArray<FLHServiceOfferView> Offers(const FLHContentId& Npc,const FLHSaveSnapshot& S)
{
    using namespace LHServiceAuthorityPrivate;
    TArray<FLHServiceOfferView> Views;
    for(const auto& O:Catalog()) if(O.Npc.Value==Npc.Value) {
        FLHServiceOfferView V; V.Offer=O; auto N=S; V.Reason=Cost(N,O,1);
        if(V.Reason==ELHCommandReason::None && O.Kind==ELHServiceKind::LearnSpell) { const auto* A=LHAbilities::Find(O.Subject); V.Reason=A?Eligible(S,A->Eligibility):ELHCommandReason::UnresolvedRules; if(S.Character.LearnedSpells.ContainsByPredicate([&](const auto& K){return K.Value==O.Subject.Value;})) V.Reason=ELHCommandReason::Ineligible; }
        if(V.Reason==ELHCommandReason::None && O.Kind==ELHServiceKind::TrainSkill) { const auto* K=S.Character.LearnedSkills.FindByPredicate([&](const auto& X){return X.Skill.Value==O.Subject.Value;}); if(K && (!Ready(K->TrainedValue) || K->TrainedValue.Value>=O.RankCap.Value)) V.Reason=ELHCommandReason::Ineligible; }
        if(S.Character.CurrentHealth.Value<=0) V.Reason=ELHCommandReason::InvalidLifeState;
        V.bEligible=V.Reason==ELHCommandReason::None; Views.Add(V);
    }
    return Views;
}
}
