#include "Visual/Npc/LHNpcVisual.h"
#include "World/LHWorldMarkers.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Misc/PackageName.h"
#include "Misc/Crc.h"
#include "Visual/Npc/LHNpcRoster.inl"
ULHNpcVisualComponent::ULHNpcVisualComponent()
{
    PrimaryComponentTick.bCanEverTick=true;
    SetCanEverAffectNavigation(false);
    for(const TCHAR* Name:LHNpcVisualPrivate::Names)
    {
        const FString Path=FString(TEXT("/Game/Lighthaven/Art/Npc/DA_"))+Name;
        Appearances.Add(FPackageName::DoesPackageExist(Path)?LoadObject<ULHNpcAppearance>(nullptr,*Path):nullptr);
    }
}
bool ULHNpcVisualComponent::SharePlayerRig(USkeletalMesh* Mesh,USkeletalMesh* Player,UAnimSequence* Clip)
{
#if WITH_EDITOR
    if(!Mesh || !Player || !Player->GetSkeleton()) return false;
    const auto& A=Mesh->GetRefSkeleton(); const auto& B=Player->GetRefSkeleton();
    if(A.GetNum()!=B.GetNum()) { UE_LOG(LogTemp,Error,TEXT("NPC rig bone count %d versus player %d"),A.GetNum(),B.GetNum()); return false; }
    for(int32 I=0;I<A.GetNum();++I)
        if(A.GetBoneName(I)!=B.GetBoneName(I) || A.GetParentIndex(I)!=B.GetParentIndex(I) || !A.GetRefBonePose()[I].Equals(B.GetRefBonePose()[I],.005f)) { UE_LOG(LogTemp,Error,TEXT("NPC bind mismatch %d: %s versus %s"),I,*A.GetBoneName(I).ToString(),*B.GetBoneName(I).ToString()); return false; }
    Mesh->Modify(); Mesh->SetSkeleton(Player->GetSkeleton()); Mesh->MarkPackageDirty();
    if(Clip) { Clip->Modify(); Clip->SetSkeleton(Player->GetSkeleton()); Clip->MarkPackageDirty(); }
    return true;
#else
    return false;
#endif
}
void ULHNpcVisualComponent::Refresh()
{
    auto* Marker=Cast<ALHInteractableMarker>(GetOwner()); if(!Marker) return;
    for(USkeletalMeshComponent* Part:Parts) if(Part) Part->DestroyComponent(); Parts.Empty(); Active=nullptr;
    Marker->SetNpcFallbackVisible(true); SetComponentTickEnabled(false);
    for(ULHNpcAppearance* Row:Appearances) if(Row && Row->Identity==Marker->DefinitionId.Value) { Active=Row; break; }
    if(!Active || Active->Meshes.IsEmpty() || !Active->Idle || !Active->Gesture) { Active=nullptr; return; }
    int32 Slots=0;
    for(USkeletalMesh* Mesh:Active->Meshes)
    {
        if(!Mesh || Mesh->GetSkeleton()!=Active->Idle->GetSkeleton() || Mesh->GetSkeleton()!=Active->Gesture->GetSkeleton()) { Active=nullptr; return; }
        Slots+=Mesh->GetMaterials().Num();
    }
    if(Slots!=Active->Materials.Num() || Active->Materials.Contains(nullptr)) { Active=nullptr; return; }
    SetRelativeTransform(FTransform(FQuat::Identity,FVector::ZeroVector,FVector(Active->Stature)));
    int32 Slot=0;
    for(USkeletalMesh* Mesh:Active->Meshes)
    {
        auto* Part=NewObject<USkeletalMeshComponent>(Marker);
        Part->SetupAttachment(this); Part->SetSkeletalMeshAsset(Mesh);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision); Part->SetCanEverAffectNavigation(false);
        for(int32 I=0;I<Mesh->GetMaterials().Num();++I) Part->SetMaterial(I,Active->Materials[Slot++]);
        Part->RegisterComponent(); Part->PlayAnimation(Active->Idle,true); Parts.Add(Part);
    }
    Clock=static_cast<float>(FCrc::StrCrc32(*Marker->DefinitionId.Value.ToString())%1000)/1000.f*18.f;
    bGesture=false; SetComponentTickEnabled(true); Marker->SetNpcFallbackVisible(false);
}
void ULHNpcVisualComponent::BeginPlay() { Super::BeginPlay(); Refresh(); }
void ULHNpcVisualComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if(auto* Marker=Cast<ALHInteractableMarker>(GetOwner())) Marker->SetNpcFallbackVisible(true);
    Super::EndPlay(Reason);
}
void ULHNpcVisualComponent::TickComponent(float Delta,ELevelTick Tick,FActorComponentTickFunction* Function)
{
    Super::TickComponent(Delta,Tick,Function); if(!Active) return;
    // Prototype presentation: one two-second gesture per twenty-second cycle.
    Clock=FMath::Fmod(Clock+Delta,20.f); const bool Gesture=Clock>=18.f;
    if(Gesture!=bGesture) { bGesture=Gesture; for(USkeletalMeshComponent* Part:Parts) Part->PlayAnimation(Gesture?Active->Gesture:Active->Idle,true); }
}
