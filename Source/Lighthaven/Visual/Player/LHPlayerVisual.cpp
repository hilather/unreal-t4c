#include "Visual/Player/LHPlayerVisual.h"
#include "Data/Items/LHItemCatalog.h"
#include "Engine/World.h"
#include "Misc/Crc.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Animation/AnimSequence.h"
#include "Misc/PackageName.h"
#include "Materials/MaterialInterface.h"
#if WITH_EDITOR
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshLODModel.h"
#endif

namespace LHPlayerVisualPrivate
{
bool Has(const FLHCharacterRecord& C,const TCHAR* Id) { return C.AppearanceIds.ContainsByPredicate([&](const FLHContentId& V){return V.Value==Id;}); }
FLinearColor Color(const TCHAR* Hex) { return FLinearColor(FColor::FromHex(Hex)); }
FLHVisualBox Box(FVector Center,FVector Size,int32 Surface=0,FRotator Rotation=FRotator::ZeroRotator)
{ FLHVisualBox B; B.Center=Center; B.Size=Size; B.Surface=Surface; B.Rotation=Rotation; return B; }
}
TArray<FLHPlayerPart> LHPlayerVisual::Build(const FLHCharacterRecord& C)
{
    using namespace LHPlayerVisualPrivate;
    TArray<FLHPlayerPart> Parts;
    const bool Narrow=Has(C,TEXT("Presentation.Player.Body.B")), Tied=Has(C,TEXT("Presentation.Player.Hair.Tied"));
    const float Width=Narrow?38:46;
    const FLinearColor Skin=Color(Has(C,TEXT("Presentation.Player.Skin.DeepWarm"))?TEXT("51362C"):Has(C,TEXT("Presentation.Player.Skin.MediumWarm"))?TEXT("8B5A40"):TEXT("BD8E72"));
    const FLinearColor Cloth=Color(Narrow?TEXT("6D7974"):TEXT("85806A")), Leather=Color(TEXT("43352A")), Metal=Color(TEXT("979A96"));
    auto Add=[&](FName Name,FName Parent,FVector Position,FLinearColor A,FLinearColor B,TArray<FLHVisualBox> Boxes)
    { FLHPlayerPart P; P.Name=Name; P.Parent=Parent; P.Transform=FTransform(Position); P.Recipe.Id=FName(FString(TEXT("Presentation.Player."))+Name.ToString()); P.Recipe.Colors[0]=A; P.Recipe.Colors[1]=B; P.Recipe.Geometry=MoveTemp(Boxes); Parts.Add(MoveTemp(P)); };
    Add(TEXT("Torso"),NAME_None,{0,0,96},Cloth,Leather,{Box({0,0,15},{26,Width,48}),Box({0,0,-7},{28,Width+2,8},1)});
    Add(TEXT("Head"),TEXT("Torso"),{0,0,55},Skin,Color(TEXT("30251D")),{Box({0,0,9},{Narrow?23.f:27.f,25,29}),Box({1,0,25},{28,29,8},1),Box({14,0,9},{5,7,7}),Box({-13,0,Tied?10.f:19.f},Tied?FVector(13,16,16):FVector(6,27,9),1)});
    for(int32 Side:{-1,1})
    {
        const FName Arm=Side<0?TEXT("Arm.L"):TEXT("Arm.R"), Leg=Side<0?TEXT("Leg.L"):TEXT("Leg.R");
        Add(Arm,TEXT("Torso"),{0,Side*(Width/2+5),32},Cloth,Skin,{Box({0,0,-17},{14,14,34}),Box({0,0,-39},{11,11,16},1)});
        Add(Side<0?TEXT("Socket.Bow.L"):TEXT("Socket.Weapon.R"),Arm,{0,0,-49},Skin,Skin,{Box({0,0,0},{12,12,12})});
        Add(Leg,NAME_None,{0,Side*11.f,81},Cloth,Leather,{Box({0,0,-29},{18,18,58}),Box({0,0,-65},{18,19,18},1),Box({7,0,-76},{32,20,10},1)});
    }
    Add(TEXT("Socket.Quiver.Back"),TEXT("Torso"),{-21,12,18},Leather,Leather,{});
    for(const auto& Binding:C.Equipment)
    {
        const auto* Item=C.Inventory.FindByPredicate([&](const FLHItemInstance& I){ return I.Id.RunId==Binding.Item.RunId && I.Id.Area.Content.Value==Binding.Item.Area.Content.Value && I.Id.InstanceId==Binding.Item.InstanceId; });
        if(!Item) continue;
        const auto* Row=LHItemData::Find(Item->Definition);
        if(!Row || !Row->bEquipable) continue;
        if(Binding.Slot==ELHEquipmentSlot::Quiver)
            Add(TEXT("Quiver"),TEXT("Socket.Quiver.Back"),{0,0,0},Leather,Color(TEXT("B2A58A")),{Box({0,0,0},{18,18,52}),Box({0,0,31},{5,5,25},1),Box({0,6,31},{5,5,25},1)});
        if(Binding.Slot!=ELHEquipmentSlot::MainHand) continue;
        if(Row->Character.bBow)
        {
            Add(TEXT("Weapon"),TEXT("Socket.Bow.L"),{0,0,0},Leather,Leather,{Box({5,0,0},{9,9,45}),Box({0,0,44},{9,9,49},0,FRotator(-15,0,0)),Box({0,0,-44},{9,9,49},0,FRotator(15,0,0))});
            for(int32 Sign:{-1,1}) Add(Sign<0?TEXT("BowString.Lower"):TEXT("BowString.Upper"),TEXT("Weapon"),{-15,0,Sign*31.5f},Color(TEXT("B2A58A")),Leather,{Box({0,0,0},{2,2,63})});
            Add(TEXT("BowArrow"),TEXT("Weapon"),{-15,0,0},Color(TEXT("B2A58A")),Metal,{Box({30,0,0},{60,3,3}),Box({62,0,0},{6,6,6},1)});
        }
        else
        {
            const FString Id=Item->Definition.Value.ToString();
            const bool Staff=Id.Contains(TEXT("Staff")), Dirk=Id.Contains(TEXT("Dirk"));
            const float Length=Staff?145:Dirk?33:72;
            Add(TEXT("Weapon"),TEXT("Socket.Weapon.R"),{0,0,0},Staff?Leather:Metal,Leather,{Box({Length/2+8,0,0},{Length,Staff?9.f:6.f,Staff?9.f:12.f}),Box({0,0,0},{16,9,9},1),Box({9,0,0},{5,9,24},1)});
        }
    }
    return Parts;
}
uint32 LHPlayerVisual::Fingerprint(const TArray<FLHPlayerPart>& Parts)
{
    uint32 Hash=0;
    for(const auto& P:Parts) { Hash=HashCombine(Hash,P.Recipe.Fingerprint()); Hash=HashCombine(Hash,FCrc::StrCrc32(*P.Parent.ToString())); Hash=HashCombine(Hash,GetTypeHash(P.Transform.GetLocation())); }
    return Hash;
}
bool LHPlayerVisual::ResolveAppearance(const FLHCharacterRecord& C,int32& Body,int32& Hair,int32& Skin)
{
    using namespace LHPlayerVisualPrivate;
    Body=Has(C,TEXT("Presentation.Player.Body.B"))?1:0;
    Hair=Has(C,TEXT("Presentation.Player.Hair.Tied"))?1:0;
    Skin=Has(C,TEXT("Presentation.Player.Skin.DeepWarm"))?2:Has(C,TEXT("Presentation.Player.Skin.MediumWarm"))?1:0;
    TSet<FName> Seen; int32 Bodies=0,Hairs=0,Skins=0,Faces=0,Outfits=0;
    for(const auto& Id:C.AppearanceIds)
    {
        if(Seen.Contains(Id.Value)) return false; Seen.Add(Id.Value);
        const FString V=Id.Value.ToString();
        if(V==TEXT("Presentation.Player.Body.A") || V==TEXT("Presentation.Player.Body.B")) ++Bodies;
        else if(V==TEXT("Presentation.Player.Hair.Cropped") || V==TEXT("Presentation.Player.Hair.Tied")) ++Hairs;
        else if(V==TEXT("Presentation.Player.Skin.LightWarm") || V==TEXT("Presentation.Player.Skin.MediumWarm") || V==TEXT("Presentation.Player.Skin.DeepWarm")) ++Skins;
        else if(V==TEXT("Presentation.Player.Outfit.StarterLinen")) ++Outfits;
        else if(V==TEXT("Presentation.Player.Face.A") || V==TEXT("Presentation.Player.Face.B"))
        { ++Faces; if(V!=(Body?TEXT("Presentation.Player.Face.B"):TEXT("Presentation.Player.Face.A"))) return false; }
        else return false;
    }
    return Bodies==1 && Hairs==1 && Skins==1 && Outfits==1 && Faces<=1;
}
FString LHPlayerVisual::AssetRoot(const FLHCharacterRecord& C)
{ return FString(TEXT("/Game/Lighthaven/Art/Player/"))+(LHPlayerVisualPrivate::Has(C,TEXT("Presentation.Player.Body.B"))?TEXT("player_b/"):TEXT("player_a/")); }
FName LHPlayerVisual::Action(ELHPlayerPose P)
{
    static const FName Names[]={TEXT("idle"),TEXT("move"),TEXT("run"),TEXT("melee"),TEXT("bow"),TEXT("cast"),TEXT("hit"),TEXT("death")};
    return Names[static_cast<uint8>(P)];
}
FString LHPlayerVisual::MeshAssetPath(const FLHCharacterRecord& C,FName Part)
{
    const FString Kind=LHPlayerVisualPrivate::Has(C,TEXT("Presentation.Player.Body.B"))?TEXT("player_b"):TEXT("player_a");
    const FString Label=Part.ToString();
    return AssetRoot(C)+Label+TEXT("/")+Kind+TEXT("-")+Label+TEXT("/SkeletalMeshes/SK_")+Label;
}
FString LHPlayerVisual::ActionAssetPath(const FLHCharacterRecord& C,ELHPlayerPose Pose)
{ return MeshAssetPath(C,TEXT("body"))+Action(Pose).ToString(); }
ULHPlayerVisualComponent::ULHPlayerVisualComponent()
{
    SetRelativeLocation({0,0,-90}); SetCanEverAffectNavigation(false);
    // CDO references keep all appearances/actions reachable by the cooker.
    for(int32 B=0;B<2;++B)
    {
        FLHCharacterRecord C; if(B) { FLHContentId Id; Id.Value=TEXT("Presentation.Player.Body.B"); C.AppearanceIds.Add(Id); }
        const FString Root=LHPlayerVisual::AssetRoot(C);
        auto Mesh=[&](FName Part) { const FString Path=LHPlayerVisual::MeshAssetPath(C,Part); return FPackageName::DoesPackageExist(Path)?LoadObject<USkeletalMesh>(nullptr,*Path):nullptr; };
        BodyAssets.Add(Mesh(TEXT("body")));
        for(const TCHAR* Hair:{TEXT("Cropped"),TEXT("Tied")}) HairAssets.Add(Mesh(Hair));
        for(int32 A=0;A<8;++A) { const FString Path=LHPlayerVisual::ActionAssetPath(C,static_cast<ELHPlayerPose>(A)); ActionAssets.Add(FPackageName::DoesPackageExist(Path)?LoadObject<UAnimSequence>(nullptr,*Path):nullptr); }
        for(const TCHAR* Skin:{TEXT("LightWarm"),TEXT("MediumWarm"),TEXT("DeepWarm")}) { const FString Path=Root+TEXT("MI_")+Skin; SkinAssets.Add(FPackageName::DoesPackageExist(Path)?LoadObject<UMaterialInterface>(nullptr,*Path):nullptr); }
    }
}
bool ULHPlayerVisualComponent::ConfigureImportedMesh(USkeletalMesh* Mesh,UMaterialInterface* Clothing,UMaterialInterface* Skin)
{
#if WITH_EDITOR
    if(!Mesh || !Clothing || !Skin || Mesh->GetMaterials().IsEmpty()) return false;
    Mesh->Modify(); auto Slots=Mesh->GetMaterials();
    for(auto& Slot:Slots)
    {
        const bool bSkin=Slot.ImportedMaterialSlotName.ToString().Contains(TEXT("SkinTint"));
        Slot.MaterialInterface=bSkin?Skin:Clothing;
        Slot.MaterialSlotName=bSkin?FName(TEXT("SkinTint")):FName(TEXT("ClothingAtlas"));
    }
    Mesh->SetMaterials(Slots);
    for(const FName Name:{FName(TEXT("Socket.Weapon.R")),FName(TEXT("Socket.Bow.L")),FName(TEXT("Socket.Arrow")),FName(TEXT("Socket.Quiver.Back"))})
    {
        FName Bone=Name;
        if(Mesh->GetRefSkeleton().FindBoneIndex(Bone)==INDEX_NONE) Bone=FName(Name.ToString().Replace(TEXT("."),TEXT("_")));
        if(Mesh->GetRefSkeleton().FindBoneIndex(Bone)==INDEX_NONE)
        {
            for(const auto& Info:Mesh->GetRefSkeleton().GetRawRefBoneInfo()) UE_LOG(LogTemp,Display,TEXT("PLAYER_IMPORT_BONE %s"),*Info.Name.ToString());
            UE_LOG(LogTemp,Display,TEXT("PLAYER_IMPORT_MISSING_BONE %s"),*Name.ToString()); return false;
        }
        auto* Socket=Mesh->FindSocket(Name);
        if(!Socket) { Socket=NewObject<USkeletalMeshSocket>(Mesh); Socket->SocketName=Name; Mesh->AddSocket(Socket); }
        Socket->BoneName=Bone; Socket->RelativeLocation=FVector::ZeroVector;
        // Blender bone +Y becomes native glTF/Unreal local +Z. Legacy blades
        // and arrows extend on local +X; bow/quiver already extend on +Z.
        Socket->RelativeRotation=(Name==TEXT("Socket.Weapon.R") || Name==TEXT("Socket.Arrow"))?FRotator(90,0,0):FRotator::ZeroRotator;
        Socket->RelativeScale=FVector::OneVector; Socket->bForceAlwaysAnimated=true;
    }
    Mesh->MarkPackageDirty(); return true;
#else
    return false;
#endif
}
bool ULHPlayerVisualComponent::CompactImportedMesh(USkeletalMesh* Mesh)
{
#if WITH_EDITOR
    if(!Mesh || Mesh->GetLODNum()!=1 || !Mesh->HasMeshDescription(0) || !Mesh->GetMorphTargets().IsEmpty()) return false;
    auto* Model=Mesh->GetImportedModel(); if(!Model || Model->LODModels.Num()!=1) return false;
    Mesh->Modify(); for(auto& Section:Model->LODModels[0].Sections) Section.SoftVertices.Empty(); Mesh->MarkPackageDirty(); return true;
#else
    return false;
#endif
}
void ULHPlayerVisualComponent::ClearImported()
{
    if(ImportedHair) ImportedHair->DestroyComponent(); ImportedHair=nullptr;
    if(ImportedBody) ImportedBody->DestroyComponent(); ImportedBody=nullptr;
}
void ULHPlayerVisualComponent::RebuildImported(const FLHCharacterRecord& C)
{
    ClearImported();
    using namespace LHPlayerVisualPrivate;
    int32 HairChoice=0,SkinChoice=0;
    if(!LHPlayerVisual::ResolveAppearance(C,ImportedIndex,HairChoice,SkinChoice)) return;
    const int32 Hair=ImportedIndex*2+HairChoice, Skin=ImportedIndex*3+SkinChoice;
    if(!BodyAssets[ImportedIndex] || !HairAssets[Hair] || !SkinAssets[Skin]) return;
    for(int32 A=0;A<8;++A) if(!ActionAssets[ImportedIndex*8+A]) return;
    auto Create=[&](USkeletalMesh* Mesh)
    {
        auto* Part=NewObject<USkeletalMeshComponent>(GetOwner());
        Part->SetMobility(EComponentMobility::Movable); Part->SetupAttachment(this);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision); Part->SetGenerateOverlapEvents(false); Part->SetCanEverAffectNavigation(false);
        Part->SetSkeletalMesh(Mesh); Part->RegisterComponent(); Part->SetVisibility(bEnabled); return Part;
    };
    ImportedBody=Create(BodyAssets[ImportedIndex]); ImportedHair=Create(HairAssets[Hair]);
    ImportedHair->SetLeaderPoseComponent(ImportedBody);
    const auto& Slots=BodyAssets[ImportedIndex]->GetMaterials();
    for(int32 I=0;I<Slots.Num();++I) if(Slots[I].MaterialSlotName.ToString().Contains(TEXT("SkinTint"))) ImportedBody->SetMaterial(I,SkinAssets[Skin]);
    // Keep only equipment visible; procedural anchors remain available for fallback/tests.
    for(const auto& Piece:RenderPieces) if(Piece)
    {
        const FString Id=Piece->GetRecipe().Id.ToString();
        const bool Equipment=Id.EndsWith(TEXT("Weapon")) || Id.EndsWith(TEXT("Quiver")) || Id.EndsWith(TEXT("BowArrow")) || Id.Contains(TEXT("BowString"));
        if(!Equipment) Piece->SetActorHiddenInGame(true);
        else if(Id.EndsWith(TEXT("Weapon")))
        {
            const FName Grip=Socket(TEXT("Weapon"))->GetAttachParent()==Socket(TEXT("Socket.Bow.L"))?FName(TEXT("Socket.Bow.L")):FName(TEXT("Socket.Weapon.R"));
            Piece->AttachToComponent(ImportedBody,FAttachmentTransformRules::SnapToTargetNotIncludingScale,Grip);
        }
        else if(Id.EndsWith(TEXT("Quiver"))) Piece->AttachToComponent(ImportedBody,FAttachmentTransformRules::SnapToTargetNotIncludingScale,TEXT("Socket.Quiver.Back"));
        else if(Id.EndsWith(TEXT("BowArrow"))) Piece->AttachToComponent(ImportedBody,FAttachmentTransformRules::SnapToTargetNotIncludingScale,TEXT("Socket.Arrow"));
    }
}
void ULHPlayerVisualComponent::SampleImported()
{
    if(!ImportedBody) return;
    auto* Clip=ActionAssets[ImportedIndex*8+static_cast<uint8>(CurrentPose)].Get();
    const bool Loop=CurrentPose==ELHPlayerPose::Idle || CurrentPose==ELHPlayerPose::Walk || CurrentPose==ELHPlayerPose::Run;
    float Time=Clock;
    if(CurrentPose==ELHPlayerPose::Walk || CurrentPose==ELHPlayerPose::Run) Time=Phase*Clip->GetPlayLength();
    else if(CurrentPose==ELHPlayerPose::Hit) Time=.2f-FlinchRemaining;
    else if(CurrentPose==ELHPlayerPose::Death) Time=DeathAge;
    else if(!Loop)
    {
        const float Contact=CurrentPose==ELHPlayerPose::Melee?.5f:.7f;
        Time=bAction?(ImpactDelay>0?Contact*FMath::Clamp(ActionAge/ImpactDelay,0.f,1.f):Contact):Contact+(Clip->GetPlayLength()-Contact)*(1-ReleaseRemaining/.18f);
    }
    Time=Loop?FMath::Fmod(Time,Clip->GetPlayLength()):FMath::Clamp(Time,0.f,Clip->GetPlayLength());
    ImportedBody->PlayAnimation(Clip,Loop); ImportedBody->SetPlayRate(0); ImportedBody->SetPosition(Time,false);
    ImportedBody->RefreshBoneTransforms();
    // Move the legacy equipment anchors to authored sockets. This preserves
    // their hierarchy (including bow string) and never moves the pawn/capsule.
    for(const FName Name:{FName(TEXT("Socket.Weapon.R")),FName(TEXT("Socket.Bow.L")),FName(TEXT("Socket.Quiver.Back"))})
        if(auto* Anchor=Socket(Name)) Anchor->SetWorldTransform(ImportedBody->GetSocketTransform(Name));
    if(auto* Arrow=Socket(TEXT("BowArrow"))) Arrow->SetWorldTransform(ImportedBody->GetSocketTransform(TEXT("Socket.Arrow")));
}
USceneComponent* ULHPlayerVisualComponent::Socket(FName Name) const
{ for(const auto& A:Anchors) if(A && A->GetFName()==Name) return A; return nullptr; }
void ULHPlayerVisualComponent::Rebuild(const TArray<FLHPlayerPart>& Parts)
{
    for(const auto& P:RenderPieces) if(P) P->Destroy(); RenderPieces.Reset();
    for(int32 I=Anchors.Num()-1;I>=0;--I) if(Anchors[I]) Anchors[I]->DestroyComponent(); Anchors.Reset();
    for(const auto& Part:Parts)
    {
        auto* Anchor=NewObject<USceneComponent>(GetOwner(),Part.Name);
        Anchor->SetMobility(EComponentMobility::Movable); Anchor->SetCanEverAffectNavigation(false);
        Anchor->SetupAttachment(Part.Parent.IsNone()?this:Socket(Part.Parent)); Anchor->SetRelativeTransform(Part.Transform); Anchor->RegisterComponent(); Anchors.Add(Anchor);
        if(Part.Recipe.Geometry.IsEmpty()) continue;
        auto* Piece=GetWorld()->SpawnActor<ALHVisualPiece>();
        if(!Piece) continue;
        Piece->SetOwner(GetOwner()); Piece->Build(Part.Recipe);
        Piece->AttachToComponent(Anchor,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        Piece->SetActorHiddenInGame(!bEnabled); RenderPieces.Add(Piece);
    }
}
void ULHPlayerVisualComponent::Unbind()
{ if(auto* C=BoundCombat.Get()) { C->OnAttackCommitted.Remove(CommitHandle); C->OnAttackFinished.Remove(FinishHandle); } BoundCombat.Reset(); }
void ULHPlayerVisualComponent::Bind(ULHCombatComponent* C)
{
    if(C==BoundCombat.Get()) return;
    Unbind(); bAction=false; ReleaseRemaining=0; PreviousHealth=-1;
    BoundCombat=C;
    if(C) { CommitHandle=C->OnAttackCommitted.AddUObject(this,&ULHPlayerVisualComponent::Committed); FinishHandle=C->OnAttackFinished.AddUObject(this,&ULHPlayerVisualComponent::Finished); }
}
void ULHPlayerVisualComponent::Committed(const FLHAttackEvent& E)
{
    ActionIdentity=E.Identity; ReleaseRemaining=0; bAction=true; ActionAge=0; ImpactDelay=FMath::Max(0.f,float(E.ImpactSeconds));
    const FString Id=E.Ability.Value.ToString();
    ActionPose=Id.Contains(TEXT("Bow"))?ELHPlayerPose::Bow:Id.StartsWith(TEXT("Spell."))?ELHPlayerPose::Cast:ELHPlayerPose::Melee;
}
void ULHPlayerVisualComponent::Finished(const FLHAttackEvent& Event,ELHAttackOutcome Outcome)
{ if(!(Event.Identity==ActionIdentity)) return; bAction=false; ReleaseRemaining=(Outcome==ELHAttackOutcome::ResolvedHit || Outcome==ELHAttackOutcome::ResolvedMiss)?.18f:0; }
void ULHPlayerVisualComponent::SetPresentationEnabled(bool Enabled)
{
    bEnabled=Enabled;
    if(ImportedBody) ImportedBody->SetVisibility(Enabled);
    if(ImportedHair) ImportedHair->SetVisibility(Enabled);
    for(const auto& P:RenderPieces) if(P)
    {
        const FString Id=P->GetRecipe().Id.ToString();
        const bool Equipment=Id.EndsWith(TEXT("Weapon")) || Id.EndsWith(TEXT("Quiver")) || Id.Contains(TEXT("BowString")) || Id.EndsWith(TEXT("BowArrow"));
        P->SetActorHiddenInGame(!Enabled || (ImportedBody && !Equipment) || (Id.EndsWith(TEXT("BowArrow")) && (CurrentPose!=ELHPlayerPose::Bow || !bAction)));
    }
}
void ULHPlayerVisualComponent::Present(const FLHCharacterRecord* C,ULHCombatComponent* Combat,float Speed,float Seconds)
{
    Bind(Combat);
    const auto Parts=LHPlayerVisual::Build(C?*C:FLHCharacterRecord{}); uint32 Hash=LHPlayerVisual::Fingerprint(Parts);
    TArray<FString> IDs; if(C) for(const auto& ID:C->AppearanceIds) IDs.Add(ID.Value.ToString()); IDs.Sort();
    for(const auto& ID:IDs) Hash=HashCombine(Hash,FCrc::StrCrc32(*ID));
    if(!bBuilt || Hash!=BuiltFingerprint) { Rebuild(Parts); RebuildImported(C?*C:FLHCharacterRecord{}); BuiltFingerprint=Hash; bBuilt=true; }
    const float Dt=FMath::IsFinite(Seconds)?FMath::Max(0.f,Seconds):0;
    Clock+=Dt; Phase+=Dt*FMath::Max(0.f,Speed)/(Speed>300?360.f:220.f); ActionAge+=Dt;
    ReleaseRemaining=FMath::Max(0.f,ReleaseRemaining-Dt); FlinchRemaining=FMath::Max(0.f,FlinchRemaining-Dt);
    // IsAlive also requires an attached GAS avatar. Travel/spawn detachment is
    // not a death pose: use the saved health until this pawn owns the avatar.
    const bool Ready=Combat && Combat->GetAvatarActor()==GetOwner();
    const bool Alive=Ready?Combat->IsAlive():!C || C->CurrentHealth.Resolution!=ELHValueResolution::Resolved || C->CurrentHealth.Value>0;
    if(!Ready) { PreviousHealth=-1; bAction=false; ReleaseRemaining=0; FlinchRemaining=0; }
    if(Ready) { const float Health=Combat->GetCombatAttributes()->GetHealth(); if(Alive && PreviousHealth>=0 && Health<PreviousHealth) FlinchRemaining=.2f; PreviousHealth=Health; }
    if(Alive && !bWasAlive) { DeathAge=0; bAction=false; ReleaseRemaining=0; FlinchRemaining=0; } bWasAlive=Alive;
    if(!Alive) DeathAge+=Dt;
    CurrentPose=!Alive?ELHPlayerPose::Death:FlinchRemaining>0?ELHPlayerPose::Hit:(bAction || ReleaseRemaining>0)?ActionPose:Speed>300?ELHPlayerPose::Run:Speed>5?ELHPlayerPose::Walk:ELHPlayerPose::Idle;
    const float Cycle=FMath::Sin(Phase*2*PI), Stride=FMath::Clamp(Speed/450.f,0.f,1.f)*38;
    auto Rotate=[&](FName Name,FRotator R){ if(auto* A=Socket(Name)) A->SetRelativeRotation(R); };
    Rotate(TEXT("Leg.L"),FRotator(Cycle*Stride,0,0)); Rotate(TEXT("Leg.R"),FRotator(-Cycle*Stride,0,0));
    Rotate(TEXT("Arm.L"),FRotator(-Cycle*Stride*.7f,0,0)); Rotate(TEXT("Arm.R"),FRotator(Cycle*Stride*.7f,0,0));
    Rotate(TEXT("Socket.Bow.L"),FRotator::ZeroRotator);
    if(auto* Torso=Socket(TEXT("Torso"))) Torso->SetRelativeLocation({0,0,96+FMath::Sin(Clock*2)*1.2f});
    Rotate(TEXT("Torso"),FRotator(CurrentPose==ELHPlayerPose::Hit?-18:0,0,0));
    const float Wind=ImpactDelay>0?FMath::Clamp(ActionAge/ImpactDelay,0.f,1.f):1;
    if(CurrentPose==ELHPlayerPose::Melee) Rotate(TEXT("Arm.R"),FRotator(ReleaseRemaining>0?60:-75+Wind*135,0,-15));
    if(CurrentPose==ELHPlayerPose::Bow) { Rotate(TEXT("Arm.L"),FRotator(80,0,0)); Rotate(TEXT("Socket.Bow.L"),FRotator(-80,0,0)); Rotate(TEXT("Arm.R"),FRotator(ReleaseRemaining>0?50:75,Wind*35,0)); }
    const float Draw=CurrentPose==ELHPlayerPose::Bow && bAction?Wind*18:0;
    for(int32 Sign:{-1,1}) if(auto* String=Socket(Sign<0?TEXT("BowString.Lower"):TEXT("BowString.Upper")))
    {
        String->SetRelativeLocation({-15-Draw/2,0,Sign*31.5f});
        String->SetRelativeRotation(FRotator(Sign*FMath::RadiansToDegrees(FMath::Atan2(Draw,63.f)),0,0));
        String->SetRelativeScale3D({1,1,FMath::Sqrt(63*63+Draw*Draw)/63});
    }
    if(auto* Arrow=Socket(TEXT("BowArrow"))) Arrow->SetRelativeLocation({-15-Draw,0,0});
    for(const auto& Piece:RenderPieces) if(Piece && Piece->GetRecipe().Id==TEXT("Presentation.Player.BowArrow"))
        Piece->SetActorHiddenInGame(!bEnabled || CurrentPose!=ELHPlayerPose::Bow || !bAction);
    if(CurrentPose==ELHPlayerPose::Cast) { Rotate(TEXT("Arm.L"),FRotator(75,0,-25)); Rotate(TEXT("Arm.R"),FRotator(75,0,25)); }
    SetRelativeRotation(!ImportedBody && CurrentPose==ELHPlayerPose::Death?FRotator(-90*FMath::Clamp(DeathAge/.65f,0.f,1.f),0,0):FRotator::ZeroRotator);
    SampleImported();
}
void ULHPlayerVisualComponent::EndPlay(const EEndPlayReason::Type Reason)
{ Unbind(); ClearImported(); for(const auto& P:RenderPieces) if(P) P->Destroy(); RenderPieces.Reset(); Super::EndPlay(Reason); }

void ULHPlayerVisualComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    Unbind(); ClearImported();
    for(const auto& P:RenderPieces) if(P) P->Destroy();
    RenderPieces.Reset();
    for(int32 I=Anchors.Num()-1;I>=0;--I) if(Anchors[I]) Anchors[I]->DestroyComponent();
    Anchors.Reset();
    Super::OnComponentDestroyed(bDestroyingHierarchy);
}
