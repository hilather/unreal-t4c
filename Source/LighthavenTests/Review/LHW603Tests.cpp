#include "Misc/AutomationTest.h"
#include "Framework/LHWave2Session.h"
#include "Framework/LHWave2Profile.h"
#include "Framework/LHWave2Closure.h"
#include "World/LHTravelSaveAdapter.h"
#include "Framework/LHPlayerState.h"
#include "Framework/LHPlayerController.h"
#include "Framework/LHCharacter.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Abilities/LHAbilityCatalog.h"
#include "AI/LHEncounterDirector.h"
#include "Framework/LHEnemyCharacter.h"
#include "World/LHWorldMarkers.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Data/Items/LHItemCatalog.h"
#include "Data/Encounters/LHEncounterCatalog.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "TimerManager.h"
#include "Abilities/LHResourceRecovery.h"
#include "Quests/LHQuests.h"
#include "Async/TaskGraphInterfaces.h"
#include "HAL/PlatformProcess.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/TargetPoint.h"
#include "EngineUtils.h"
#include "Components/PrimitiveComponent.h"
#include "Components/CapsuleComponent.h"
#include "StaticMeshCompiler.h"
#include "AssetCompilingManager.h"
#include "AI/NavigationSystemBase.h"
#include "UObject/UnrealType.h"
#include "UObject/Package.h"
#include "UObject/LinkerInstancingContext.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Framework/LHSessionSubsystem.h"
#include "Persistence/LHSaveSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "UI/LHUIWidgetHarness.h"
#include "UI/LHFrontendGameMode.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHW603TestsPrivate
{
constexpr auto Flags=EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter;
class FStorage : public ILHSaveStorage
{
public:
    TMap<FGuid,TArray<uint8>> Slots[2];
    int32 Writes=0; bool bFail=false, bFailReadback=false, bReadbackArmed=false;
    TArray<FGuid> Enumerate() override
    { TArray<FGuid> R; Slots[0].GetKeys(R); for (const auto& K:Slots[1]) R.AddUnique(K.Key); return R; }
    bool Read(const FGuid& Id,int32 Slot,TArray<uint8>& Bytes,FLHSaveError& E) override
    { if (bReadbackArmed) { bReadbackArmed=false; E={ELHSaveReason::IoFailure,TEXT("Injected readback failure")}; return false; } if (auto* B=Slots[Slot].Find(Id)) { Bytes=*B; return true; } E={ELHSaveReason::NotFound,TEXT("Missing generation")}; return false; }
    void Write(const FGuid& Id,int32 Slot,TArray<uint8> Bytes,TFunction<void(bool)> Done) override
    { ++Writes; if (!bFail) Slots[Slot].Add(Id,MoveTemp(Bytes)); bReadbackArmed=bFailReadback; Done(!bFail); }
};
struct FRuntime
{
    UGameInstance* Instance=nullptr;
    ULocalPlayer* Local=nullptr;
    UWorld* World=nullptr;
    ALHPlayerState* State=nullptr;
    ALHPlayerController* Controller=nullptr;
    TSharedPtr<FLHSaveStore> Store;
    TSharedPtr<FLHWave2Session> Session;
    TSharedPtr<FLHUIPresenter> UI;
    explicit FRuntime(TSharedRef<FStorage> Disk,bool OwnedSession=false)
    {
        const FName Name=MakeUniqueObjectName(nullptr,UWorld::StaticClass(),TEXT("LHWave2World"),EUniqueObjectNameOptions::GloballyUnique);
        const auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).RequiresHitProxies(false)
            .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false);
        auto& Context=GEngine->CreateNewWorldContext(EWorldType::Game);
        World=UWorld::CreateWorld(EWorldType::Game,false,Name,GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
        Context.SetCurrentWorld(World); World->InitializeActorsForPlay(FURL());
        Controller=World->SpawnActor<ALHPlayerController>(); Controller->SetAsLocalPlayerController();
        State=World->SpawnActor<ALHPlayerState>(); Controller->SetPlayerState(State); Controller->InitInputSystem();
        if (OwnedSession)
        {
            Instance=NewObject<UGameInstance>(GEngine); Instance->AddToRoot();
            Instance->InitializeStandalone(MakeUniqueObjectName(nullptr,UWorld::StaticClass(),TEXT("LHSessionDummy")));
            auto* Dummy=Instance->GetWorld(); auto* OwnedContext=GEngine->GetWorldContextFromWorld(Dummy);
            Dummy->DestroyWorld(false); GEngine->DestroyWorldContext(World);
            OwnedContext->SetCurrentWorld(World); World->SetGameInstance(Instance);
            Local=NewObject<ULocalPlayer>(GEngine); Instance->AddLocalPlayer(Local,FPlatformUserId::CreateFromInternalId(0));
            Local->PlayerController=Controller; Controller->Player=Local;
            Store=Instance->GetSubsystem<ULHSaveSubsystem>()->GetStore();
            Session=Instance->GetSubsystem<ULHSessionSubsystem>()->Session();
            // Test adapters load generated worlds synchronously. Prevent OpenLevel
            // while retaining the real initialized subsystem and death bridge.
            Session->Travel=[](){}; Session->Exit=[](){};
        }
        else { Store=MakeShared<FLHSaveStore>(Disk); Session=MakeShared<FLHWave2Session>(Store.ToSharedRef()); }
        Session->Bind(State);
        UI=MakeShared<FLHUIPresenter>(*Session,*Session,*Session);
    }
    ~FRuntime()
    {
        UI.Reset(); Session.Reset(); if(Instance) { Controller->Player=nullptr; Local->PlayerController=nullptr; Instance->RemoveLocalPlayer(Local); Instance->Shutdown(); Instance->RemoveFromRoot(); } if(World) { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); }
    }
    FLHCommandResult Create()
    {
        UI->Open(ELHUIScreen::Creation);
        TArray<FLHContentId> Appearance;
        for (const TCHAR* Key:{TEXT("Presentation.Player.Body.A"),TEXT("Presentation.Player.Hair.Cropped"),TEXT("Presentation.Player.Skin.LightWarm"),TEXT("Presentation.Player.Outfit.StarterLinen")})
        { FLHContentId I; I.Value=Key; Appearance.Add(I); }
        UI->EditCreation(TEXT("Same name"),Appearance,LHWave2::PrototypeAnswers()); UI->Roll(false);
        return UI->ConfirmCreation();
    }
    void Flush()
    {
        Session->Flush();
        if(Instance && Session->HasCharacter())
        {
            const double Deadline=FPlatformTime::Seconds()+10;
            while(Store->IsWriting(Session->Snapshot().Header.CharacterId) && FPlatformTime::Seconds()<Deadline)
            { FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread); FPlatformProcess::Sleep(0.001f); }
            FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
        }
    }
    void EarnAllocationPoints()
    {
        check(State->GetCharacterAuthority()->Authority().GrantExperience(1000)==ELHCommandReason::None);
    }
    bool Restore(FLHCharacterId Id,bool Ack=false)
    { UI->Open(ELHUIScreen::Characters); UI->SelectProfile(Id); return UI->Continue(Ack); }
};
bool Equal(const FLHSaveSnapshot& A,const FLHSaveSnapshot& B)
{
    TArray<uint8> X,Y; FLHSaveError E; return LHSave::Encode(A,X,E) && LHSave::Encode(B,Y,E) && X==Y;
}
FLHAttributeBlock Points()
{
    FLHAttributeBlock P; P.Strength=LHWave2::PrototypeInteger(1); P.Endurance=P.Agility=P.Intelligence=P.Wisdom=LHWave2::PrototypeInteger(0); return P;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHW603References,"Lighthaven.Review.W603.References",LHW603TestsPrivate::Flags)
