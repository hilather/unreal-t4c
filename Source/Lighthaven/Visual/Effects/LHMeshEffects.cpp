#include "Visual/Effects/LHMeshEffects.h"
#include "Visual/Props/LHPropsCatalog.h"
#include "Framework/LHEnemyCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
ULHMeshEffects::ULHMeshEffects() { PrimaryComponentTick.bCanEverTick=true; }
void ULHMeshEffects::Bind(ULHCombatComponent* C)
{
    if(Bound.Get()==C) return;
    if(auto* Old=Bound.Get()) { Old->OnAttackCommitted.Remove(CommitHandle); Old->OnAttackFinished.Remove(FinishHandle); }
    for(auto& S:Slots) { S.Duration=0; if(S.Mesh) S.Mesh->SetVisibility(false); }
    Bound=C;
    if(C) { CommitHandle=C->OnAttackCommitted.AddUObject(this,&ULHMeshEffects::Committed); FinishHandle=C->OnAttackFinished.AddUObject(this,&ULHMeshEffects::Finished); }
}
AActor* ULHMeshEffects::TargetActor(const FLHAttackEvent& E) const
{
    for(TActorIterator<ALHEnemyCharacter> It(GetWorld());It;++It)
    { const auto Id=It->GetEntityId(E.Target.RunId); if(Id.InstanceId==E.Target.InstanceId && Id.Area.Content.Value==E.Target.Area.Content.Value) return *It; }
    // Enemy attacks target the current player avatar; no authority lookup/write.
    if(auto* PC=GetWorld()->GetFirstPlayerController()) if(APawn* Pawn=PC->GetPawn()) return Pawn;
    return GetOwner();
}
FVector ULHMeshEffects::Target(const FLHAttackEvent& E) const { return TargetActor(E)->GetActorLocation(); }
FVector ULHMeshEffects::Ground(const FLHAttackEvent& E) const
{
    auto* Actor=TargetActor(E); FVector P=Actor->GetActorLocation();
    if(auto* Capsule=Actor->FindComponentByClass<UCapsuleComponent>()) P=Capsule->GetComponentLocation()-FVector(0,0,Capsule->GetScaledCapsuleHalfHeight());
    return P;
}
bool ULHMeshEffects::Emit(FName Kind,FVector Start,FVector End,float Duration,bool Travel,const FLHHitIdentity& Identity)
{
    auto* Asset=ULHPropsCatalog::Mesh(Kind);
    if(!Asset || !FMath::IsFinite(Duration) || Duration<=0) return false;
    FLHMeshEffectSlot* S=Slots.FindByPredicate([](const auto& V){return V.Duration<=0;});
    if(!S) { if(Slots.Num()>=PoolCap) return false; S=&Slots.AddDefaulted_GetRef(); }
    if(!S->Mesh)
    {
        S->Mesh=NewObject<UStaticMeshComponent>(GetOwner()); S->Mesh->SetMobility(EComponentMobility::Movable);
        S->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); S->Mesh->SetGenerateOverlapEvents(false); S->Mesh->SetCanEverAffectNavigation(false); S->Mesh->RegisterComponent();
    }
    S->Kind=Kind; S->Start=Start; S->End=End; S->Age=0; S->Duration=Duration; S->Travel=Travel; S->Identity=Identity;
    S->Mesh->EmptyOverrideMaterials(); S->Mesh->SetStaticMesh(Asset);
    for(int32 I=0;I<S->Mesh->GetNumMaterials();++I) if(auto* M=S->Mesh->CreateAndSetMaterialInstanceDynamic(I)) M->SetScalarParameterValue(TEXT("PresentationFade"),1);
    S->Mesh->SetWorldLocation(Start); S->Mesh->SetWorldRotation(Travel?(End-Start).Rotation():FRotator::ZeroRotator); S->Mesh->SetWorldScale3D(FVector::OneVector); S->Mesh->SetVisibility(true); return true;
}
void ULHMeshEffects::Committed(const FLHAttackEvent& E)
{
    const FName Id=E.Ability.Value;
    const FName Kind=Id==TEXT("Spell.FireDart")?FName(TEXT("FireDart")):Id==TEXT("Spell.StoneShard")?FName(TEXT("StoneShard")):Id==TEXT("Attack.Ranged.Bow")?FName(TEXT("ArrowFlight")):NAME_None;
    if(!Kind.IsNone()) Emit(Kind,GetOwner()->GetActorLocation(),Target(E),float(E.ImpactSeconds),true,E.Identity);
}
void ULHMeshEffects::Finished(const FLHAttackEvent& E,ELHAttackOutcome Outcome)
{
    for(auto& S:Slots) if(S.Duration>0 && S.Identity==E.Identity) { S.Duration=0; S.Mesh->SetVisibility(false); }
    if(Outcome!=ELHAttackOutcome::ResolvedHit && Outcome!=ELHAttackOutcome::ResolvedMiss) return;
    const FName Id=E.Ability.Value;
    const FVector P=Target(E), Self=GetOwner()->GetActorLocation();
    if(Id==TEXT("Spell.Light")) return; // Orb follows the authoritative remaining state.
    if(Id==TEXT("Spell.HealLight")) { Emit(TEXT("HealLight"),Self-FVector(0,0,90),Self-FVector(0,0,55),.65f); return; }
    if(Id==TEXT("Spell.DustDevil")) { Emit(TEXT("DustDevil"),Ground(E),Ground(E),.65f); return; }
    if(!Id.ToString().StartsWith(TEXT("Spell.")) && Id!=TEXT("Attack.Ranged.Bow")) Emit(TEXT("SlashArc"),Self,Self,.22f);
    if(Outcome==ELHAttackOutcome::ResolvedHit)
    {
        const FName Kind=Id==TEXT("Spell.FireDart")?FName(TEXT("FireImpact")):Id==TEXT("Spell.StoneShard")?FName(TEXT("StoneShatter")):Id==TEXT("Attack.Ranged.Bow")?FName(TEXT("ImpactPuff")):FName(TEXT("MeleeSpark"));
        Emit(Kind,P,P+FVector(0,0,12),.35f);
    }
}
int32 ULHMeshEffects::ActiveCount() const { return Slots.FilterByPredicate([](const auto& S){return S.Duration>0;}).Num(); }
void ULHMeshEffects::Advance(float Seconds)
{
    const float Dt=FMath::IsFinite(Seconds)?FMath::Max(0.f,Seconds):0;
    for(auto& S:Slots) if(S.Duration>0)
    {
        S.Age+=Dt; const float T=FMath::Clamp(S.Age/S.Duration,0.f,1.f);
        if(T>=1) { S.Duration=0; S.Mesh->SetVisibility(false); continue; }
        S.Mesh->SetWorldLocation(FMath::Lerp(S.Start,S.End,T));
        const float Fade=FMath::Clamp((1-T)/.35f,0.f,1.f);
        for(int32 I=0;I<S.Mesh->GetNumMaterials();++I) if(auto* M=Cast<UMaterialInstanceDynamic>(S.Mesh->GetMaterial(I))) M->SetScalarParameterValue(TEXT("PresentationFade"),Fade);
        // Opaque fragments shrink out; translucent surfaces additionally fade.
        const float Scale=S.Travel?1.f:FMath::Min(1.f,.15f+T*4)*FMath::Clamp((1-T)/.35f,0.f,1.f);
        FVector Size(Scale);
        if(S.Kind==TEXT("FireDart")) Size.X=.7f+.4f*FMath::Sin(T*PI);
        S.Mesh->SetWorldScale3D(Size);
        if(S.Kind==TEXT("SlashArc")) S.Mesh->SetWorldRotation(FRotator(0,T*120,0));
        if(S.Kind==TEXT("DustDevil")) S.Mesh->SetWorldRotation(FRotator(0,S.Age*360,0));
        if(S.Kind==TEXT("StoneShard")) S.Mesh->AddLocalRotation(FRotator(0,0,Dt*45));
    }
}
void ULHMeshEffects::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick); Advance(Delta);
    const bool Light=Bound.IsValid() && Bound->GetLightRemainingSeconds()>0;
    if(Light && !Orb)
    {
        Orb=NewObject<UStaticMeshComponent>(GetOwner()); Orb->SetMobility(EComponentMobility::Movable); Orb->SetCollisionEnabled(ECollisionEnabled::NoCollision); Orb->SetGenerateOverlapEvents(false); Orb->SetCanEverAffectNavigation(false); Orb->SetStaticMesh(ULHPropsCatalog::Mesh(TEXT("Light"))); Orb->RegisterComponent();
        for(int32 I=0;I<Orb->GetNumMaterials();++I) Orb->CreateAndSetMaterialInstanceDynamic(I);
    }
    OrbFade=Light?1.f:FMath::Max(0.f,OrbFade-Delta*5);
    if(Orb) for(int32 I=0;I<Orb->GetNumMaterials();++I) if(auto* M=Cast<UMaterialInstanceDynamic>(Orb->GetMaterial(I))) M->SetScalarParameterValue(TEXT("PresentationFade"),OrbFade);
    if(Orb) { Orb->SetVisibility(OrbFade>0); Orb->SetWorldScale3D(FVector(OrbFade*(1+.05f*FMath::Sin(GetWorld()->GetTimeSeconds()*2)))); Orb->SetWorldLocation(GetOwner()->GetActorLocation()+FVector(0,0,100)); }
}
void ULHMeshEffects::EndPlay(const EEndPlayReason::Type Reason)
{ Bind(nullptr); for(auto& S:Slots) if(S.Mesh) S.Mesh->DestroyComponent(); Slots.Reset(); if(Orb) Orb->DestroyComponent(); Orb=nullptr; Super::EndPlay(Reason); }
