#pragma once
#include "Rewards/LHRewardTypes.h"
#include "Character/LHCharacterAuthority.h"
struct LIGHTHAVEN_API FLHInteractContext
{
    FLHContentId Npc;
    FLHEntityId Entity;
    double DistanceCm = -1;
    bool bLineOfSight = false;
    // Authoritative profile required for derived max HP and level-growth awards.
    const FLHCharacterProfile* Profile = nullptr;
};
struct LIGHTHAVEN_API FLHQuestTopic
{
    FLHContentId Id;
    FString Label, Text;
};
namespace LHQuests
{
    LIGHTHAVEN_API TArray<FLHQuestTopic> Topics(const FLHSaveSnapshot&, const FLHContentId& Npc);
    LIGHTHAVEN_API ELHCommandReason ExecuteInteract(FLHSaveSnapshot&, const FLHInteractContext&, const FLHInteractRequest&);
}
class LIGHTHAVEN_API FLHQuestKillObserver : public ILHKillObserver
{
public:
    virtual ELHCommandReason OnSettledKill(const FLHKillFacts&, FLHSaveSnapshot&) override;
};
