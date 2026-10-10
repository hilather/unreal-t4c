#include "Abilities/LHLightEffect.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/LHWave2Session.h"
#include "Framework/LHWave2Profile.h"
#include "Framework/LHPlayerState.h"
#include "Framework/LHEnemyCharacter.h"
#include "Framework/LHArrivalReview.h"
#include "Abilities/LHAbilityCatalog.h"
#include "Abilities/LHItemUse.h"
#include "Abilities/LHResourceRecovery.h"
#include "AI/LHEncounterDirector.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Data/Items/LHItemCatalog.h"
#include "Quests/LHQuests.h"
#include "Services/LHServiceAuthority.h"
#include "Framework/LHPlayerController.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "World/LHWorldMarkers.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
namespace LHStage1SessionPrivate
{
bool Same(const FLHEntityId& A,const FLHEntityId& B)
{ return A.RunId==B.RunId && A.Area.Content.Value==B.Area.Content.Value && A.InstanceId==B.InstanceId; }
const FLHCombatItemData* Item(const FLHContentId& Id)
{ const auto* Row=LHItemData::Find(Id); return Row?&Row->Combat:nullptr; }
FLHNumber Health(const FLHContentId& Id)
{ const auto* Row=LHEnemyData::Find(Id); return Row?Row->Health:FLHNumber{}; }
FString EnemyPrefix(const FLHSpawnLifeId& Life)
{ return TEXT("RNG.Enemy.S")+Life.SpawnSlot.ToString(EGuidFormats::Digits)+TEXT("."); }
FName EnemyStream(const FLHSpawnLifeId& Life)
{ return FName(*(EnemyPrefix(Life)+FString::Printf(TEXT("Life%lld"),Life.LifeGeneration))); }
void CaptureEnemy(ALHEnemyCharacter& Enemy,FLHSaveSnapshot& S)
{
    const auto Id=Enemy.GetEntityId(S.World.RunId);
    LHAbilities::CaptureCooldowns(*Enemy.GetCombatComponent(),Id,S.Session.Cooldowns);
    const FString Prefix=EnemyPrefix(Enemy.GetLife());
    S.Session.GameplayRng.RemoveAll([&](const auto& R){return R.StreamId.ToString().StartsWith(Prefix);});
    if (!Enemy.IsAlive()) return;
    FLHRngState R; R.StreamId=EnemyStream(Enemy.GetLife()); R.Algorithm=TEXT("UE.FRandomStream"); R.AlgorithmRevision=1;
    const uint32 Seed=uint32(Enemy.GetCombatComponent()->GetCombatRandomState().GetCurrentSeed());
    for(int32 B=0;B<4;++B) R.State.Add(uint8(Seed>>(B*8))); S.Session.GameplayRng.Add(R);
}
void RestoreEnemy(ALHEnemyCharacter& Enemy,const FLHSaveSnapshot& S)
{
    const auto* Rng=S.Session.GameplayRng.FindByPredicate([&](const auto& R){return R.StreamId==EnemyStream(Enemy.GetLife());});
    // A new generation has its constructor seed and no previous-life cooldown.
    if (!Rng || Rng->State.Num()!=4) return;
    uint32 Seed=0; for(int32 B=0;B<4;++B) Seed|=uint32(Rng->State[B])<<(B*8);
    auto* C=Enemy.GetCombatComponent(); C->SetCombatRandomState(FRandomStream(int32(Seed)));
    TMap<FName,double> Remaining;
    for(const auto& R:S.Session.Cooldowns) if(Same(R.Owner,Enemy.GetEntityId(S.World.RunId))) Remaining.Add(R.Ability.Value,R.RemainingSeconds.Value);
    C->RestoreCooldownMap(Remaining);
}
FLHAreaRecord* Area(FLHSaveSnapshot& S)
{ return S.World.Areas.FindByPredicate([&](const auto& A){return LHWorld::SameArea(A.Area,S.Character.ActiveEntrance.Area);}); }
}
FLHEntityId FLHWave2Session::PlayerEntity() const
{
    FLHEntityId Id; Id.RunId=Complete.World.RunId; Id.Area=Complete.Character.ActiveEntrance.Area;
    Id.InstanceId=Complete.Header.CharacterId.Value; return Id;
}
bool FLHWave2Session::SyncResources()
{
    if (!Authority() || !HasCharacter()) return false;
    Authority()->Export(Complete);
    if (Owner->GetCombatAvatar())
    {
        auto* C=Owner->GetCombatComponent(); auto* A=C->GetCombatAttributes();
        if (!FMath::IsFinite(A->GetHealth()) || !FMath::IsFinite(A->GetMana())) return false;
        Complete.Character.CurrentHealth.Value=A->GetHealth(); Complete.Character.CurrentMana.Value=A->GetMana();
        Complete.Session.ManaRegenFractionalSeconds=LHWave2::PrototypeNumber(C->ManaRegenFractionalSeconds);
        LHAbilities::CaptureCooldowns(*C,PlayerEntity(),Complete.Session.Cooldowns);
        auto* Rng=Complete.Session.GameplayRng.FindByPredicate([](const auto& R){return R.StreamId==TEXT("RNG.Combat");});
        if (Rng)
        { const uint32 Seed=uint32(C->GetCombatRandomState().GetCurrentSeed()); Rng->State.Reset(); for(int32 B=0;B<4;++B) Rng->State.Add(uint8(Seed>>(B*8))); }
    }
    auto* Light=Owner->GetCombatComponent();
    // Lethal settlement can run inside GAS publication. Capture Light only after
    // the action finishes; Flush already waits for the complete combat boundary.
    if (!Light->IsActionPending() && !Light->IsPublishingActionEvents())
    {
        if (bDeadAwaitingRespawn) Light->RestoreLightRemainingSeconds(0);
        if (!LHAbilities::CaptureLightEffect(*Light,PlayerEntity(),Complete.Session)) return false;
    }
    if (Director.IsValid()) if (auto* A=LHStage1SessionPrivate::Area(Complete)) Director->CaptureLive(*A);
    if (Director.IsValid()) for (TActorIterator<ALHEnemyCharacter> It(Owner->GetWorld());It;++It)
        if (!It->IsCorpse() && LHWorld::SameArea(It->GetLife().Area,Complete.Character.ActiveEntrance.Area)) LHStage1SessionPrivate::CaptureEnemy(**It,Complete);
    return Authority()->Import(Complete)==ELHCommandReason::None;
}
bool FLHWave2Session::AcceptBoundary(FLHSaveSnapshot&& Next,bool InstallResources)
{
    if (!Authority()) return false;
    auto Check=*Authority(); if (Check.Import(Next)!=ELHCommandReason::None) return false;
    *Authority()=MoveTemp(Check); Complete=MoveTemp(Next);
    if (InstallResources && !InstallDerived()) return false;
    bSaveQueued=true; Message=TEXT("Saving completed action…"); return true;
}
FLHCommandResult FLHWave2Session::Persist(const FLHRequestId& Id,FName Name,const UScriptStruct* Type,const void* Payload,TFunctionRef<ELHCommandReason(FLHSaveSnapshot&)> Domain)
{
    FLHCommandResult R; R.Request=Id; FString Digest;
    // Replay does not re-run spatial, life, or resource checks.
    if (!LHSave::BeginRequest(Complete,Name,Type,Payload,R,Digest))
    { if (R.Reason==ELHCommandReason::Busy) Message=TEXT("Durable request capacity reached; further transactions require an approved epoch rollover policy."); return R; }
    if (IsTransactionBlocked() || !Authority()) { R.Reason=ELHCommandReason::Busy; return R; }
    auto* Combat=Owner->GetCombatComponent();
    if (Combat->IsActionPending() || Combat->IsPublishingActionEvents()) { R.Reason=ELHCommandReason::ActiveAction; return R; }
    if (!SyncResources()) { R.Reason=ELHCommandReason::InvalidRequest; return R; }
    auto Next=Complete; R.Reason=Domain(Next); if (R.Reason!=ELHCommandReason::None) return R;
    auto Check=*Authority(); R.Reason=Check.Import(Next); if (R.Reason!=ELHCommandReason::None) return R;
    LHSave::CommitRequest(Next,Id,Digest,R);
    if (R.Disposition==ELHCommandDisposition::Accepted && !R.bReplay && !AcceptBoundary(MoveTemp(Next),true))
    { R.Disposition=ELHCommandDisposition::Rejected; R.Reason=ELHCommandReason::InvalidRequest; }
    return R;
}
bool FLHWave2Session::Spatial(AActor* Target,double Range) const
{
    auto* Pawn=Owner.IsValid()?Owner->GetCombatAvatar():nullptr;
    if (!Pawn || !IsValid(Target) || Pawn->GetWorld()!=Target->GetWorld() || FVector::Dist(Pawn->GetActorLocation(),Target->GetActorLocation())>Range) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(LHSessionInteraction),false,Pawn); Params.AddIgnoredActor(Target);
    return !Pawn->GetWorld()->LineTraceTestByChannel(Pawn->GetActorLocation(),Target->GetActorLocation()+FVector(0,0,60),ECC_Visibility,Params);
}
ALHInteractableMarker* FLHWave2Session::ResolveNpc(const FLHEntityId& Id) const
{
    if (!Owner.IsValid() || Id.RunId!=Complete.World.RunId || !LHWorld::SameArea(Id.Area,Complete.Character.ActiveEntrance.Area)) return nullptr;
    ALHInteractableMarker* Found=nullptr;
    for (TActorIterator<ALHInteractableMarker> It(Owner->GetWorld());It;++It)
        if (LHStage1SessionPrivate::Same(It->Materialize(Complete.World.RunId),Id))
        { if (Found) return nullptr; Found=*It; }
    return Found;
}
FLHCommandResult FLHWave2Session::Execute(const FLHInteractRequest& Q)
{
    return Persist(Q.Request,TEXT("Interact"),Q.StaticStruct(),&Q,[&](FLHSaveSnapshot& Next){
        auto* Npc=ResolveNpc(Q.Target); if (!Npc) return ELHCommandReason::NotFound;
        auto* Pawn=Owner->GetCombatAvatar(); if (!Pawn) return ELHCommandReason::InvalidLifeState;
        const auto Profile=LHWave2::PrototypeProfile(); FLHInteractContext Context;
        Context.Npc=Npc->DefinitionId; Context.Entity=Npc->Materialize(Complete.World.RunId); Context.Profile=&Profile;
        Context.DistanceCm=FVector::Dist(Pawn->GetActorLocation(),Npc->GetActorLocation()); Context.bLineOfSight=Spatial(Npc,250);
        return LHQuests::ExecuteInteract(Next,Context,Q);
    });
}
FLHCommandResult FLHWave2Session::Execute(const FLHUseItemRequest& Q)
{
    return Persist(Q.Request,TEXT("UseItem"),Q.StaticStruct(),&Q,[&](FLHSaveSnapshot& Next){
        LHAbilities::FLHUseItemContext Context; Context.Snapshot=Next; Context.ItemLookup=LHStage1SessionPrivate::Item;
        Context.Owner=PlayerEntity(); Context.MaximumMana=Owner->GetCombatComponent()->GetCombatAttributes()->GetMaxMana();
        Context.bAlive=Owner->GetCombatComponent()->IsAlive();
        const auto Reason=LHAbilities::ExecuteUseItem(Context,Q); if(Reason==ELHCommandReason::None) Next=MoveTemp(Context.Snapshot); return Reason;
    });
}
FLHCommandResult FLHWave2Session::Execute(const FLHTakeLootRequest& Q)
{
    return Persist(Q.Request,TEXT("TakeLoot"),Q.StaticStruct(),&Q,[&](FLHSaveSnapshot& Next){
        if (!Director.IsValid()) return ELHCommandReason::NotFound;
        auto* Area=LHStage1SessionPrivate::Area(Next); if (!Area) return ELHCommandReason::NotFound;
        const auto* Corpse=Area->Corpses.FindByPredicate([&](const auto& C){return LHStage1SessionPrivate::Same(C.Container,Q.Container);});
        ALHEnemyCharacter* Actor=nullptr;
        if (Corpse) for (TActorIterator<ALHEnemyCharacter> It(Owner->GetWorld());It;++It)
            if (It->IsCorpse() && LHAI::SameLife(It->GetLife(),Corpse->SourceLife)) { Actor=*It; break; }
        if (!Actor || !Actor->IsCorpse()) return ELHCommandReason::NotFound;
        if (!Spatial(Actor,200)) return ELHCommandReason::OutOfRange;
        return LHRewards::ExecuteTakeLoot(Next,LHWave2::PrototypeProfile(),Q);
    });
}
FLHCommandResult FLHWave2Session::Execute(const FLHUseAbilityRequest& Q)
{
    FLHCommandResult R; R.Request=Q.Request;
    if (IsBlocked() || bGameplayPaused || !Owner.IsValid()) { R.Reason=ELHCommandReason::Busy; return R; }
    if (!Q.Request.Value.IsValid() || Q.Request.Epoch!=Complete.Session.RequestEpoch) return R;
    const FString Digest=LHSave::RequestDigest(TEXT("UseAbility"),Q.StaticStruct(),&Q);
    if (const auto* Previous=RuntimeAbilities.Find(Q.Request.Value))
    {
        if (Previous->Key!=Digest) { R.Reason=ELHCommandReason::ReusedRequestId; return R; }
        R=Previous->Value; R.bReplay=true; return R;
    }
    if (RuntimeAbilities.Num()>=4096) { R.Reason=ELHCommandReason::Busy; Message=TEXT("Action request capacity reached; save and reload before issuing more attacks."); return R; }
    LHAbilities::FLHUseAbilityContext Context; Context.Source=Owner->GetCombatComponent(); Context.Snapshot=Snapshot();
    Context.SourceId=PlayerEntity(); Context.ItemLookup=LHStage1SessionPrivate::Item;
    Context.Combat=LHWave2::PrototypeProfile().Rules.Combat;
    auto* Target=Director.IsValid()?Director->FindByEntity(Q.Target):nullptr;
    if (Target) { Context.Target=Target->GetCombatComponent(); Context.TargetId=Target->GetEntityId(Complete.World.RunId); }
    if (LHStage1SessionPrivate::Same(Q.Target,Context.SourceId))
    { Context.Target=Context.Source; Context.TargetId=Context.SourceId; Context.bTargetFriendly=true; }
    R.Reason=LHAbilities::ExecuteUseAbility(Context,Q);
    if (R.Reason==ELHCommandReason::None)
    { R.Disposition=ELHCommandDisposition::Accepted; RuntimeAbilities.Add(Q.Request.Value,{Digest,R}); }
    // D16: no receipt, sequence advance, or save per activation.
    return R;
}
bool FLHWave2Session::PopulateEncounterCheckpoint(FLHSaveSnapshot& Snapshot,FString& Error)
{
    const auto* Definition=LHWorld::FindArea(Snapshot.Character.ActiveEntrance.Area);
    if (!Definition) { Error=TEXT("Encounter area missing from registry"); UE_LOG(LogTemp,Warning,TEXT("LH populate refused: %s"),*Error); return false; }
    if (Definition->Spawns.IsEmpty()) return true;
    auto Next=Snapshot;
    auto* Area=LHStage1SessionPrivate::Area(Next);
    if (!Area) { FLHAreaRecord New; New.Area=Definition->Id; Next.World.Areas.Add(New); Area=&Next.World.Areas.Last(); }
    const auto Reason=LHRewards::PopulateArea(*Area,*Definition,LHStage1SessionPrivate::Health);
    if (Reason!=ELHCommandReason::None) { Error=FString::Printf(TEXT("PopulateArea area=%s reason=%d"),*Definition->Id.Content.Value.ToString(),int32(Reason)); UE_LOG(LogTemp,Warning,TEXT("LH populate refused: %s"),*Error); return false; }
    for (auto& E:Area->Encounters)
    {
        FLHSaveError SaveError;
        if (!LHSave::EnemyLifeRewardId(Next.World.RunId,E.Life,E.KillReward,SaveError))
        { Error=SaveError.Detail; UE_LOG(LogTemp,Warning,TEXT("LH populate refused marker=%s: %s"),*E.Life.SpawnSlot.ToString(),*Error); return false; }
    }
    Snapshot=MoveTemp(Next); return true;
}
bool FLHWave2Session::StartEncounters()
{
    if (!Owner.IsValid() || !Owner->GetCombatAvatar() || !HasCharacter()) { UE_LOG(LogTemp,Warning,TEXT("LH populate refused: owner=%d avatar=%d character=%d"),Owner.IsValid(),Owner.IsValid() && Owner->GetCombatAvatar()!=nullptr,HasCharacter()); return false; }
    auto* Combat=Owner->GetCombatComponent(); Combat->SetStableEntity(PlayerEntity());
    Combat->ConfigureManaRegen(LHWave2::PrototypeProfile().Rules.Mana);
    Combat->ManaRegenFractionalSeconds=Complete.Session.ManaRegenFractionalSeconds.Value;
    LHAbilities::RestoreCooldowns(*Combat,PlayerEntity(),Complete.Session.Cooldowns);
    if (const auto* Rng=Complete.Session.GameplayRng.FindByPredicate([](const auto& R){return R.StreamId==TEXT("RNG.Combat");}); Rng && Rng->State.Num()==4)
    { uint32 Seed=0; for(int32 B=0;B<4;++B) Seed|=uint32(Rng->State[B])<<(B*8); Combat->SetCombatRandomState(FRandomStream(int32(Seed))); }
    const auto* Definition=LHWorld::FindArea(Complete.Character.ActiveEntrance.Area);
    if (!Definition) { UE_LOG(LogTemp,Warning,TEXT("LH populate refused: registry area=%s missing"),*Complete.Character.ActiveEntrance.Area.Content.Value.ToString()); return false; }
    bDeadAwaitingRespawn=Complete.Character.CurrentHealth.Value<=0;
    if (Definition->Spawns.IsEmpty()) return true;
    auto Next=Complete; auto* Area=LHStage1SessionPrivate::Area(Next);
    if (!Area) { FLHAreaRecord New; New.Area=Definition->Id; Next.World.Areas.Add(New); Area=&Next.World.Areas.Last(); }
    const bool FirstVisit=Area->Encounters.IsEmpty();
    FString PopulateError;
    if (!PopulateEncounterCheckpoint(Next,PopulateError)) return false;
    if (FirstVisit)
    {
        if (Next.Header.TransactionSequence==MAX_int64) { UE_LOG(LogTemp,Warning,TEXT("LH populate refused: transaction sequence exhausted")); return false; }
        ++Next.Header.TransactionSequence;
        if (!AcceptBoundary(MoveTemp(Next),false)) { UE_LOG(LogTemp,Warning,TEXT("LH populate refused: checkpoint authority validation failed")); return false; }
    }
    Director=Owner->GetWorld()->GetSubsystem<ULHEncounterDirector>(); if (!Director.IsValid()) { UE_LOG(LogTemp,Warning,TEXT("LH populate refused: loaded world director missing")); return false; }
    Director->RunId=Complete.World.RunId; Director->Player=Owner->GetCombatAvatar();
    Director->ResolveSpec=[](const FLHContentId& Id)->const FLHEnemyRuntimeSpec* { const auto* Row=LHEnemyData::Find(Id); return Row?&Row->Runtime:nullptr; };
    // Director lives in the world; weak session callbacks cannot outlive the session owner.
    Director->SettleKill=[this](const FLHSpawnLifeId& Life,const FLHHitIdentity& Hit,AActor* Killer){return SettleEnemyKill(Life,Hit,Killer);};
    Director->AdvanceRespawns=[this](float Seconds){
        auto Next=Complete; auto* Area=LHStage1SessionPrivate::Area(Next); if (!Area || !Director.IsValid()) return;
        TArray<FLHSpawnLifeId> Lives;
        if (LHRewards::AdvanceRespawns(*Area,Next.World.RunId,Seconds,[this](const FGuid& Id){return Director->IsSpawnSafe(Id);},LHStage1SessionPrivate::Health,Lives)!=ELHCommandReason::None) return;
        const auto OldCorpses=Area->Corpses;
        if (LHRewards::AdvanceLootCleanup(*Area,Seconds,[](const FLHContentId& Id){return Id.Value!=TEXT("Item.Torch");},[](const FLHEntityId&){return false;})!=ELHCommandReason::None) return;
        // Dialogue/loot pause freezes this callback, so open containers are never expired.
        for (const auto& Old:OldCorpses)
            if (!Area->Corpses.ContainsByPredicate([&](const auto& C){return LHStage1SessionPrivate::Same(C.Container,Old.Container);}))
            {
                Director->Despawn(Old.SourceLife);
                for (TActorIterator<ALHEnemyCharacter> It(Owner->GetWorld());It;++It)
                    if (It->IsCorpse() && LHAI::SameLife(It->GetLife(),Old.SourceLife)) It->Destroy();
            }
        // Timer progress is kept in memory until the next completed save/travel boundary.
        Complete.World=MoveTemp(Next.World); Authority()->Import(Complete);
        if (!Lives.IsEmpty())
        {
            ++Complete.Header.TransactionSequence; Authority()->Import(Complete); bSaveQueued=true;
            for (const auto& Life:Lives)
            {
                Director->SpawnLife(Life);
            }
        }
    };
    Director->Populate(*LHStage1SessionPrivate::Area(Complete));
    for (TActorIterator<ALHEnemyCharacter> It(Owner->GetWorld());It;++It)
        if (It->IsAlive() && LHWorld::SameArea(It->GetLife().Area,Complete.Character.ActiveEntrance.Area)) LHStage1SessionPrivate::RestoreEnemy(**It,Complete);
    // Hydrate remaining loot as native corpse actors. Director hydrates only alive encounters.
    for (const auto& C:LHStage1SessionPrivate::Area(Complete)->Corpses)
    {
        if (C.bClaimed) continue;
        const auto* Spawn=Definition->Spawns.FindByPredicate([&](const auto& S){return S.SpawnId==C.SourceLife.SpawnSlot;});
        const auto* Row=Spawn?LHEnemyData::Find(Spawn->Enemy):nullptr; if (!Row) continue;
        bool Exists=false; for (TActorIterator<ALHEnemyCharacter> It(Owner->GetWorld());It;++It) Exists|=It->IsCorpse() && LHAI::SameLife(It->GetLife(),C.SourceLife);
        if (Exists) continue;
        FTransform At=Spawn->Anchor; At.AddToTranslation(FVector(0,0,Row->Runtime.CapsuleHalfHeightCm.Value));
        auto* Actor=Owner->GetWorld()->SpawnActorDeferred<ALHEnemyCharacter>(ALHEnemyCharacter::StaticClass(),At,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        FString Error; auto Hp=Row->Health;
        if (Actor && Actor->ApplyRuntimeSpec(Row->Runtime,C.SourceLife,Hp,Error)) { Actor->FinishSpawning(At); Actor->GetCombatComponent()->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),0); Actor->MarkCorpse(); }
        else if (Actor) Actor->Destroy();
    }
    Director->SetSimulationFrozen(bWorldTravelFrozen || bGameplayPaused || bDeadAwaitingRespawn); return true;
}
bool FLHWave2Session::SettleEnemyKill(const FLHSpawnLifeId& Life,const FLHHitIdentity&,AActor* Killer)
{
    if (!Owner.IsValid() || Killer!=Owner->GetCombatAvatar() || !SyncResources()) return false;
    const auto* Definition=LHWorld::FindArea(Life.Area);
    const auto* Spawn=Definition?Definition->Spawns.FindByPredicate([&](const auto& S){return S.SpawnId==Life.SpawnSlot;}):nullptr;
    const auto* Row=Spawn?LHEnemyData::Find(Spawn->Enemy):nullptr; if (!Row) return false;
    auto Next=Complete; FLHQuestKillObserver Observer; ILHKillObserver* Observers[]={&Observer};
    FLHKillFacts Facts; Facts.Area=Life.Area; Facts.Life=Life; Facts.Enemy=Spawn->Enemy; Facts.bKillerIsPlayer=true;
    if (LHRewards::SettleKill(Next,Facts,Row->Reward,LHWave2::PrototypeProfile(),MakeArrayView(Observers))!=ELHCommandReason::None) return false;
    if (Next.Header.TransactionSequence==MAX_int64) return false;
    ++Next.Header.TransactionSequence; return AcceptBoundary(MoveTemp(Next),true);
}
void FLHWave2Session::TickGameplay(float Seconds)
{
    if (!bInGameplay || IsBlocked() || bGameplayPaused || !Owner.IsValid()) return;
    Owner->GetCombatComponent()->AdvanceLightEffect(Seconds,UGameplayStatics::IsGamePaused(Owner->GetWorld()),
        Director.IsValid() && !Director->IsSimulationEnabled(),false);
    if (Director.IsValid()) Director->TickActiveSimulation(Seconds);
}
void FLHWave2Session::SetGameplayPaused(bool Paused)
{
    bGameplayPaused=Paused;
    if (Director.IsValid()) Director->SetSimulationFrozen(Paused || IsBlocked());
    if (Owner.IsValid()) Owner->GetCombatComponent()->SetRecoveryMenuPaused(Paused || IsBlocked());
}
void FLHWave2Session::HandlePlayerDeath()
{
    if (bDeadAwaitingRespawn || !HasCharacter() || !SyncResources()) return;
    Owner->GetCombatComponent()->RestoreLightRemainingSeconds(0);
    Complete.Session.DurableEffects.Reset();
    bDeadAwaitingRespawn=true; if (Complete.Header.TransactionSequence==MAX_int64) { Message=TEXT("Death checkpoint sequence exhausted."); return; }
    ++Complete.Header.TransactionSequence;
    Authority()->Import(Complete); bSaveQueued=true; SetGameplayPaused(bGameplayPaused);
}
FString FLHWave2Session::RequestRespawn()
{
    if (!HasCharacter() || bTravel || bAwaitingSave || bSaveQueued || !Authority()) return TEXT("Wait for death checkpoint durability.");
    if (Complete.Character.CurrentHealth.Value>0 && !bDeadAwaitingRespawn) return TEXT("Character is alive.");
    const auto* Entrance=LHWorld::FindEntrance(Complete.Session.SafeRespawn.Entrance);
    if (!Entrance || !LHArrivalReview::IsReviewed(*Entrance)) return TEXT("Church arrival safety review is missing.");
    auto Next=Complete;
    const auto Reason=LHRewards::SettlePlayerDeath(Next,LHWave2::PrototypeProfile(),Next.Session.SafeRespawn,true);
    if (Reason!=ELHCommandReason::None) return TEXT("Death recovery rejected.");
    ++Next.Header.TransactionSequence;
    if (!AcceptBoundary(MoveTemp(Next),false)) return TEXT("Death recovery validation failed.");
    // Keep the dead movement gate until durability and Bind on the new church avatar.
    bEnterAfterSave=true; return {};
}
FLHUIHud FLHWave2Session::HudState() const
{
    FLHUIHud H; H.Player=PlayerEntity(); H.Health=Complete.Character.CurrentHealth; H.Mana=Complete.Character.CurrentMana;
    if (Owner.IsValid())
    {
        const auto* C=Owner->GetCombatComponent(); const auto* A=C->GetCombatAttributes();
        if (Owner->GetCombatAvatar()) { H.Health.Value=A->GetHealth(); H.Mana.Value=A->GetMana(); }
        H.MaxHealth=LHWave2::PrototypeNumber(A->GetMaxHealth()); H.MaxMana=LHWave2::PrototypeNumber(A->GetMaxMana());
    }
    H.bDead=bDeadAwaitingRespawn || (HasCharacter() && H.Health.Value<=0); H.SaveStatus=OwnerStatus();
    for (const auto& Ability:LHAbilities::Catalog())
    {
        FLHBasicAttackConfig Config; FString Error;
        FLHUIAbility View; View.Id=Ability.Id; View.Label=Ability.Id.Value.ToString();
        View.bTargetsSelf=Ability.TargetPolicy!=ELHAbilityTarget::Hostile;
        const bool Learned=Ability.Effects==ELHAbilityEffect::PhysicalDamage || Complete.Character.LearnedSpells.ContainsByPredicate([&](const auto& Id){return Id.Value==Ability.Id.Value;});
        View.bAvailable=Learned && !H.bDead && LHAbilities::BuildAttackConfig(Complete,Ability.Id,LHStage1SessionPrivate::Item,LHWave2::PrototypeProfile().Rules.Combat,Config,Error);
        View.Feedback=Learned?Error:TEXT("Learn from a trainer"); H.Abilities.Add(View);
    }
    if (Owner.IsValid() && Owner->GetCombatAvatar())
        if (auto* Controller=Cast<ALHPlayerController>(Owner->GetCombatAvatar()->GetController()))
            if (auto* Target=Cast<ALHEnemyCharacter>(Controller->GetSelectedTarget()))
            {
                H.TargetName=Target->GetRuntimeSpec()?Target->GetRuntimeSpec()->ContentId.Value.ToString():Target->GetName();
                H.TargetHealth=LHWave2::PrototypeNumber(Target->GetCombatComponent()->GetCombatAttributes()->GetHealth());
                H.TargetMaxHealth=LHWave2::PrototypeNumber(Target->GetCombatComponent()->GetCombatAttributes()->GetMaxHealth());
            }
    for (const auto& Q:Complete.World.Quests) if (Q.Quest.Value==TEXT("Quest.SamaritanRats"))
        H.Objective=Q.bRewarded?TEXT("Samaritan errand completed"):FString::Printf(TEXT("Samaritan rats: %lld / 15 — return for the reward"),Q.EligibleKillCount.Value);
    for (const auto& Q:Complete.World.Quests) if (Q.Quest.Value==TEXT("Quest.BalorkReturn"))
    {
        H.Objective=Q.bCompleted?TEXT("Balork return completed"):TEXT("Balork defeated — return to Brother Kiran at the church");
        if (Q.bCompleted) H.CompletionNotice=TEXT("Balork completion reward claimed");
    }
    return H;
}
TArray<FLHUIDialogueTopic> FLHWave2Session::DialogueTopics(const FLHEntityId& Id) const
{
    TArray<FLHUIDialogueTopic> Out; const auto* Npc=ResolveNpc(Id); if (!Npc) return Out;
    for (const auto& T:LHQuests::Topics(Complete,Npc->DefinitionId))
    {
        FLHUIDialogueTopic V; V.Id=T.Id; V.Label=T.Label; V.Text=T.Text; V.bEnabled=Spatial(const_cast<ALHInteractableMarker*>(Npc),250); Out.Add(V);
    }
    return Out;
}
TArray<FLHUILootRow> FLHWave2Session::CorpseContents(const FLHEntityId& Id) const
{
    TArray<FLHUILootRow> Out;
    for (const auto& A:Complete.World.Areas) for (const auto& C:A.Corpses) if (LHStage1SessionPrivate::Same(C.Container,Id) && !C.bClaimed)
    {
        if(C.RemainingGold.Value>0) Out.Add({ELHLootTransferKind::Gold,{},C.RemainingGold,TEXT("Gold")});
        for (const auto& I:C.RemainingItems) Out.Add({ELHLootTransferKind::Item,I.Id,I.Quantity,I.Definition.Value.ToString()});
    }
    return Out;
}

