#pragma once
#include "UI/LHUIPresenter.h"
#include "Character/LHCharacterAuthority.h"
#include "Persistence/LHSaveStore.h"
class ALHPlayerState;
class ULHEncounterDirector;
class ALHInteractableMarker;
struct FLHHitIdentity;
// Game-thread owner adapter. Flush runs on the next controller tick, outside command publication.
class LIGHTHAVEN_API FLHWave2Session : public ILHCommandHandler, public ILHUIReadOwner, public ILHUISessionOwner
{
public:
    explicit FLHWave2Session(TSharedRef<FLHSaveStore> Store);
    ~FLHWave2Session();
    static FLHSaveCompatibility Compatibility();
    bool Bind(ALHPlayerState* State, bool bFreshCreation = false);
    bool HasCharacter() const { return Complete.Header.CharacterId.Value.IsValid(); }
    bool IsBlocked() const { return bContentUnavailable || bTravel || bWorldTravelFrozen || bDeadAwaitingRespawn; }
    void Flush();
    const FString& Status() const { return Message; }
    bool OwnsPersistenceStatus() const override { return true; }
    bool HasUnsavedChanges() const override { return bSaveQueued || bAwaitingSave || (HasCharacter() && Saves->IsDirty(Complete.Header.CharacterId)); }
    FString DerivedSummary() const override;
    FString OwnerStatus() const override { return bContentUnavailable ? TEXT("Required packaged map content is missing.") : bWorldTravelFrozen && WorldTravelStatus ? WorldTravelStatus() : Message; }
    bool BeginCreation() override;
    void ClearSelectionError() override;
    FString RetryPersistence() override;
    void FreezeWorldTravel(bool Frozen);
    void AbortGameplayArrival(const FString& Error);
    TFunction<FString()> WorldTravelStatus;
    bool CaptureTravel(FLHSaveSnapshot& Out,FString& Error);
    bool InstallTravel(const FLHSaveSnapshot& Snapshot);
    TFunction<bool(const FLHRequestTravelRequest&,FString&)> RequestWorldTravel;
    FLHCommandResult Execute(const FLHRequestTravelRequest&) override;
    bool bWorldTravelFrozen=false, bContentUnavailable=false;
    TFunction<bool(FString&)> RetryWorldTravel;
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
    LH_UNSUPPORTED(FLHSellItemRequest)
    FLHCommandResult Execute(const FLHUseAbilityRequest&) override;
    FLHCommandResult Execute(const FLHInteractRequest&) override;
    FLHCommandResult Execute(const FLHUseItemRequest&) override;
    FLHCommandResult Execute(const FLHTakeLootRequest&) override;
    FLHUIHud HudState() const override;
    TArray<FLHUIDialogueTopic> DialogueTopics(const FLHEntityId&) const override;
    TArray<FLHUILootRow> CorpseContents(const FLHEntityId&) const override;
    FString RequestRespawn() override;
    void SetGameplayPaused(bool) override;
    bool StartEncounters();
    void TickGameplay(float Seconds);
    void HandlePlayerDeath();
    bool SettleEnemyKill(const FLHSpawnLifeId&, const FLHHitIdentity&, AActor*);
#undef LH_UNSUPPORTED
private:
    bool IsTransactionBlocked() const { return IsBlocked() || bSaveQueued || bAwaitingSave; }
    bool SyncResources();
    bool AcceptBoundary(FLHSaveSnapshot&&, bool bInstallResources);
    FLHEntityId PlayerEntity() const;
    ALHInteractableMarker* ResolveNpc(const FLHEntityId&) const;
    bool Spatial(AActor*, double Range) const;
    FLHCommandResult Persist(const FLHRequestId&, FName, const UScriptStruct*, const void*, TFunctionRef<ELHCommandReason(FLHSaveSnapshot&)>);
    TWeakObjectPtr<ULHEncounterDirector> Director;
    bool bDeadAwaitingRespawn=false, bGameplayPaused=false;
    TMap<FGuid,TPair<FString,FLHCommandResult>> RuntimeAbilities;
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
