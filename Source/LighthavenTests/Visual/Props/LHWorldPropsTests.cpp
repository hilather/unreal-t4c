#include "Misc/AutomationTest.h"
#include "Visual/Props/LHLootMarker.h"
#include "Visual/Props/LHPropsCatalog.h"
#include "Visual/Player/LHPlayerEquipmentVisual.h"
#include "Visual/Player/LHPlayerVisual.h"
#include "Visual/Effects/LHMeshEffects.h"
#include "Framework/LHCharacter.h"
#include "Framework/LHEnemyCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWorldPropsPolicyTest,"Lighthaven.Visual.Props.WorldBindingPolicy",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWorldPropsPolicyTest::RunTest(const FString&)
{
    FLHCharacterRecord C;
    TestEqual(TEXT("Empty equipment"),ULHPlayerEquipmentVisual::Equipped(C,ELHEquipmentSlot::MainHand),NAME_None);
    FLHItemInstance I; I.Id.InstanceId=FGuid::NewGuid(); I.Definition.Value=TEXT("Item.RustedDirk"); C.Inventory.Add(I);
    FLHEquipmentBinding B; B.Slot=ELHEquipmentSlot::MainHand; B.Item=I.Id; C.Equipment.Add(B);
    TestEqual(TEXT("Exact dirk ID"),ULHPlayerEquipmentVisual::Equipped(C,B.Slot),I.Definition.Value);
    C.Inventory[0].Definition.Value=TEXT("Item.GoblinBlade"); TestEqual(TEXT("Exact goblin ID"),ULHPlayerEquipmentVisual::Equipped(C,B.Slot),FName(TEXT("Item.GoblinBlade")));
    C.Inventory.Reset(); TestEqual(TEXT("Dangling binding fallback"),ULHPlayerEquipmentVisual::Equipped(C,B.Slot),NAME_None);
    FLHCorpseLootRecord Corpse; TestFalse(TEXT("Unfinalized absent"),LHLootMarker::Lootable(Corpse));
    Corpse.bFinalized=true; TestFalse(TEXT("Empty absent"),LHLootMarker::Lootable(Corpse));
    Corpse.RemainingItems.Add(I); TestTrue(TEXT("Item loot badge"),LHLootMarker::Lootable(Corpse));
    Corpse.bClaimed=true; TestFalse(TEXT("Claimed absent"),LHLootMarker::Lootable(Corpse));
    Corpse.bClaimed=false; Corpse.RemainingItems.Reset(); Corpse.RemainingGold.Resolution=ELHValueResolution::Resolved; Corpse.RemainingGold.Value=1;
    TestTrue(TEXT("Gold loot badge"),LHLootMarker::Lootable(Corpse)); Corpse.RemainingGold.Value=0; TestFalse(TEXT("Emptied absent"),LHLootMarker::Lootable(Corpse));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWorldPropsPoolTest,"Lighthaven.Visual.Props.MeshEffectPool",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWorldPropsPoolTest::RunTest(const FString&)
{
    const FName Name=MakeUniqueObjectName(nullptr,UWorld::StaticClass(),TEXT("LHWorldPropsTest"));
    auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    auto& Context=GEngine->CreateNewWorldContext(EWorldType::Game);
    auto* World=UWorld::CreateWorld(EWorldType::Game,false,Name,GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init); Context.SetCurrentWorld(World);
    auto* Owner=World->SpawnActor<AActor>(); auto* FX=NewObject<ULHMeshEffects>(Owner); Owner->AddInstanceComponent(FX); FX->RegisterComponent();
    for(FName Kind:{FName("FireDart"),FName("FireImpact"),FName("StoneShard"),FName("StoneShatter"),FName("DustDevil"),FName("HealLight"),FName("MeleeSpark"),FName("SlashArc"),FName("ArrowFlight"),FName("ImpactPuff")})
    {
        TestTrue(*Kind.ToString(),FX->Emit(Kind,FVector::ZeroVector,FVector(100,0,0),.3f));
        TestEqual(TEXT("One active"),FX->ActiveCount(),1); FX->Advance(.4f); TestEqual(TEXT("Returns idle"),FX->ActiveCount(),0);
    }
    TestFalse(TEXT("Missing asset fallback"),FX->Emit("Unknown",FVector::ZeroVector,FVector::ZeroVector,1));
    for(int32 Index=0;Index<ULHMeshEffects::PoolCap;++Index) FX->Emit("FireImpact",FVector::ZeroVector,FVector::ZeroVector,1);
    TestFalse(TEXT("Bounded overflow"),FX->Emit("FireImpact",FVector::ZeroVector,FVector::ZeroVector,1));
    FX->Advance(2); TestEqual(TEXT("All returned idle"),FX->ActiveCount(),0);
    auto* Visual=NewObject<ULHPlayerVisualComponent>(Owner); Owner->AddInstanceComponent(Visual); Visual->RegisterComponent();
    FLHCharacterRecord Character; FLHItemInstance Item; Item.Id.InstanceId=FGuid::NewGuid(); Character.Inventory.Add(Item);
    FLHEquipmentBinding Binding; Binding.Slot=ELHEquipmentSlot::MainHand; Binding.Item=Item.Id; Character.Equipment.Add(Binding);
    for(FName Id:{FName("Item.RustedDirk"),FName("Item.GoblinBlade"),FName("Item.AshwoodFlatbow")})
    {
        Character.Inventory[0].Definition.Value=Id; Visual->Present(&Character,nullptr,0,0);
        TArray<UStaticMeshComponent*> Components; Owner->GetComponents(Components);
        TestTrue(*(Id.ToString()+TEXT(" world mesh")),Components.ContainsByPredicate([&](const auto* M){return M->GetStaticMesh()==ULHPropsCatalog::Mesh(Id);}));
    }
    {
        TGuardValue<TObjectPtr<ULHPropsAssets>> Missing(GetMutableDefault<ULHPropsCatalog>()->Assets,nullptr);
        Visual->Present(&Character,nullptr,0,0);
        TestTrue(TEXT("Missing weapon restores box"),Visual->Pieces().ContainsByPredicate([](const auto& P){return P && P->GetRecipe().Id==TEXT("Presentation.Player.Weapon") && !P->IsHidden();}));
    }
    auto* Combat=NewObject<ULHCombatComponent>(Owner); Owner->AddInstanceComponent(Combat); Combat->RegisterComponent();
    FX->Bind(Combat);
    const auto Before=Combat->GetCombatAttributes()->GetHealth(); const auto Random=Combat->GetCombatRandomState().GetCurrentSeed();
    for(FName Ability:ULHPropsCatalog::AbilityIds()) if(Ability!=TEXT("Spell.Light"))
    {
        FLHAttackEvent Event; Event.Ability.Value=Ability; Event.Identity.ActivationId=FGuid::NewGuid(); Event.ImpactSeconds=.3;
        Combat->OnAttackCommitted.Broadcast(Event);
        if(Ability==TEXT("Spell.FireDart") || Ability==TEXT("Spell.StoneShard") || Ability==TEXT("Attack.Ranged.Bow")) TestEqual(TEXT("Commit projectile"),FX->ActiveCount(),1);
        Combat->OnAttackFinished.Broadcast(Event,ELHAttackOutcome::ResolvedHit);
        TestTrue(*(Ability.ToString()+TEXT(" resolved cosmetic")),FX->ActiveCount()>0);
        FX->Advance(1); TestEqual(TEXT("Resolved idle"),FX->ActiveCount(),0);
        Combat->OnAttackCommitted.Broadcast(Event); Combat->OnAttackFinished.Broadcast(Event,ELHAttackOutcome::Cancelled);
        TestEqual(TEXT("Cancel removes flight"),FX->ActiveCount(),0);
    }
    Combat->RestoreLightRemainingSeconds(1);
    FX->TickComponent(.1f,LEVELTICK_All,nullptr);
    TestEqual(TEXT("Orb does not advance authority duration"),Combat->GetLightRemainingSeconds(),1.0);
    TArray<UStaticMeshComponent*> OrbComponents; Owner->GetComponents(OrbComponents);
    TestTrue(TEXT("Active light orb"),OrbComponents.ContainsByPredicate([](const auto* M){return M->GetStaticMesh()==ULHPropsCatalog::Mesh("Light") && M->IsVisible();}));
    Combat->RestoreLightRemainingSeconds(0); FX->TickComponent(.3f,LEVELTICK_All,nullptr);
    TestFalse(TEXT("Ended light orb"),OrbComponents.ContainsByPredicate([](const auto* M){return M->GetStaticMesh()==ULHPropsCatalog::Mesh("Light") && M->IsVisible();}));
    auto* Enemy=World->SpawnActor<ALHEnemyCharacter>();
    FLHAttackEvent Dust; Dust.Ability.Value=TEXT("Spell.DustDevil"); Dust.Target=Enemy->GetEntityId(FGuid{}); Dust.Identity.ActivationId=FGuid::NewGuid();
    Combat->OnAttackFinished.Broadcast(Dust,ELHAttackOutcome::ResolvedHit);
    TArray<UStaticMeshComponent*> DustComponents; Owner->GetComponents(DustComponents);
    const FVector Foot=Enemy->GetCapsuleComponent()->GetComponentLocation()-FVector(0,0,Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    TestTrue(TEXT("Dust uses target capsule foot"),DustComponents.ContainsByPredicate([&](const auto* M){return M->GetStaticMesh()==ULHPropsCatalog::Mesh("DustDevil") && M->IsVisible() && M->GetComponentLocation().Equals(Foot,.01f);})); FX->Advance(1);
    FLHSaveSnapshot View; auto& Area=View.World.Areas.AddDefaulted_GetRef(); auto& Loot=Area.Corpses.AddDefaulted_GetRef(); Loot.SourceLife=Enemy->GetLife(); Loot.bFinalized=true; Loot.RemainingItems.Add(Item);
    LHLootMarker::Present(Enemy,View);
    TArray<UStaticMeshComponent*> Badges; Enemy->GetComponents(Badges);
    TestFalse(TEXT("Living enemy has no badge"),Badges.ContainsByPredicate([](const auto* M){return M->GetFName()==TEXT("CorpseLootBadge");}));
    Enemy->MarkCorpse(); LHLootMarker::Present(Enemy,View); Enemy->GetComponents(Badges);
    TestTrue(TEXT("Lootable corpse has badge"),Badges.ContainsByPredicate([](const auto* M){return M->GetFName()==TEXT("CorpseLootBadge") && M->IsVisible();}));
    Loot.RemainingItems.Reset(); LHLootMarker::Present(Enemy,View);
    TestFalse(TEXT("Emptied corpse hides badge"),Badges.ContainsByPredicate([](const auto* M){return M->GetFName()==TEXT("CorpseLootBadge") && M->IsVisible();}));
    Enemy->Destroy();
    TestEqual(TEXT("Cosmetics leave health unchanged"),Combat->GetCombatAttributes()->GetHealth(),Before);
    TestEqual(TEXT("Cosmetics leave RNG unchanged"),Combat->GetCombatRandomState().GetCurrentSeed(),Random);
    Owner->Destroy(); GEngine->DestroyWorldContext(World); World->DestroyWorld(false); return true;
}
#endif