FLHCommandResult FLHWave2Session::Execute(const FLHTrainSkillRequest& Q)
{
    return Persist(Q.Request,TEXT("TrainSkill"),Q.StaticStruct(),&Q,[&](FLHSaveSnapshot& Next){
        auto* Npc=ResolveNpc(Q.Trainer); if (!Npc) return ELHCommandReason::NotFound;
        auto* Pawn=Owner->GetCombatAvatar(); if (!Pawn) return ELHCommandReason::InvalidLifeState;
        const auto Profile=LHWave2::PrototypeProfile(); FLHServiceContext Context;
        Context.Npc=Npc->DefinitionId; Context.Entity=Npc->Materialize(Complete.World.RunId); Context.Profile=&Profile;
        Context.DistanceCm=FVector::Dist(Pawn->GetActorLocation(),Npc->GetActorLocation()); Context.bLineOfSight=Spatial(Npc,250);
        Context.ItemLookup=[](const FLHContentId& Id)->const FLHCharacterItemDefinition* {const auto* Row=LHItemData::Find(Id); return Row?&Row->Character:nullptr;};
        return LHServices::Execute(Context,Next,Q);
    });
}

FLHCommandResult FLHWave2Session::Execute(const FLHLearnSpellRequest& Q)
{
    return Persist(Q.Request,TEXT("LearnSpell"),Q.StaticStruct(),&Q,[&](FLHSaveSnapshot& Next){
        auto* Npc=ResolveNpc(Q.Trainer); if (!Npc) return ELHCommandReason::NotFound;
        auto* Pawn=Owner->GetCombatAvatar(); if (!Pawn) return ELHCommandReason::InvalidLifeState;
        const auto Profile=LHWave2::PrototypeProfile(); FLHServiceContext Context;
        Context.Npc=Npc->DefinitionId; Context.Entity=Npc->Materialize(Complete.World.RunId); Context.Profile=&Profile;
        Context.DistanceCm=FVector::Dist(Pawn->GetActorLocation(),Npc->GetActorLocation()); Context.bLineOfSight=Spatial(Npc,250);
        Context.ItemLookup=[](const FLHContentId& Id)->const FLHCharacterItemDefinition* {const auto* Row=LHItemData::Find(Id); return Row?&Row->Character:nullptr;};
        return LHServices::Execute(Context,Next,Q);
    });
}

