#include "Visual/Props/LHLootMarker.h"
#include "Visual/Props/LHPropsCatalog.h"
#include "Framework/LHEnemyCharacter.h"
#include "Components/StaticMeshComponent.h"
bool LHLootMarker::Lootable(const FLHCorpseLootRecord& C)
{ return C.bFinalized && !C.bClaimed && (!C.RemainingItems.IsEmpty() || (C.RemainingGold.Resolution==ELHValueResolution::Resolved && C.RemainingGold.Value>0)); }
void LHLootMarker::Present(ALHEnemyCharacter* Enemy,const FLHSaveSnapshot& Snapshot)
{
    if(!Enemy) return;
    bool Show=false;
    if(Enemy->IsCorpse()) for(const auto& Area:Snapshot.World.Areas) for(const auto& C:Area.Corpses)
    { const auto& L=Enemy->GetLife(); if(C.SourceLife.SpawnSlot==L.SpawnSlot && C.SourceLife.Area.Content.Value==L.Area.Content.Value && C.SourceLife.LifeGeneration==L.LifeGeneration) { Show=Lootable(C); break; } }
    UStaticMeshComponent* Badge=nullptr;
    TArray<UStaticMeshComponent*> Components; Enemy->GetComponents(Components);
    for(auto* M:Components) if(M->GetFName()==TEXT("CorpseLootBadge")) { Badge=M; break; }
    if(Show && !Badge)
    {
        Badge=NewObject<UStaticMeshComponent>(Enemy,TEXT("CorpseLootBadge")); Enemy->AddInstanceComponent(Badge);
        Badge->SetMobility(EComponentMobility::Movable); Badge->SetupAttachment(Enemy->GetRootComponent());
        Badge->SetCollisionEnabled(ECollisionEnabled::NoCollision); Badge->SetGenerateOverlapEvents(false); Badge->SetCanEverAffectNavigation(false);
        Badge->SetStaticMesh(ULHPropsCatalog::Mesh(TEXT("CorpseLootBadge"))); Badge->SetRelativeLocation(FVector(0,0,110)); Badge->SetRelativeScale3D(FVector(.45f)); Badge->RegisterComponent();
    }
    if(Badge) Badge->SetVisibility(Show);
}
