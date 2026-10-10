#include "Visual/Monsters/LHMonsterVisual.h"
#include "Visual/LHVisualKit.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Misc/PackageName.h"
#include "ProceduralMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Framework/LHEnemyCharacter.h"
#include "AI/LHEnemyAIController.h"
namespace LHMonsterPrivate {
const TCHAR* Ids[]={TEXT("BrownRat"),TEXT("Bat"),TEXT("GreenSlime"),TEXT("Goblin"),TEXT("GiantSpider"),TEXT("Balork"),TEXT("GoblinWarrior"),TEXT("Atrocity"),TEXT("DungeonBat"),TEXT("GiantBat"),TEXT("UndeadBat")};
const TCHAR* Art[]={TEXT("rat"),TEXT("bat"),TEXT("slime"),TEXT("goblin"),TEXT("giant_spider"),TEXT("balork"),TEXT("goblin_warrior"),TEXT("atrocity"),TEXT("dungeon_bat"),TEXT("giant_bat"),TEXT("undead_bat")};
const TCHAR* Actions[]={TEXT("idle"),TEXT("move"),TEXT("attack"),TEXT("hit"),TEXT("death")};
#include "LHMonsterRecipes.inl"
bool Matches(FName Id, const FPiece& P) { return Id.ToString()==FString(TEXT("Enemy."))+P.Id; }
}
ULHMonsterVisual::ULHMonsterVisual()
{
    PrimaryComponentTick.bCanEverTick=true;
    PrimaryComponentTick.TickGroup=TG_PostUpdateWork;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    ShapeMeshes={Sphere.Object,Cone.Object,Cylinder.Object};
    // Hard CDO references retain imported art and dependencies in cooks.
    for(int32 I=0;I<11;++I)
    {
        const FString Root=FString(TEXT("/Game/Lighthaven/Art/Creatures/"))+LHMonsterPrivate::Art[I]+TEXT("/");
        const FString Mesh=Root+TEXT("SK_")+LHMonsterPrivate::Art[I];
        CreatureAssets.Add(FPackageName::DoesPackageExist(Mesh)?LoadObject<USkeletalMesh>(nullptr,*Mesh):nullptr);
        for(const TCHAR* Action:LHMonsterPrivate::Actions)
        {
            const FString Path=Root+TEXT("A_")+Action;
            CreatureActions.Add(FPackageName::DoesPackageExist(Path)?LoadObject<UAnimSequence>(nullptr,*Path):nullptr);
        }
    }
}
FString ULHMonsterVisual::ArtId(FName Id)
{
    for(int32 I=0;I<11;++I) if(Id==FName(*(FString(TEXT("Enemy."))+LHMonsterPrivate::Ids[I]))) return LHMonsterPrivate::Art[I];
    return FString();
}
bool ULHMonsterVisual::Known(FName Id)
{ for(const auto& P:LHMonsterPrivate::Pieces) if(LHMonsterPrivate::Matches(Id,P)) return true; return false; }
uint32 ULHMonsterVisual::RecipeFingerprint(FName Id)
{
    uint32 Hash=GetTypeHash(Id);
    for(const auto& P:LHMonsterPrivate::Pieces) if(LHMonsterPrivate::Matches(Id,P))
    {
        // Reuse the kit's explicit-field checksum rather than hash object memory/padding.
        FLHVisualRecipe R; R.Id=FName(P.Mesh); R.Colors[0]=FLinearColor(P.Color);
        FLHVisualBox B; B.Center=P.Center; B.Size=P.Size; B.Rotation=P.Rotation.Rotator(); R.Geometry.Add(B);
        Hash=HashCombine(Hash,R.Fingerprint()); Hash=HashCombine(Hash,GetTypeHash(P.Weapon));
    }
    return Hash;
}
void ULHMonsterVisual::Clear()
{
    for(const auto& P:KitPieces) if(IsValid(P)) P->Destroy();
    for(const auto& P:Parts) if(IsValid(P) && P->GetOwner()==GetOwner()) P->DestroyComponent();
    if(CreatureMesh) CreatureMesh->DestroyComponent();
    CreatureMesh=nullptr; CreatureIndex=INDEX_NONE;
    KitPieces.Reset(); Parts.Reset(); Rest.Reset(); Weapons.Reset();
}
void ULHMonsterVisual::EndPlay(const EEndPlayReason::Type Reason) { Clear(); Super::EndPlay(Reason); }
bool ULHMonsterVisual::Build(FName Id,double Radius,double HH,bool UseImportedArt)
{
    if(!GetOwner() || !GetWorld() || !FMath::IsFinite(Radius) || !FMath::IsFinite(HH) || Radius<=0 || HH<=0) return false;
    Clear(); ComponentTags.Remove(TEXT("LH.Monster.UnknownPlaceholder")); Definition=Id; Clock=AttackElapsed=HitRemaining=StrikeRemaining=DeathElapsed=0; Pending=Dead=false;
    Motion=ELHMonsterMotion::Idle;
    SetRelativeLocation(FVector(0,0,-HH));
    const FString Art=ArtId(Id);
    for(int32 I=0;I<11;++I) if(UseImportedArt && Art==LHMonsterPrivate::Art[I] && CreatureAssets[I])
    {
        bool Complete=true;
        for(int32 A=0;A<5;++A) Complete &= CreatureActions[I*5+A]!=nullptr && CreatureActions[I*5+A]->GetSkeleton()==CreatureAssets[I]->GetSkeleton();
        if(!Complete) break;
        CreatureIndex=I;
        CreatureMesh=NewObject<USkeletalMeshComponent>(GetOwner());
        GetOwner()->AddInstanceComponent(CreatureMesh);
        CreatureMesh->SetSkeletalMesh(CreatureAssets[I]);
        CreatureMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CreatureMesh->SetGenerateOverlapEvents(false); CreatureMesh->SetCanEverAffectNavigation(false);
        CreatureMesh->SetupAttachment(this); CreatureMesh->RegisterComponent();
        CreatureMesh->SetVisibility(IsVisible(),true);
        CreatureMesh->PlayAnimation(CreatureActions[I*5],true); CreatureMesh->SetPlayRate(0);
        return true;
    }
    auto Add=[&](const LHMonsterPrivate::FPiece& P)
    {
        USceneComponent* Part=nullptr;
        FVector Scale=P.Size/100.f;
        if(FString(P.Mesh)==TEXT("Cube"))
        {
            auto* Actor=GetWorld()->SpawnActor<ALHVisualPiece>();
            if(!Actor) return;
            Actor->SetOwner(GetOwner());
            FLHVisualRecipe R; R.Id=Id; R.Colors[0]=FLinearColor(P.Color); R.Roughness[0]=.85f;
            FLHVisualBox B; B.Size=P.Size; R.Geometry.Add(B); // Empty Collision: purely decorative.
            if(!Actor->Build(R)) { Actor->Destroy(); return; }
            KitPieces.Add(Actor); Part=Actor->GetRootComponent(); Scale=FVector::OneVector;
        }
        else
        {
            auto* Mesh=NewObject<UStaticMeshComponent>(GetOwner()); GetOwner()->AddInstanceComponent(Mesh);
            const int32 Shape=FString(P.Mesh)==TEXT("Sphere")?0:FString(P.Mesh)==TEXT("Cone")?1:2;
            Mesh->SetStaticMesh(ShapeMeshes[Shape]);
            Mesh->SetCollisionProfileName(TEXT("NoCollision")); Mesh->SetGenerateOverlapEvents(false); Mesh->SetCanEverAffectNavigation(false);
            Mesh->RegisterComponent();
            if(auto* M=Mesh->CreateDynamicMaterialInstance(0)) { M->SetVectorParameterValue(TEXT("Color"),FLinearColor(P.Color)); M->SetScalarParameterValue(TEXT("Roughness"),.85f); }
            Part=Mesh;
        }
        Part->SetMobility(EComponentMobility::Movable);
        Part->AttachToComponent(this,FAttachmentTransformRules::KeepRelativeTransform);
        FTransform T(P.Rotation,P.Center,Scale); Part->SetRelativeTransform(T);
        Part->SetVisibility(IsVisible(), true);
        Parts.Add(Part); Rest.Add(T); Weapons.Add(P.Weapon);
    };
    FlyingFloor=0;
    if(Id.ToString().Contains(TEXT("Bat"))) {
        FlyingFloor=TNumericLimits<double>::Max();
        for(const auto& P:LHMonsterPrivate::Pieces) if(LHMonsterPrivate::Matches(Id,P)) {
            const FVector Extent=P.Rotation.RotateVector(FVector(P.Size.X,0,0)).GetAbs()+P.Rotation.RotateVector(FVector(0,P.Size.Y,0)).GetAbs()+P.Rotation.RotateVector(FVector(0,0,P.Size.Z)).GetAbs();
            FlyingFloor=FMath::Min(FlyingFloor,P.Center.Z-Extent.Z*.5);
        }
        if(FlyingFloor==TNumericLimits<double>::Max()) FlyingFloor=0;
    }
    const bool Found=Known(Id);
    for(const auto& P:LHMonsterPrivate::Pieces) if(LHMonsterPrivate::Matches(Id,P)) Add(P);
    if(!Found)
    {
        UE_LOG(LogTemp,Warning,TEXT("W5-03 UNKNOWN MONSTER %s: magenta placeholder"),*Id.ToString());
        const LHMonsterPrivate::FPiece P={TEXT("Unknown"),TEXT("Cube"),FVector(Radius,Radius,HH),FVector(0,0,HH*.5),FQuat::Identity,FColor::Magenta,false}; Add(P);
        ComponentTags.AddUnique(TEXT("LH.Monster.UnknownPlaceholder"));
    }
    else ComponentTags.Remove(TEXT("LH.Monster.UnknownPlaceholder"));
    return Found && !Parts.IsEmpty();
}
void ULHMonsterVisual::Attack(double Delay, double CommitTime) { if(Dead) return; Pending=true; CommitWorldTime=CommitTime; AttackElapsed=0; Impact=FMath::Max(0.,Delay); StrikeRemaining=0; }
void ULHMonsterVisual::CancelAttack() { Pending=false; StrikeRemaining=0; }
void ULHMonsterVisual::Strike() { if(Dead) return; Pending=false; StrikeRemaining=.18; }
void ULHMonsterVisual::Hit() { if(!Dead) HitRemaining=.22; }
void ULHMonsterVisual::Die() { if(Dead) return; Dead=true; Pending=false; HitRemaining=StrikeRemaining=0; DeathElapsed=0; }
void ULHMonsterVisual::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Dt,Type,Tick);
    const auto* E=Cast<ALHEnemyCharacter>(GetOwner());
    const auto* AI=E?Cast<ALHEnemyAIController>(E->GetController()):nullptr;
    if(AI && AI->GetState()==ELHAIState::Dead) Die();
    const bool Moving=E && E->GetVelocity().SizeSquared2D()>1 && (!AI || AI->GetState()==ELHAIState::Chase || AI->GetState()==ELHAIState::ReturnHome);
    // Use the authority world clock at runtime: a commit late in this frame must not
    // consume the entire frame again. Manual sampling remains available for tests.
    if(Pending && CommitWorldTime>=0 && GetWorld()) AttackElapsed=FMath::Max(0.,double(GetWorld()->GetTimeSeconds())-CommitWorldTime)-Dt;
    AdvancePresentation(Dt,Moving);
}
void ULHMonsterVisual::AdvancePresentation(float Dt,bool Moving)
{
    if(!bPresentationEnabled || !FMath::IsFinite(Dt) || Dt<0) return;
    Clock+=Dt; if(Pending) AttackElapsed+=Dt;
    HitRemaining=FMath::Max(0.,HitRemaining-Dt); StrikeRemaining=FMath::Max(0.,StrikeRemaining-Dt);
    if(Dead) DeathElapsed+=Dt;
    Motion=Dead?ELHMonsterMotion::Dead:StrikeRemaining>0?ELHMonsterMotion::Strike:Pending?(AttackElapsed<Impact?ELHMonsterMotion::Telegraph:ELHMonsterMotion::Strike):HitRemaining>0?ELHMonsterMotion::Hit:Moving?ELHMonsterMotion::Move:ELHMonsterMotion::Idle;
    if(CreatureMesh)
    {
        const int32 A=Dead?4:Motion==ELHMonsterMotion::Hit?3:(Motion==ELHMonsterMotion::Telegraph || Motion==ELHMonsterMotion::Strike)?2:Moving?1:0;
        UAnimSequence* Clip=CreatureActions[CreatureIndex*5+A];
        // Source contact is frame 30. The authority owns impact; animation only samples.
        double Time=Clock;
        if(A==2) Time=Pending?(Impact>0?FMath::Min(AttackElapsed/Impact,1.):1.):1.+(.18-StrikeRemaining);
        else if(A==3) Time=.22-HitRemaining;
        else if(A==4) Time=DeathElapsed;
        if(A<2 && Clip->GetPlayLength()>0) Time=FMath::Fmod(Time,double(Clip->GetPlayLength()));
        else Time=FMath::Clamp(Time,0.,double(Clip->GetPlayLength()));
        CreatureMesh->PlayAnimation(Clip,A<2); CreatureMesh->SetPlayRate(0);
        CreatureMesh->SetPosition(Time,false);
        return;
    }
    const FString S=Definition.ToString(); const bool Bat=S.Contains(TEXT("Bat")), Slime=S.EndsWith(TEXT("Slime"));
    const double Phase=Clock*(Bat?12:Slime?4:Moving?16:3);
    for(int32 I=0;I<Parts.Num();++I)
    {
        FTransform T=Rest[I]; FVector Pos=T.GetLocation(), Scale=T.GetScale3D(); FQuat Rot=T.GetRotation();
        if(Dead)
        {
            const double Collapse=FMath::Clamp(DeathElapsed/.6,0.,1.);
            const double Z=FMath::Lerp(1.,.18,Collapse); Pos.Z=(Pos.Z-FlyingFloor*Collapse)*Z; Scale.Z*=Z;
        }
        else
        {
            const double Wave=FMath::Sin(Phase+I*.7);
            const bool Wing=Bat && FMath::Abs(Pos.Y)>10;
            if(Wing) {
                const double Angle=Motion==ELHMonsterMotion::Strike?-.3:FMath::Sin(Phase)*.3;
                const FQuat Flap(FVector::ForwardVector,Angle*FMath::Sign(Pos.Y));
                const FVector Pivot(0,0,Rest[0].GetLocation().Z);
                Pos=Pivot+Flap.RotateVector(Pos-Pivot); Rot=Flap*Rot;
            }
            else if(Slime) { if(I>0) { Scale.X*=1+Wave*.08; Scale.Z*=1-Wave*.08; } }
            else if(Moving && Pos.Z<60 && !(Definition==TEXT("Enemy.Balork") && I>=19)) { Pos.Z+=FMath::Max(0.,Wave)*4; Rot=FQuat(FVector::RightVector,Wave*.15)*Rot; }
            else if(Pos.Z>20 && !Weapons[I]) Pos.Z+=FMath::Sin(Phase)*.5;
            const bool Contact=Weapons[I] || (Bat && I<2) ||
                (S.EndsWith(TEXT("BrownRat")) && (I==1 || I==2)) ||
                (Slime && I==2) || (S.EndsWith(TEXT("GiantSpider")) && I==2) ||
                (S.EndsWith(TEXT("Atrocity")) && Pos.Y < -60);
            if(Contact && Motion==ELHMonsterMotion::Telegraph) {
                const double P=Impact>0?FMath::Clamp(AttackElapsed/Impact,0.,1.):1.; Pos.X-=P*8;
                if(Slime) Scale.Z*=1+P*.2;
            }
            if(Contact && Motion==ELHMonsterMotion::Strike) {
                Pos.X+=Slime?4:10;
                if(Slime) { Scale.X*=1.12; Scale.Z*=.8; }
                if(S.EndsWith(TEXT("Atrocity"))) Pos.Z-=8;
            }
            if(HitRemaining>0) { Pos.X-=8; Rot=FQuat(FVector::RightVector,-.12)*Rot; }
        }
        T.SetLocation(Pos); T.SetScale3D(Scale); T.SetRotation(Rot); Parts[I]->SetRelativeTransform(T);
    }
}
