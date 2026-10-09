#include "Abilities/LHItemUse.h"
namespace LHItemUsePrivate
{
bool Same(const FLHEntityId& A,const FLHEntityId& B) { return A.RunId==B.RunId && A.InstanceId==B.InstanceId && A.Area.Content.Value==B.Area.Content.Value; }
bool Ready(const FLHNumber& N) { return N.Resolution==ELHValueResolution::Resolved && N.Provenance.Status!=ELHProvenanceStatus::Missing && N.Provenance.Status!=ELHProvenanceStatus::Disputed && FMath::IsFinite(N.Value) && N.Value>=0; }
}
namespace LHAbilities
{
ELHCommandReason ExecuteUseItem(FLHUseItemContext& C,const FLHUseItemRequest& R)
{
    if (!C.bAlive) return ELHCommandReason::InvalidLifeState;
    const bool Unset=!R.Target.RunId.IsValid() && !R.Target.InstanceId.IsValid() && R.Target.Area.Content.Value.IsNone();
    if (!Unset && !LHItemUsePrivate::Same(R.Target,C.Owner)) return ELHCommandReason::Ineligible;
    const int32 Index=C.Snapshot.Character.Inventory.IndexOfByPredicate([&](const auto& I){return LHItemUsePrivate::Same(I.Id,R.Item);});
    if(Index==INDEX_NONE) return ELHCommandReason::NotFound;
    const auto& Item=C.Snapshot.Character.Inventory[Index];
    const auto* Data=C.ItemLookup?C.ItemLookup(Item.Definition):nullptr;
    if(!Data || !Data->bConsumable) return ELHCommandReason::NotUsable;
    if(!LHItemUsePrivate::Ready(Data->Consumable.ManaRestore) || !LHItemUsePrivate::Ready(C.Snapshot.Character.CurrentMana)) return ELHCommandReason::UnresolvedRules;
    if(Item.Quantity.Resolution!=ELHValueResolution::Resolved || Item.Quantity.Value<=0 || !FMath::IsFinite(C.MaximumMana) || C.MaximumMana<C.Snapshot.Character.CurrentMana.Value) return ELHCommandReason::InvalidRequest;
    const double Current=C.Snapshot.Character.CurrentMana.Value;
    const double Next=FMath::Min(C.MaximumMana,Current+Data->Consumable.ManaRestore.Value);
    if(Next<=Current) return ELHCommandReason::NoEffect;
    if(C.Snapshot.Character.Equipment.ContainsByPredicate([&](const auto& E){return LHItemUsePrivate::Same(E.Item,R.Item);})) return ELHCommandReason::NotUsable;
    C.Snapshot.Character.CurrentMana.Value=Next;
    if(--C.Snapshot.Character.Inventory[Index].Quantity.Value==0) C.Snapshot.Character.Inventory.RemoveAt(Index);
    return ELHCommandReason::None;
}
}