bool FLHW603References::RunTest(const FString&)
{
    using namespace LHW603TestsPrivate;
    FRuntime R(MakeShared<FStorage>()); R.Create(); R.Flush();
    const auto Good=R.Session->Snapshot();
    FLHEntityId Player; Player.RunId=Good.World.RunId; Player.Area=Good.Character.ActiveEntrance.Area; Player.InstanceId=Good.Header.CharacterId.Value;
    auto Decode=[&](const FLHSaveSnapshot& S,bool Expected) {
        TArray<uint8> Bytes; FLHSaveError E; FLHSaveSnapshot Out=Good;
        if(!TestTrue(TEXT("real encode"),LHSave::Encode(S,Bytes,E))) return;
        TestEqual(TEXT("production decode"),LHSave::Decode(Bytes,S.Header.CharacterId,FLHWave2Session::Compatibility(),Out,E),Expected);
        if(!Expected) { TestEqual(TEXT("semantic rejection"),E.Reason,ELHSaveReason::InvalidSnapshot); TestTrue(TEXT("output unchanged"),Equal(Out,Good)); }
    };
    Decode(Good,true);
    auto Bad=Good; FLHContentId Unknown; Unknown.Value=TEXT("Spell.DoesNotExist"); Bad.Character.LearnedSpells.Add(Unknown); Decode(Bad,false);
    // Newest integrity-valid generation is semantically bad: production A/B fallback.
    auto FallbackDisk=MakeShared<FStorage>(); TArray<uint8> Bytes; FLHSaveError FallbackError;
    LHSave::Encode(Good,Bytes,FallbackError); FallbackDisk->Slots[0].Add(Good.Header.CharacterId.Value,Bytes);
    ++Bad.Header.TransactionSequence; LHSave::Encode(Bad,Bytes,FallbackError); FallbackDisk->Slots[1].Add(Good.Header.CharacterId.Value,Bytes);
    FLHSaveStore Fallback(FallbackDisk); FLHSaveSnapshot Recovered;
    TestTrue(TEXT("semantic A/B fallback"),Fallback.Load(Good.Header.CharacterId,FLHWave2Session::Compatibility(),Recovered,FallbackError));
    TestTrue(TEXT("fallback selects original"),Equal(Recovered,Good));
    Bad=Good; FLHLearnedSkill Skill; Skill.Skill.Value=TEXT("Skill.DoesNotExist"); Skill.TrainedValue=LHWave2::PrototypeInteger(999); Bad.Character.LearnedSkills.Add(Skill); Decode(Bad,false);
    Bad=Good; Bad.Character.LearnedSkills[0].Skill.Value=TEXT("skill.attack"); Decode(Bad,false);
    Bad=Good; FLHBossRecord Boss; Boss.Boss.Value=TEXT("Enemy.Balork"); Boss.bDefeated=true; Boss.UniqueClaim.Value=FGuid(1,3,5,7); Bad.World.Bosses.Add(Boss); Decode(Bad,false);
    Bad=Good; Boss.bDefeated=false; FLHSaveError E; LHSave::BossUniqueRewardId(Bad.World.RunId,Boss.Boss,Boss.UniqueClaim,E); Bad.World.Bosses.Add(Boss); Bad.World.ClaimedUniqueRewards.Add(Boss.UniqueClaim); Decode(Bad,false);
    Bad=Good; FLHCooldownRecord Cooldown; Cooldown.Owner=Player; Cooldown.Ability.Value=TEXT("Spell.DoesNotExist"); Cooldown.RemainingSeconds=LHWave2::PrototypeNumber(1); Bad.Session.Cooldowns.Add(Cooldown); Decode(Bad,false);
    Bad=Good; FLHQuestRecord Quest; Quest.Quest.Value=TEXT("Quest.DoesNotExist"); Quest.Stage=TEXT("HuntRats"); Quest.EligibleKillCount=LHWave2::PrototypeInteger(0); Bad.World.Quests.Add(Quest); Decode(Bad,false);
    Bad=Good; Boss={}; Boss.Boss.Value=TEXT("Enemy.Rat"); Bad.World.Bosses.Add(Boss); Decode(Bad,false);
    Bad=Good; FLHObjectRecord Object; Object.Id=Player; Object.Id.InstanceId=FGuid::NewGuid(); Object.Definition.Value=TEXT("Object.PrototypeContainer");
    auto Contained=Good.Character.Inventory[0]; Contained.Id.InstanceId=FGuid::NewGuid(); Contained.Definition.Value=TEXT("Item.DoesNotExist"); Object.RemainingItems.Add(Contained); Bad.World.Areas[0].Objects.Add(Object); Decode(Bad,false);
    // Real reward settlement with the production quest observer, then another life.
    auto S=Good; S.Character.ActiveEntrance=LHWorld::Registry()[4].Entrances[0].Id; FString Error;
    TestTrue(TEXT("populate B4"),FLHWave2Session::PopulateEncounterCheckpoint(S,Error));
    auto& Area=S.World.Areas.Last(); auto* Enemy=Area.Encounters.FindByPredicate([](const auto& X){return X.Definition.Value==TEXT("Enemy.Balork");});
    if(!TestNotNull(TEXT("Balork"),Enemy)) return false;
    FLHKillFacts Facts; Facts.bKillerIsPlayer=true; Facts.Area=Area.Area; Facts.Life=Enemy->Life; Facts.Enemy=Enemy->Definition;
    FLHQuestKillObserver Observer; ILHKillObserver* Observers[]={&Observer};
    const auto* Row=LHEnemyData::Find(Facts.Enemy);
    TestEqual(TEXT("first kill"),LHRewards::SettleKill(S,Facts,Row->Reward,LHWave2::PrototypeProfile(),Observers),ELHCommandReason::None); Decode(S,true);
    TArray<FLHSpawnLifeId> Lives;
    TestEqual(TEXT("later generation"),LHRewards::AdvanceRespawns(S.World.Areas.Last(),S.World.RunId,901,[](const FGuid&){return true;},[](const FLHContentId& Id){return LHEnemyData::Find(Id)->Health;},Lives),ELHCommandReason::None);
    ++Facts.Life.LifeGeneration;
    TestEqual(TEXT("later kill settles"),LHRewards::SettleKill(S,Facts,Row->Reward,LHWave2::PrototypeProfile(),Observers),ELHCommandReason::None);
    TestEqual(TEXT("one unique claim"),S.World.ClaimedUniqueRewards.Num(),1); Decode(S,true);
    TestEqual(TEXT("duplicate life rejected"),LHRewards::SettleKill(S,Facts,Row->Reward,LHWave2::PrototypeProfile(),Observers),ELHCommandReason::InvalidLifeState);
    return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHW603Travel,"Lighthaven.Review.W603.TravelCooldownAndResult",LHW603TestsPrivate::Flags)