FLHCommandResult FLHWave2Session::Execute(const FLHBuyItemRequest& Q)
{
    return Persist(Q.Request,TEXT("BuyItem"),Q.StaticStruct(),&Q,[&](FLHSaveSnapshot& Next){
        auto* Npc=ResolveNpc(Q.Vendor); if (!Npc) return ELHCommandReason::NotFound;
        auto* Pawn=Owner->GetCombatAvatar(); if (!Pawn) return ELHCommandReason::InvalidLifeState;
        const auto Profile=LHWave2::PrototypeProfile(); FLHServiceContext Context;
        Context.Npc=Npc->DefinitionId; Context.Entity=Npc->Materialize(Complete.World.RunId); Context.Profile=&Profile;
        Context.DistanceCm=FVector::Dist(Pawn->GetActorLocation(),Npc->GetActorLocation()); Context.bLineOfSight=Spatial(Npc,250);
        Context.ItemLookup=[](const FLHContentId& Id)->const FLHCharacterItemDefinition* {const auto* Row=LHItemData::Find(Id); return Row?&Row->Character:nullptr;};
        return LHServices::Execute(Context,Next,Q);
    });
}

FLHCommandResult FLHWave2Session::Execute(const FLHSellItemRequest& Q)
{
    return Persist(Q.Request,TEXT("SellItem"),Q.StaticStruct(),&Q,[&](FLHSaveSnapshot& Next){
        auto* Npc=ResolveNpc(Q.Vendor); if (!Npc) return ELHCommandReason::NotFound;
        auto* Pawn=Owner->GetCombatAvatar(); if (!Pawn) return ELHCommandReason::InvalidLifeState;
        const auto Profile=LHWave2::PrototypeProfile(); FLHServiceContext Context;
        Context.Npc=Npc->DefinitionId; Context.Entity=Npc->Materialize(Complete.World.RunId); Context.Profile=&Profile;
        Context.DistanceCm=FVector::Dist(Pawn->GetActorLocation(),Npc->GetActorLocation()); Context.bLineOfSight=Spatial(Npc,250);
        Context.ItemLookup=[](const FLHContentId& Id)->const FLHCharacterItemDefinition* {const auto* Row=LHItemData::Find(Id); return Row?&Row->Character:nullptr;};
        return LHServices::Execute(Context,Next,Q);
    });
}

