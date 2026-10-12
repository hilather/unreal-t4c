#include "Visual/Player/LHPlayerEquipmentVisual.h"
#include "Visual/Player/LHPlayerVisual.h"
#include "Visual/Props/LHPropsCatalog.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
FName ULHPlayerEquipmentVisual::Equipped(const FLHCharacterRecord& C,ELHEquipmentSlot Slot)
{
    for(const auto& B:C.Equipment) if(B.Slot==Slot)
        for(const auto& I:C.Inventory) if(I.Id.RunId==B.Item.RunId && I.Id.Area.Content.Value==B.Item.Area.Content.Value && I.Id.InstanceId==B.Item.InstanceId) return I.Definition.Value;
    return NAME_None;
}
void ULHPlayerEquipmentVisual::Present(ULHPlayerVisualComponent* V,const FLHCharacterRecord& C,ULHCombatComponent* Combat,bool Enabled,bool Drawing)
{
    if(!V) return;
    const FName Main=Equipped(C,ELHEquipmentSlot::MainHand), Quiver=Equipped(C,ELHEquipmentSlot::Quiver);
    const bool Bow=Main==TEXT("Item.AshwoodFlatbow"), Light=Combat && Combat->GetLightRemainingSeconds()>0;
    const FName Ids[]={Main,Quiver,Bow?FName(TEXT("ArrowFlight")):NAME_None,Light && !Bow?FName(TEXT("Item.Torch")):NAME_None};
    const FName Sockets[]={Bow?FName(TEXT("Socket.Bow.L")):FName(TEXT("Socket.Weapon.R")),TEXT("Socket.Quiver.Back"),TEXT("Socket.Arrow"),TEXT("Socket.Bow.L")};
    while(Meshes.Num()<4)
    {
        auto* M=NewObject<UStaticMeshComponent>(GetOwner()); M->SetMobility(EComponentMobility::Movable);
        M->SetCollisionEnabled(ECollisionEnabled::NoCollision); M->SetGenerateOverlapEvents(false); M->SetCanEverAffectNavigation(false); M->RegisterComponent(); Meshes.Add(M);
    }
    for(int32 I=0;I<4;++I)
    {
        auto* M=Meshes[I].Get(); M->SetStaticMesh(ULHPropsCatalog::Mesh(Ids[I]));
        USceneComponent* Parent=V->BodyMesh()?static_cast<USceneComponent*>(V->BodyMesh()):V->Socket(I==2?FName(TEXT("BowArrow")):Sockets[I]);
        if(Parent) M->AttachToComponent(Parent,FAttachmentTransformRules::SnapToTargetNotIncludingScale,V->BodyMesh()?Sockets[I]:NAME_None);
        M->SetVisibility(Enabled && M->GetStaticMesh() && Parent && (I!=2 || Drawing));
    }
    for(const auto& P:V->Pieces()) if(P)
    {
        const FString Id=P->GetRecipe().Id.ToString();
        if(Id.EndsWith(TEXT("Weapon")) || Id.Contains(TEXT("BowString"))) P->SetActorHiddenInGame(!Enabled || Meshes[0]->GetStaticMesh()!=nullptr);
        else if(Id.EndsWith(TEXT("Quiver"))) P->SetActorHiddenInGame(!Enabled || Meshes[1]->GetStaticMesh()!=nullptr);
        else if(Id.EndsWith(TEXT("BowArrow"))) P->SetActorHiddenInGame(!Enabled || !Drawing || Meshes[2]->GetStaticMesh()!=nullptr);
    }
}

void ULHPlayerEquipmentVisual::SetEnabled(bool Enabled)
{ if(!Enabled) for(const auto& M:Meshes) if(M) M->SetVisibility(false); }
void ULHPlayerEquipmentVisual::OnComponentDestroyed(bool DestroyingHierarchy)
{ for(const auto& M:Meshes) if(M) M->DestroyComponent(); Meshes.Reset(); Super::OnComponentDestroyed(DestroyingHierarchy); }
