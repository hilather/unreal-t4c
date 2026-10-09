#pragma once
#include "Abilities/LHAbilityCatalog.h"
enum class ELHServiceKind : uint8 { TrainSkill, LearnSpell, BuyItem };
struct LIGHTHAVEN_API FLHServiceOffer
{
    FLHContentId Id, Npc, Subject;
    FName Area;
    ELHServiceKind Kind=ELHServiceKind::TrainSkill;
    FLHInteger Gold, SkillPoints, RankCap;
};
namespace LHServices
{
LIGHTHAVEN_API const TArray<FLHServiceOffer>& Catalog();
LIGHTHAVEN_API FLHInteger PrototypeInteger(int64 Value, const TCHAR* Note);
LIGHTHAVEN_API FLHInteger SellPrice(const FLHContentId& Item);
}
