#include "Quests/LHQuests.h"
#include "Persistence/LHSaveCodec.h"
namespace LHQuestPrivate
{
FLHContentId Id(const TCHAR* V) { FLHContentId I; I.Value=V; return I; }
FLHInteger Count(int64 V) { FLHInteger I; I.Value=V; I.Resolution=ELHValueResolution::Resolved; I.Provenance.Status=ELHProvenanceStatus::Prototype; I.Provenance.Notes=TEXT("Authored quest bookkeeping; count of settled eligible kills"); return I; }
FLHQuestRecord* Find(FLHSaveSnapshot& S,const TCHAR* V) { return S.World.Quests.FindByPredicate([&](const auto& Q){return Q.Quest.Value==V;}); }
const FLHQuestRecord* Find(const FLHSaveSnapshot& S,const TCHAR* V) { return S.World.Quests.FindByPredicate([&](const auto& Q){return Q.Quest.Value==V;}); }
FLHQuestRecord& Ensure(FLHSaveSnapshot& S,const TCHAR* V) { if(auto* Q=Find(S,V)) return *Q; FLHQuestRecord Q; Q.Quest=Id(V); Q.EligibleKillCount=Count(0); S.World.Quests.Add(Q); return S.World.Quests.Last(); }
bool Same(const FLHEntityId& A,const FLHEntityId& B) { return A.RunId==B.RunId && A.Area.Content.Value==B.Area.Content.Value && A.InstanceId==B.InstanceId; }
}
TArray<FLHQuestTopic> LHQuests::Topics(const FLHSaveSnapshot& S,const FLHContentId& Npc)
{
    using namespace LHQuestPrivate;
    TArray<FLHQuestTopic> T;
    auto Add=[&](const TCHAR* I,const TCHAR* L,const TCHAR* Text){ T.Add({Id(I),L,Text}); };
    if(Npc.Value==TEXT("NPC.Samaritan"))
    {
        const auto* Q=Find(S,TEXT("Quest.SamaritanRats"));
        if(!Q || !Q->bAccepted) Add(TEXT("Topic.AcceptRats"),TEXT("Help with rats"),TEXT("Defeat fifteen Brown Rats below the temple after accepting, then return."));
        else if(!Q->bRewarded) Add(TEXT("Topic.TurnInRats"),TEXT("Report on rats"),TEXT("The reward for fifteen eligible rats is 2500 experience, separate from combat experience."));
    }
    if(Npc.Value==TEXT("NPC.BrotherKiran"))
    {
        Add(TEXT("Topic.Church"),TEXT("The church"),TEXT("You can return here for shelter. Speak with Nevanis below for healing."));
        Add(TEXT("Topic.Descent"),TEXT("The descent"),TEXT("Explore the four temple basement floors. Defeat Balork and speak with me here when you return."));
        const auto* Q=Find(S,TEXT("Quest.BalorkReturn"));
        if(Q && Q->Stage==TEXT("ReturnToChurch") && !Q->bCompleted) Add(TEXT("Topic.BalorkReturn"),TEXT("Balork defeated"),TEXT("You defeated Balork and returned to the church. This completes the Lighthaven slice objective."));
    }
    if(Npc.Value==TEXT("NPC.Nevanis")) Add(TEXT("Topic.Heal"),TEXT("Heal wounds"),TEXT("Prototype: free full health for living injured players. Mana is unchanged."));
    // Service availability is an owner view; these identities match the authored service roster.
    for(const TCHAR* V:{TEXT("NPC.Sigfried"),TEXT("NPC.Fali"),TEXT("NPC.Iraltok"),TEXT("NPC.Kilhiam"),TEXT("NPC.Moonrock"),TEXT("NPC.Uranos"),TEXT("NPC.Shovanis"),TEXT("NPC.BrotherKiran"),TEXT("NPC.Ortanalas"),TEXT("NPC.JagarKar"),TEXT("NPC.Kalastor"),TEXT("NPC.Murmuntag"),TEXT("NPC.Rolph")})
        if(Npc.Value==V) Add(TEXT("Topic.Services"),TEXT("Services"),TEXT("Browse available offers."));
    return T;
}
ELHCommandReason LHQuests::ExecuteInteract(FLHSaveSnapshot& S,const FLHInteractContext& C,const FLHInteractRequest& R)
{
    using namespace LHQuestPrivate;
    if(!C.Entity.InstanceId.IsValid() || C.Entity.Area.Content.Value.IsNone() || C.Entity.RunId!=S.World.RunId || !Same(C.Entity,R.Target)) return ELHCommandReason::NotFound;
    if(!FMath::IsFinite(C.DistanceCm) || C.DistanceCm<0 || C.DistanceCm>250) return ELHCommandReason::OutOfRange;
    if(!C.bLineOfSight) return ELHCommandReason::Obstructed;
    if(S.Character.CurrentHealth.Resolution!=ELHValueResolution::Resolved) return ELHCommandReason::UnresolvedRules;
    if(S.Character.CurrentHealth.Value<=0) return ELHCommandReason::InvalidLifeState;
    if(!Topics(S,C.Npc).ContainsByPredicate([&](const auto& T){return T.Id.Value==R.Topic.Value;})) return ELHCommandReason::Ineligible;
    auto Next=S;
    if(R.Topic.Value==TEXT("Topic.AcceptRats")) { auto& Q=Ensure(Next,TEXT("Quest.SamaritanRats")); Q.bAccepted=true; Q.Stage=TEXT("HuntRats"); }
    else if(R.Topic.Value==TEXT("Topic.TurnInRats"))
    {
        auto* Q=Find(Next,TEXT("Quest.SamaritanRats"));
        if(!Q || Q->EligibleKillCount.Resolution!=ELHValueResolution::Resolved || Q->EligibleKillCount.Value<15 || Q->bRewarded) return ELHCommandReason::Ineligible;
        FLHRewardId Claim; FLHSaveError Error;
        if(!LHSave::QuestTurnInRewardId(S.World.RunId,Q->Quest,TEXT("TurnIn"),Claim,Error)) return ELHCommandReason::InvalidRequest;
        if(S.World.ClaimedUniqueRewards.ContainsByPredicate([&](const auto& I){return I.Value==Claim.Value;})) return ELHCommandReason::Ineligible;
        if(!C.Profile) return ELHCommandReason::UnresolvedRules;
        FLHCharacterAuthority A;
        if(!A.Initialize(*C.Profile,S.Session.RequestEpoch,0) || A.Import(S)!=ELHCommandReason::None) return ELHCommandReason::InvalidRequest;
        const auto Reason=A.GrantExperience(2500); if(Reason!=ELHCommandReason::None) return Reason;
        A.Export(Next); Next.Header.TransactionSequence=S.Header.TransactionSequence;
        Q=Find(Next,TEXT("Quest.SamaritanRats")); Q->bCompleted=true; Q->bRewarded=true; Q->Stage=TEXT("TurnIn"); Q->TurnInClaim=Claim;
        Next.World.ClaimedUniqueRewards.Add(Claim);
    }
    else if(R.Topic.Value==TEXT("Topic.BalorkReturn")) { auto& Q=Ensure(Next,TEXT("Quest.BalorkReturn")); Q.bCompleted=true; Q.Stage=TEXT("Complete"); }
    else if(R.Topic.Value==TEXT("Topic.Heal"))
    {
        if(!C.Profile) return ELHCommandReason::UnresolvedRules;
        FLHCharacterAuthority A; if(!A.Initialize(*C.Profile,S.Session.RequestEpoch,0) || A.Import(S)!=ELHCommandReason::None) return ELHCommandReason::InvalidRequest;
        const auto Stats=A.Stats(); if(!Stats.Diagnostic.IsAccepted()) return ELHCommandReason::UnresolvedRules;
        if(S.Character.CurrentHealth.Value>=Stats.Value.MaxHealth) return ELHCommandReason::NoEffect;
        Next.Character.CurrentHealth.Value=Stats.Value.MaxHealth;
        Next.Character.CurrentHealth.Provenance.Status=ELHProvenanceStatus::Prototype;
        Next.Character.CurrentHealth.Provenance.Notes=TEXT("R-03 missing Nevanis magnitude/cost/restrictions: free full HP, alive injured,250cm LOS; replace after evidence/balance review. No MP restore.");
    }
    S=MoveTemp(Next); return ELHCommandReason::None;
}
ELHCommandReason FLHQuestKillObserver::OnSettledKill(const FLHKillFacts& F,FLHSaveSnapshot& S)
{
    using namespace LHQuestPrivate;
    if(!F.bKillerIsPlayer) return ELHCommandReason::None;
    if(F.Enemy.Value==TEXT("Enemy.BrownRat") && (F.Area.Content.Value==TEXT("Area.TempleB1") || F.Area.Content.Value==TEXT("Area.TempleB2") || F.Area.Content.Value==TEXT("Area.TempleB3") || F.Area.Content.Value==TEXT("Area.TempleB4")))
    {
        if(auto* Q=Find(S,TEXT("Quest.SamaritanRats")); Q && Q->bAccepted && !Q->bCompleted)
        {
            if(Q->EligibleKillCount.Resolution!=ELHValueResolution::Resolved || Q->EligibleKillCount.Value<0) return ELHCommandReason::UnresolvedRules;
            Q->EligibleKillCount=Count(FMath::Min<int64>(14,Q->EligibleKillCount.Value)+1);
        }
    }
    if(F.Enemy.Value==TEXT("Enemy.Balork")) { auto& Q=Ensure(S,TEXT("Quest.BalorkReturn")); if(!Q.bCompleted) { Q.bAccepted=true; Q.Stage=TEXT("ReturnToChurch"); } }
    return ELHCommandReason::None;
}