bool FLHW603Travel::RunTest(const FString&)
{
    using namespace LHW603TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State);
    auto* Pawn=R.World->SpawnActor<ALHCharacter>(); R.State->InitializeAvatar(Pawn);
    auto S=R.Session->Snapshot(); S.Character.ActiveEntrance=LHWorld::Registry()[1].Entrances[0].Id;
    TestTrue(TEXT("install B1"),R.Session->InstallTravel(S));
    // StartEncounters restores the player before attempting any enemy materialization.
    R.Session->StartEncounters(); R.Flush();
    auto* C=R.State->GetCombatComponent(); TMap<FName,double> Remaining; Remaining.Add(TEXT("Attack.Melee.Basic"),1.25); C->RestoreCooldownMap(Remaining);
    const auto& Edge=LHWorld::Registry()[1].Portals[0];
    auto* Portal=R.World->SpawnActor<ALHPortal>(); Portal->Source=Edge.Source; Portal->Destination=Edge.Destination; Portal->PortalId=Edge.Portal.InstanceId;
    Pawn->SetActorLocation(Portal->GetActorLocation());
    FLHTravelSaveAdapter::FHooks H;
    H.Freeze=[&](bool F){R.Session->FreezeWorldTravel(F);};
    H.Capture=[&](FLHSaveSnapshot& Out,FString& E){return R.Session->CaptureTravel(Out,E);};
    H.Durable=[&](const FLHSaveSnapshot& Out){TestTrue(TEXT("durable install"),R.Session->InstallTravel(Out));};
    H.Load=[](const FLHAreaDefinition&,uint64){};
    H.Install=[&](const FLHSaveSnapshot& Out,const FLHEntranceDefinition&,FString&){return R.Session->InstallTravel(Out) && R.Session->StartEncounters();};
    H.Restore=[](const FLHSaveSnapshot&,const FLHEntranceDefinition&,uint64){};
    FLHTravelSaveAdapter Adapter(R.Store.ToSharedRef(),FLHWave2Session::Compatibility(),MoveTemp(H));
    R.Session->RequestWorldTravel=[&](const FLHRequestTravelRequest& Q,FString& E){return Adapter.Travel().Begin(Q,E);};
    FLHRequestTravelRequest Q; Q.Request.Epoch=S.Session.RequestEpoch; Q.Request.Value=FGuid::NewGuid(); Q.Portal=Portal->Materialize(S.World.RunId); Q.Destination=Edge.Destination;
    auto Result=R.Session->Execute(Q); TestEqual(TEXT("accepted"),Result.Disposition,ELHCommandDisposition::Accepted); TestEqual(TEXT("accepted reason"),Result.Reason,ELHCommandReason::None); TestEqual(TEXT("correlation"),Result.Request.Value,Q.Request.Value);
    Adapter.Travel().OnDestinationLoaded(Adapter.Travel().GetToken(),true,Q.Destination,TEXT("")); R.Flush();
    TestEqual(TEXT("destination cooldown"),C->GetCooldownMap().FindRef(TEXT("Attack.Melee.Basic")),1.25);
    FLHSaveSnapshot Loaded; FLHSaveError E; TestTrue(TEXT("destination reload"),R.Store->Load(S.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,E));
    TestTrue(TEXT("reloaded install"),R.Session->InstallTravel(Loaded)); R.Session->StartEncounters();
    TestEqual(TEXT("reloaded remainder"),C->GetCooldownMap().FindRef(TEXT("Attack.Melee.Basic")),1.25);
    for(int32 Tick=0;Tick<20;++Tick) R.World->Tick(LEVELTICK_All,0.1f); FString CaptureError; TestTrue(TEXT("capture after active time"),R.Session->CaptureTravel(Loaded,CaptureError));
    const auto& Reverse=LHWorld::Registry()[0].Portals[0];
    Portal->Source=Reverse.Source; Portal->Destination=Reverse.Destination; Portal->PortalId=Reverse.Portal.InstanceId;
    Q.Request.Value=FGuid::NewGuid(); Q.Portal=Portal->Materialize(S.World.RunId); Q.Destination=Reverse.Destination;
    TestEqual(TEXT("return travel accepted"),R.Session->Execute(Q).Disposition,ELHCommandDisposition::Accepted);
    Adapter.Travel().OnDestinationLoaded(Adapter.Travel().GetToken(),true,Q.Destination,TEXT("")); R.Flush();
    TestEqual(TEXT("no stale return"),C->GetCooldownMap().FindRef(TEXT("Attack.Melee.Basic")),0.0);
    R.Session->RequestWorldTravel=nullptr; Result=R.Session->Execute(Q); TestEqual(TEXT("reject correlation"),Result.Request.Value,Q.Request.Value); TestEqual(TEXT("reject reason"),Result.Reason,ELHCommandReason::Busy);
    return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHW603Corpse,"Lighthaven.Review.W603.LiveGenerationLookup",LHW603TestsPrivate::Flags)
