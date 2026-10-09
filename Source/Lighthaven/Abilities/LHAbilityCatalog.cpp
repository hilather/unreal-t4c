#include "Abilities/LHAbilityCatalog.h"
#include "Abilities/LHAttributeSet.h"
namespace LHAbilityCatalogPrivate
{
FLHNumber Number(double V, bool Confirmed)
{
    auto N=LHAbilities::PrototypeNumber(V,TEXT("Missing runtime field: authored Wave4 tuning; replace after fixtures/balance review"));
    if (Confirmed) { N.Provenance.Status=ELHProvenanceStatus::Confirmed; N.Provenance.SourceUrl=TEXT("https://www.t4cbible.com/Spells"); N.Provenance.SourceBaseline=TEXT("R03 live Bible; version unstated"); N.Provenance.RetrievedDate=TEXT("2026-10-09"); N.Provenance.Notes=TEXT("R03 documentary evidence; not verified in play"); }
    return N;
}
FLHInteger Integer(int64 V, bool C) { FLHInteger I; I.Value=V; I.Resolution=ELHValueResolution::Resolved; I.Provenance=Number(V,C).Provenance; return I; }
bool Same(const FLHEntityId& A,const FLHEntityId& B) { return A.RunId==B.RunId && A.Area.Content.Value==B.Area.Content.Value && A.InstanceId==B.InstanceId; }
}
namespace LHAbilities
{
FLHNumber PrototypeNumber(double V,const TCHAR* Note) { FLHNumber N; N.Value=V; N.Resolution=ELHValueResolution::Resolved; N.Provenance.Status=ELHProvenanceStatus::Prototype; N.Provenance.SourceBaseline=TEXT("LH_Prototype_v1"); N.Provenance.RetrievedDate=TEXT("2026-10-09"); N.Provenance.Notes=Note; return N; }
const TArray<FLHAbilityCatalogRow>& Catalog()
{
    static const TArray<FLHAbilityCatalogRow> Rows=[] {
        using namespace LHAbilityCatalogPrivate;
        TArray<FLHAbilityCatalogRow> R;
        auto Add=[&](const TCHAR* Id,int L,int W,int I,int MP,int Points,int Gold,ELHAbilityEffect Effect,int Min,int Max) {
            FLHAbilityCatalogRow A; A.Id.Value=Id; const bool Spell=Effect!=ELHAbilityEffect::PhysicalDamage;
            A.Eligibility.MinimumLevel=Integer(L,Spell); auto& B=A.Eligibility.MinimumAttributes;
            B.Strength=Integer(0,false); B.Endurance=Integer(0,false); B.Agility=Integer(0,false); B.Wisdom=Integer(W,Spell); B.Intelligence=Integer(I,Spell);
            A.LearningSkillPoints=Integer(Points,Spell); A.LearningGold=Integer(Gold,Spell); A.ManaCost=Number(MP,Spell);
            A.RangeCm=Number(Spell?1200:200,false); A.CooldownSeconds=Number(1.5,false); A.ImpactSeconds=Number(0,false);
            A.MinimumMagnitude=Number(Min,false); A.MaximumMagnitude=Number(Max,false); A.DurationSeconds=Number(0,false);
            A.Effects=Effect; A.ClassificationTag=Spell?TEXT("LH.Ability.Spell"):TEXT("LH.Ability.Attack.Melee"); R.Add(A);
        };
        Add(TEXT("Attack.Melee.Basic"),0,0,0,0,0,0,ELHAbilityEffect::PhysicalDamage,0,0);
        Add(TEXT("Attack.Ranged.Bow"),0,0,0,0,0,0,ELHAbilityEffect::PhysicalDamage,0,0); R.Last().ClassificationTag=TEXT("LH.Ability.Attack.Ranged");
        Add(TEXT("Spell.Light"),2,15,18,10,5,233,ELHAbilityEffect::Light,0,0); R.Last().TargetPolicy=ELHAbilityTarget::Self; R.Last().DurationSeconds=Number(600,true); R.Last().DurationSeconds.Provenance.SourceUrl=TEXT("https://www.t4cbible.com/spelldesciprt");
        Add(TEXT("Spell.FireDart"),2,15,21,1,5,532,ELHAbilityEffect::SpellDamage,8,23);
        Add(TEXT("Spell.HealLight"),3,19,15,2,9,897,ELHAbilityEffect::Heal,10,10); R.Last().TargetPolicy=ELHAbilityTarget::Friendly;
        Add(TEXT("Spell.StoneShard"),4,20,17,2,6,1328,ELHAbilityEffect::SpellDamage,13,21);
        Add(TEXT("Spell.DustDevil"),6,21,21,2,7,2388,ELHAbilityEffect::SpellDamage,8,18);
        for (auto& A:R) if (A.Effects==ELHAbilityEffect::SpellDamage) {
            for(auto* N:{&A.MinimumMagnitude,&A.MaximumMagnitude}) {
                N->Provenance.SourceUrl=TEXT("https://web.archive.org/web/20020202213610/http://www.t4cbible.com:80/spells.html");
                N->Provenance.SourceBaseline=TEXT("Classic spell chart 1.20c, measured output; uniform raw magnitude interpretation is Prototype");
                N->Provenance.RetrievedDate=TEXT("2026-10-07"); N->Provenance.Notes=TEXT("Neutral power/resistance; replace after spell scaling and mitigation fixtures");
            }
        }
        return R;
    }(); return Rows;
}
const FLHAbilityCatalogRow* Find(const FLHContentId& Id) { return Catalog().FindByPredicate([&](const auto& A){return A.Id.Value==Id.Value;}); }
bool BuildAttackConfig(const FLHSaveSnapshot& S,const FLHContentId& Id,const FLHCombatItemLookup& Lookup,const LH::Rules::FCombatParameters& P,FLHBasicAttackConfig& Out,FString& Error)
{
    const auto* Row=Find(Id); if (!Row) {Error=TEXT("Unknown ability"); return false;}
    FLHBasicAttackConfig C; C.Ability=Id; C.Combat=P; C.Eligibility=Row->Eligibility; C.ManaCost=Row->ManaCost; C.RangeCm=Row->RangeCm; C.CooldownSeconds=Row->CooldownSeconds; C.ImpactSeconds=Row->ImpactSeconds;
    C.RequirementPolicy.Basis=LH::Rules::EAttributeBasis::Base; C.RequirementPolicy.BasisProvenance=PrototypeNumber(0,TEXT("Base requirements ledger proposal")).Provenance;
    C.RequirementPolicy.BowRequiresQuiver=LHAbilityCatalogPrivate::Integer(1,false);
    C.WeaponMinimum=Row->MinimumMagnitude; C.WeaponMaximum=Row->MaximumMagnitude;
    C.bSpell=Row->Effects!=ELHAbilityEffect::PhysicalDamage; C.bHeal=Row->Effects==ELHAbilityEffect::Heal; C.bLight=Row->Effects==ELHAbilityEffect::Light;
    C.LightDuration=Row->DurationSeconds.Value;
    C.bUseLearnedSkills=!C.bSpell;
    if(C.bSpell) C.Combat=LH::Rules::MakeStage1PrototypeCombat();
    if (!C.bSpell) {
        auto Equipped=[&](ELHEquipmentSlot Slot)->const FLHItemInstance* { const auto* B=S.Character.Equipment.FindByPredicate([&](const auto& E){return E.Slot==Slot;}); return B?S.Character.Inventory.FindByPredicate([&](const auto& I){return LHAbilityCatalogPrivate::Same(I.Id,B->Item);}):nullptr; };
        const auto* W=Equipped(ELHEquipmentSlot::MainHand); const auto* D=W && Lookup?Lookup(W->Definition):nullptr;
        if (!D || !D->bWeapon || D->Weapon.bBow!=(Id.Value==TEXT("Attack.Ranged.Bow"))) {Error=TEXT("Incompatible weapon"); return false;}
        C.WeaponMinimum=D->Weapon.MinimumDamage; C.WeaponMaximum=D->Weapon.MaximumDamage; C.RangeCm=D->Weapon.RangeCm; C.bBow=D->Weapon.bBow;
        if (C.bBow) { const auto* Q=Equipped(ELHEquipmentSlot::Quiver); const auto* QD=Q && Lookup?Lookup(Q->Definition):nullptr;
            if (!QD || !QD->bQuiver || !D->Weapon.CompatibleQuivers.ContainsByPredicate([&](const auto& X){return X.Value==Q->Definition.Value;})) {Error=TEXT("Compatible quiver required"); return false;}
            if (QD->Weapon.QuiverDamageBonus.Resolution!=ELHValueResolution::Resolved || QD->Weapon.QuiverDamageBonus.Provenance.Status==ELHProvenanceStatus::Missing || QD->Weapon.QuiverDamageBonus.Provenance.Status==ELHProvenanceStatus::Disputed || !FMath::IsFinite(QD->Weapon.QuiverDamageBonus.Value) || QD->Weapon.QuiverDamageBonus.Value<0) {Error=TEXT("Unresolved quiver bonus"); return false;} C.QuiverBonus=QD->Weapon.QuiverDamageBonus.Value;
        }
    }
    LH::Rules::FCombatInput Probe; Probe.WeaponMinimum=C.WeaponMinimum.Value; Probe.WeaponMaximum=C.WeaponMaximum.Value;
    if (!LH::Rules::ResolveCombat(C.Combat,Probe).Diagnostic.IsAccepted()) {Error=TEXT("Unresolved combat coefficients");return false;}
    Out=C; return true;
}
ELHCommandReason ExecuteUseAbility(FLHUseAbilityContext& C,const FLHUseAbilityRequest& R)
{
    const auto* A=Find(R.Ability); if (!A) return ELHCommandReason::NotFound;
    if (C.Source && C.Source->IsActionPending()) return ELHCommandReason::ActiveAction;
    if (!C.Source || !C.Target || !LHAbilityCatalogPrivate::Same(R.Target,C.TargetId)) return ELHCommandReason::NotFound;
    if (A->Effects!=ELHAbilityEffect::PhysicalDamage && !C.Snapshot.Character.LearnedSpells.ContainsByPredicate([&](const auto& S){return S.Value==R.Ability.Value;})) return ELHCommandReason::Ineligible;
    if (A->TargetPolicy==ELHAbilityTarget::Self && C.Source!=C.Target) return ELHCommandReason::Ineligible;
    if (A->TargetPolicy==ELHAbilityTarget::Friendly && C.Source!=C.Target && !C.bTargetFriendly) return ELHCommandReason::Ineligible;
    if (A->TargetPolicy==ELHAbilityTarget::Hostile && (C.Source==C.Target || C.bTargetFriendly)) return ELHCommandReason::Ineligible;
    FLHBasicAttackConfig Config; FString Error;
    if (!BuildAttackConfig(C.Snapshot,R.Ability,C.ItemLookup,C.Combat,Config,Error)) return Error.Contains(TEXT("Unresolved"))?ELHCommandReason::UnresolvedRules:ELHCommandReason::Ineligible;
    LH::Rules::FRequirementInput I; const auto& B=C.Snapshot.Character.BaseAttributes;
    for (const auto* N : {&B.Strength,&B.Endurance,&B.Agility,&B.Intelligence,&B.Wisdom}) if (N->Resolution!=ELHValueResolution::Resolved) return ELHCommandReason::UnresolvedRules;
    if (C.Snapshot.Character.EarnedLevel.Resolution!=ELHValueResolution::Resolved) return ELHCommandReason::UnresolvedRules;
    for (const auto& Skill:C.Snapshot.Character.LearnedSkills) if(Skill.TrainedValue.Resolution!=ELHValueResolution::Resolved || Skill.TrainedValue.Value<0) return ELHCommandReason::UnresolvedRules;
    I.Base={B.Strength.Value,B.Endurance.Value,B.Agility.Value,B.Intelligence.Value,B.Wisdom.Value}; I.Effective=I.Base; I.Level=C.Snapshot.Character.EarnedLevel.Value; I.Skills=C.Snapshot.Character.LearnedSkills; I.Spells=C.Snapshot.Character.LearnedSpells; I.bBow=Config.bBow; I.bCompatibleQuiverEquipped=Config.bBow;
    C.Source->SetStableEntity(C.SourceId); C.Target->SetStableEntity(C.TargetId); C.Source->ConfigureAttack(Config,I); return C.Source->RequestBasicAttack(C.Target);
}
}
