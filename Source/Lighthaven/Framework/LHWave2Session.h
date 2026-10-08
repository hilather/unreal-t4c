#pragma once
#include "UI/LHUIPresenter.h"
#include "Character/LHCharacterAuthority.h"
#include "Persistence/LHSaveStore.h"
class ALHPlayerState;
// Game-thread owner adapter. Flush runs on the next controller tick, outside command publication.
class LIGHTHAVEN_API FLHWave2Session : public ILHCommandHandler, public ILHUIReadOwner, public ILHUISessionOwner
{
public:
    explicit FLHWave2Session(TSharedRef<FLHSaveStore> Store);
    ~FLHWave2Session();
    static FLHSaveCompatibility Compatibility();
    bool Bind(ALHPlayerState* State, bool bFreshCreation = false);
    bool HasCharacter() const { return Complete.Header.CharacterId.Value.IsValid(); }
    bool IsBlocked() const { return bSaveQueued || bAwaitingSave || bTravel; }
    void Flush();
    const FString& Status() const { return Message; }
    FString OwnerStatus() const override { return Message; }
    bool BeginCreation() override;
    FString RetryPersistence() override;
    TFunction<void()> Travel;
    TFunction<void()> Exit;
    TFunction<void()> Resume;
    bool bInGameplay=false;
    bool ResumeGameplay() override { if (!Resume || IsBlocked()) return false; Resume(); return true; }
    FLHSaveSnapshot Snapshot() const override;
    TArray<FLHUIProfile> Profiles() const override;
    TArray<FLHContentId> AppearanceCatalog() const override;
    TArray<FLHUIQuestion> QuestionCatalog() const override;
    FLHUICreationPreview Preview(const FString&, const TArray<FLHContentId>&, const TArray<FLHQuestionAnswer>&, bool) override;
    FLHUIIntentReview ReviewAllocation(const FLHAttributeBlock&) const override;
    FLHUIIntentReview ReviewEquipment(const FLHEntityId&, ELHEquipmentSlot, bool) const override;
    FString Continue(FLHCharacterId, bool) override;
    FString RequestExit() override;
    FLHCommandResult Execute(const FLHCreateCharacterRequest&) override;
    FLHCommandResult Execute(const FLHAllocateAttributePointsRequest&) override;
    FLHCommandResult Execute(const FLHEquipItemRequest&) override;
#define LH_UNSUPPORTED(T) FLHCommandResult Execute(const T&) override { FLHCommandResult R; R.Reason=ELHCommandReason::UnresolvedRules; return R; }
    LH_UNSUPPORTED(FLHTrainSkillRequest) LH_UNSUPPORTED(FLHLearnSpellRequest) LH_UNSUPPORTED(FLHBuyItemRequest)
    LH_UNSUPPORTED(FLHSellItemRequest) LH_UNSUPPORTED(FLHUseAbilityRequest) LH_UNSUPPORTED(FLHInteractRequest)
    LH_UNSUPPORTED(FLHTakeLootRequest) LH_UNSUPPORTED(FLHRequestTravelRequest)
#undef LH_UNSUPPORTED
private:
    TSharedRef<FLHSaveStore> Saves;
    FDelegateHandle EventHandle;
    TWeakObjectPtr<ALHPlayerState> Owner;
    FLHSaveSnapshot Complete;
    FString Message;
    mutable TSet<FGuid> ReportedUnreadable;
    mutable TArray<FLHUIProfile> CachedProfiles;
    mutable bool bProfilesDirty=true;
    bool bSaveQueued=false, bAwaitingSave=false, bTravel=false, bExit=false;
    bool bEnterAfterSave=false;
    int64 AwaitedSequence=0;
    FLHCharacterAuthority* Authority() const;
    void OnSave(const FLHSaveEvent& Event);
    void Published(const FLHCommandResult&, bool bCreation);
    bool InstallDerived();
};