bool FLHW603Corpse::RunTest(const FString&)
{
    using namespace LHW603TestsPrivate;
    FRuntime R(MakeShared<FStorage>());
    auto* D=R.World->GetSubsystem<ULHEncounterDirector>(); D->RunId=FGuid::NewGuid();
    const auto& Slot=LHWorld::Registry()[1].Spawns[0];
    FLHSpawnLifeId Life; Life.Area=LHWorld::Registry()[1].Id; Life.SpawnSlot=Slot.SpawnId;
    const auto* Row=LHEnemyData::Find(Slot.Enemy); FString Error;
    auto* Corpse=R.World->SpawnActor<ALHEnemyCharacter>();
    TestTrue(TEXT("old runtime life"),Corpse->ApplyRuntimeSpec(Row->Runtime,Life,Row->Health,Error)); Corpse->MarkCorpse();
    ++Life.LifeGeneration; auto* Living=R.World->SpawnActor<ALHEnemyCharacter>();
    TestTrue(TEXT("new runtime life"),Living->ApplyRuntimeSpec(Row->Runtime,Life,Row->Health,Error));
    // Install both registered lives in corpse-first order without requiring nav.
    auto* Property=FindFProperty<FArrayProperty>(ULHEncounterDirector::StaticClass(),TEXT("Enemies"));
    FScriptArrayHelper Registry(Property,Property->ContainerPtrToValuePtr<void>(D));
    auto* Inner=CastFieldChecked<FObjectPropertyBase>(Property->Inner);
    Inner->SetObjectPropertyValue(Registry.GetRawPtr(Registry.AddValue()),Corpse);
    Inner->SetObjectPropertyValue(Registry.GetRawPtr(Registry.AddValue()),Living);
    TestEqual(TEXT("combat resolves living generation"),D->FindByEntity(Living->GetEntityId(D->RunId)),Living);
    auto Old=Life; --Old.LifeGeneration; TestEqual(TEXT("corpse still addressable by life"),D->FindByLife(Old),Corpse);
    // Real collision: the old life lies on the next life's attack segment.
    Corpse->SetActorLocation(FVector(75,0,100));
    Living->SetActorLocation(FVector(150,0,100));
    auto* Attacker=R.World->SpawnActor<ALHEnemyCharacter>();
    auto AttackLife=Life; AttackLife.SpawnSlot=FGuid::NewGuid();
    TestTrue(TEXT("attacker runtime"),Attacker->ApplyRuntimeSpec(Row->Runtime,AttackLife,Row->Health,Error));
    Attacker->SetActorLocation(FVector(0,0,100));
    for(auto* Actor : {Corpse,Living,Attacker}) Actor->GetCapsuleComponent()->RecreatePhysicsState();
    FHitResult Hit; FCollisionQueryParams Params; Params.AddIgnoredActor(Attacker); Params.AddIgnoredActor(Living);
    TestTrue(TEXT("corpse remains cursor targetable"),R.World->LineTraceSingleByChannel(Hit,Attacker->GetActorLocation(),Living->GetActorLocation(),ECC_Visibility,Params) && Hit.GetActor()==Corpse);
    TestEqual(TEXT("retained corpse does not block next-life attack"),Attacker->GetCombatComponent()->ValidateAttack(Living->GetCombatComponent()),ELHCommandReason::None);
    Corpse->Destroy();
    auto* Obstacle=R.World->SpawnActor<ALHEnemyCharacter>(); auto ObstacleLife=Life; ObstacleLife.SpawnSlot=FGuid::NewGuid();
    TestTrue(TEXT("living obstacle runtime"),Obstacle->ApplyRuntimeSpec(Row->Runtime,ObstacleLife,Row->Health,Error));
    Obstacle->SetActorLocation(FVector(75,0,100)); Obstacle->GetCapsuleComponent()->RecreatePhysicsState();
    TestEqual(TEXT("living solid still blocks attack sight"),Attacker->GetCombatComponent()->ValidateAttack(Living->GetCombatComponent()),ELHCommandReason::OutOfRange);
    return !HasAnyErrors();
}
#endif