TArray<FLHUIServiceOffer> FLHWave2Session::ServiceOffers(const FLHEntityId& Id) const
{
    TArray<FLHUIServiceOffer> Out; auto* Npc=ResolveNpc(Id); if (!Npc) return Out;
    const bool SpatiallyAvailable=Spatial(Npc,250) && !IsTransactionBlocked();
    for (const auto& View:LHServices::Offers(Npc->DefinitionId,Snapshot()))
    {
        FLHUIServiceOffer V; V.Id=View.Offer.Id; V.Quantity=LHWave2::PrototypeInteger(1);
        V.Kind=View.Offer.Kind==ELHServiceKind::TrainSkill?ELHUIOfferKind::Train:View.Offer.Kind==ELHServiceKind::LearnSpell?ELHUIOfferKind::Learn:ELHUIOfferKind::Buy;
        // Presenter passes Subject for training/learning, Offer ID for buying.
        if (V.Kind!=ELHUIOfferKind::Buy) V.Id=View.Offer.Subject;
        V.Label=View.Offer.Subject.Value.ToString();
        V.Price=FString::Printf(TEXT("%lld gold / %lld skill points"),View.Offer.Gold.Value,View.Offer.SkillPoints.Value);
        V.bEnabled=View.bEligible && SpatiallyAvailable;
        V.Feedback=V.bEnabled?FString():TEXT("Requires range, available resources and a completed save"); Out.Add(V);
    }
    const bool Vendor=LHServices::Catalog().ContainsByPredicate([&](const auto& Offer){return Offer.Npc.Value==Npc->DefinitionId.Value && Offer.Kind==ELHServiceKind::BuyItem;});
    if (Vendor) for (const auto& Item:Complete.Character.Inventory)
    {
        const auto Price=LHServices::SellPrice(Item.Definition);
        if (Price.Resolution!=ELHValueResolution::Resolved) continue;
        FLHUIServiceOffer V; V.Kind=ELHUIOfferKind::Sell; V.Item=Item.Id; V.Quantity=LHWave2::PrototypeInteger(1);
        V.Label=TEXT("Sell ")+Item.Definition.Value.ToString(); V.Price=FString::Printf(TEXT("%lld gold"),Price.Value);
        V.bEnabled=SpatiallyAvailable && !Complete.Character.Equipment.ContainsByPredicate([&](const auto& E){return LHStage1SessionPrivate::Same(E.Item,Item.Id);}); Out.Add(V);
    }
    return Out;
}

bool FLHWave2Session::HasPendingCombat() const
{
    if (!Owner.IsValid()) return false;
    const auto* Player=Owner->GetCombatComponent();
    if (Player->IsActionPending() || Player->IsPublishingActionEvents()) return true;
    if (Director.IsValid()) for (TActorIterator<ALHEnemyCharacter> It(Owner->GetWorld());It;++It)
        if (LHWorld::SameArea(It->GetLife().Area,Complete.Character.ActiveEntrance.Area))
        {
            const auto* Combat=It->GetCombatComponent();
            if (Combat->IsActionPending() || Combat->IsPublishingActionEvents()) return true;
        }
    return false;
}
