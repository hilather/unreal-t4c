#pragma once
#include "Services/LHServiceCatalog.h"
#include "Character/LHCharacterAuthority.h"
struct LIGHTHAVEN_API FLHServiceContext
{
    FLHContentId Npc;
    FLHEntityId Entity;
    double DistanceCm=0;
    bool bLineOfSight=false, bBusy=false;
    const FLHCharacterProfile* Profile=nullptr;
    TFunction<const FLHAbilityCatalogRow*(const FLHContentId&)> AbilityLookup=LHAbilities::Find;
    TFunction<const FLHCharacterItemDefinition*(const FLHContentId&)> ItemLookup;
};
struct LIGHTHAVEN_API FLHServiceOfferView
{
    FLHServiceOffer Offer;
    bool bEligible=false;
    ELHCommandReason Reason=ELHCommandReason::UnresolvedRules;
};
namespace LHServices
{
LIGHTHAVEN_API ELHCommandReason Execute(const FLHServiceContext&, FLHSaveSnapshot&, const FLHTrainSkillRequest&);
LIGHTHAVEN_API ELHCommandReason Execute(const FLHServiceContext&, FLHSaveSnapshot&, const FLHLearnSpellRequest&);
LIGHTHAVEN_API ELHCommandReason Execute(const FLHServiceContext&, FLHSaveSnapshot&, const FLHBuyItemRequest&);
LIGHTHAVEN_API ELHCommandReason Execute(const FLHServiceContext&, FLHSaveSnapshot&, const FLHSellItemRequest&);
// Catalog-only read model: runtime range/busy/item capacity are checked by Execute.
LIGHTHAVEN_API TArray<FLHServiceOfferView> Offers(const FLHContentId&, const FLHSaveSnapshot&);
}
