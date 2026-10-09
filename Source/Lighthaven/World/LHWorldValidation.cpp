#include "LHWorldValidation.h"
#include "Services/LHServiceCatalog.h"
#include "Data/Encounters/LHEncounterCatalog.h"
#include "Data/Enemies/LHEnemyCatalog.h"
bool LHWorld::ValidatePlacements(const TArray<FLHAreaId>& Maps,const TArray<FLHPlacedIdentity>& Identities,
    const TArray<FLHPlacedEntrance>& Entrances,TArray<FString>& Errors)
{
    Errors.Reset(); TSet<FName> Areas; TSet<FGuid> AllIds;
    for (const auto& Map:Maps)
    {
        if (!FindArea(Map) || Areas.Contains(Map.Content.Value)) Errors.Add(TEXT("Unknown or duplicate playable map"));
        Areas.Add(Map.Content.Value);
    }
    if (Maps.Num()!=5 || Areas.Num()!=5) Errors.Add(TEXT("Validator requires all five playable maps"));
    for (const auto& I:Identities)
    {
        const auto* Area=FindArea(I.Area);
        if (!Area || !Areas.Contains(I.Area.Content.Value) || !I.Id.IsValid() || AllIds.Contains(I.Id)) Errors.Add(I.Context+TEXT(": unknown area, invalid or duplicate ID"));
        AllIds.Add(I.Id);
        if (I.Kind==ELHPlacedIdKind::Spawn && Area)
        {
            const auto* S=Area->Spawns.FindByPredicate([&](const auto& Slot) { return Slot.SpawnId==I.Id; });
            if (!S || !S->Enemy.Value.ToString().Equals(I.Definition.Value.ToString(),ESearchCase::CaseSensitive)) Errors.Add(I.Context+TEXT(": invalid spawn definition/slot"));
        }
        if (I.Kind==ELHPlacedIdKind::Interactable && I.Definition.Value.IsNone()) Errors.Add(I.Context+TEXT(": interactable definition missing"));
        if (I.Kind==ELHPlacedIdKind::Portal)
        {
            FLHEntityId Id; Id.Area=I.Area; Id.InstanceId=I.Id; const auto* P=FindPortal(Id);
            if (!P || !SameEntrance(P->Source,I.Source) || !SameEntrance(P->Destination,I.Destination) || !SameArea(I.Area,I.Source.Area)) Errors.Add(I.Context+TEXT(": unauthorized portal edge"));
            const bool bExpectedReturn=I.Source.LocalId.ToString().Equals(TEXT("Entry"),ESearchCase::CaseSensitive);
            if (I.bReturn!=bExpectedReturn) Errors.Add(I.Context+TEXT(": incorrect portal direction"));
            int32 Reverse=0;
            for (const auto& R:Identities) if (R.Kind==ELHPlacedIdKind::Portal && SameEntrance(I.Source,R.Destination) && SameEntrance(I.Destination,R.Source)) ++Reverse;
            if (Reverse!=1) Errors.Add(I.Context+TEXT(": portal pair missing or ambiguous"));
        }
    }
    TSet<FString> EntranceKeys;
    for (const auto& E:Entrances)
    {
        const FString Key=E.Id.Area.Content.Value.ToString()+TEXT("::")+E.Id.LocalId.ToString();
        const auto* Expected=FindEntrance(E.Id);
        if (!Expected || EntranceKeys.Contains(Key) || !SafeTransform(E.Transform) || !E.bSafetyReviewed || (Expected && !E.Transform.Equals(Expected->SafeTransform))) Errors.Add(E.Context+TEXT(": invalid/unreviewed/duplicate entrance transform"));
        EntranceKeys.Add(Key);
    }
    for (const auto& A:Registry())
    {
        for (const auto& E:A.Entrances)
            if (!EntranceKeys.Contains(E.Id.Area.Content.Value.ToString()+TEXT("::")+E.Id.LocalId.ToString())) Errors.Add(TEXT("Required entrance absent"));
        for (const auto& P:A.Portals)
            if (!Identities.ContainsByPredicate([&](const auto& I) { return I.Kind==ELHPlacedIdKind::Portal && SameArea(I.Area,A.Id) && I.Id==P.Portal.InstanceId; })) Errors.Add(TEXT("Required portal absent"));
        for (const auto& S:A.Spawns)
            if (!Identities.ContainsByPredicate([&](const auto& I) { return I.Kind==ELHPlacedIdKind::Spawn && SameArea(I.Area,A.Id) && I.Id==S.SpawnId; })) Errors.Add(TEXT("Required spawn absent"));
    }
    TMap<FName,FName> Required;
    for (const auto& Offer:LHServices::Catalog()) Required.Add(Offer.Npc.Value,Offer.Area==TEXT("B1")?FName(TEXT("Area.TempleB1")):FName(TEXT("Area.LighthavenTempleDistrict")));
    Required.Add(TEXT("NPC.Samaritan"),TEXT("Area.LighthavenTempleDistrict"));
    Required.Add(TEXT("NPC.BrotherKiran"),TEXT("Area.LighthavenTempleDistrict"));
    Required.Add(TEXT("NPC.Nevanis"),TEXT("Area.TempleB1"));
    for (const auto& Npc:Required)
    {
        int32 Count=0;
        for (const auto& I:Identities) if (I.Kind==ELHPlacedIdKind::Interactable && I.Definition.Value==Npc.Key)
        { ++Count; if (I.Area.Content.Value!=Npc.Value) Errors.Add(I.Context+TEXT(": NPC is outside its catalog area")); }
        if (Count!=1) Errors.Add(Npc.Key.ToString()+TEXT(": referenced NPC must exist exactly once"));
    }
    for (const auto& E:LHEncounterData::Catalog())
    {
        const auto* Enemy=LHEnemyData::Find(E.Enemy);
        if (!Enemy || E.bBoss!=Enemy->Reward.bBoss || E.RespawnPolicy!=Enemy->Reward.RespawnPolicy || E.RespawnSeconds.Value!=Enemy->Reward.RespawnSeconds.Value)
            Errors.Add(TEXT("Encounter reward/respawn flags disagree with enemy catalog"));
        if (E.bBoss && (E.Enemy.Value!=TEXT("Enemy.Balork") || E.Area.Content.Value!=TEXT("Area.TempleB4") ||
            E.RespawnPolicy!=ELHRespawnPolicy::OrdinaryRepeat || E.RespawnSeconds.Value!=900))
            Errors.Add(TEXT("Balork must be B4 OrdinaryRepeat with 900s respawn; completion uses BossUnique"));
    }
    return Errors.IsEmpty();
}
