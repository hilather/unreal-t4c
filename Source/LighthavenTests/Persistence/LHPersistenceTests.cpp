#include "Algo/Reverse.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"
#include "Persistence/LHSaveStore.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace LHPersistenceTestsPrivate
{
FLHInteger I(int64 V) { FLHInteger R; R.Resolution=ELHValueResolution::Resolved; R.Value=V; R.Provenance.Status=ELHProvenanceStatus::Prototype; R.Provenance.Notes=TEXT("Synthetic persistence fixture; no authentic gameplay tuning"); return R; }
FLHNumber N(double V) { FLHNumber R; R.Resolution=ELHValueResolution::Resolved; R.Value=V; R.Provenance.Status=ELHProvenanceStatus::Prototype; return R; }
FLHContentId Content(const TCHAR* V) { FLHContentId R; R.Value=V; return R; }
FLHEntityId Entity(const FLHSaveSnapshot& S, uint32 V)
{ FLHEntityId R; R.RunId=S.World.RunId; R.Area=S.Character.ActiveEntrance.Area; R.InstanceId=FGuid(0,0,0,V); return R; }
FLHSaveSnapshot Fixture()
{
    FLHSaveSnapshot S;
    S.Header.TransactionSequence=1; S.Header.BuildId=TEXT("Automation.Synthetic1"); S.Header.CharacterId.Value=FGuid(1,2,3,4);
    S.Header.ChecksumAlgorithm=TEXT("SHA256"); S.Header.PayloadCodec=TEXT("LHCanonicalBinary1");
    S.Header.Ruleset.Id=Content(TEXT("Ruleset.PersistenceFixture")); S.Header.Ruleset.Revision=1;
    S.Header.Ruleset.HashAlgorithm=TEXT("SHA256"); S.Header.Ruleset.ContentHash=FString::ChrN(64,'a'); S.Header.ContentRevision=FString::ChrN(64,'b');
    S.World.RunId=FGuid(5,6,7,8); S.Session.RequestEpoch=FGuid(9,10,11,12);
    S.Session.EffectPolicy=ELHEffectSavePolicy::CompletedActionBoundaryOnly;
    auto& C=S.Character; C.DisplayName=TEXT("Fixture \u00e9"); C.AppearanceIds={Content(TEXT("Appearance.HumanFixture"))};
    C.ActiveEntrance.Area.Content=Content(TEXT("Area.LighthavenTempleDistrict")); C.ActiveEntrance.LocalId=TEXT("SafeSpawn");
    C.BaseAttributes.Strength=I(10); C.BaseAttributes.Endurance=I(11); C.BaseAttributes.Agility=I(12); C.BaseAttributes.Intelligence=I(13); C.BaseAttributes.Wisdom=I(14);
    C.Creation.AcceptedAttributes=C.BaseAttributes; C.Creation.GenerationRevision=I(1); C.Creation.GenerationPolicy=Content(TEXT("CreationPolicy.PersistenceFixture"));
    FLHQuestionAnswer Q; Q.Question=Content(TEXT("Question.Fixture")); Q.Answer=Content(TEXT("Answer.Fixture")); C.Creation.QuestionAnswers.Add(Q);
    C.EarnedLevel=I(2); C.ExperienceBalance=I(42); C.ExperienceDebt=I(7); C.UnspentAttributePoints=I(3); C.UnspentSkillPoints=I(4);
    C.EarnedBaseHealth=N(30); C.EarnedBaseMana=N(20); C.CurrentHealth=N(17.5); C.CurrentMana=N(8.25); C.Gold=I(100);
    FLHLearnedSkill Skill; Skill.Skill=Content(TEXT("Skill.Fixture")); Skill.TrainedValue=I(9); C.LearnedSkills.Add(Skill); C.LearnedSpells.Add(Content(TEXT("Spell.Fixture")));
    FLHItemInstance Item; Item.Id=Entity(S,20); Item.Definition=Content(TEXT("Item.Fixture")); Item.Quantity=I(2);
    FLHMechanicalField Field; Field.Key=TEXT("Damage"); Field.Value=N(1.5); Item.PermanentRolledValues.Add(Field); C.Inventory.Add(Item);
    FLHEquipmentBinding Binding; Binding.Item=Item.Id; Binding.Slot=ELHEquipmentSlot::MainHand; C.Equipment.Add(Binding);
    FLHGrowthAward Growth; Growth.AwardId.Value=FGuid(0,0,0,30); Growth.FromLevel=I(1); Growth.ToLevel=I(2); Growth.GrowthInputs=C.BaseAttributes;
    Growth.HealthIncrement=N(2); Growth.ManaIncrement=N(1); Growth.AttributePoints=I(1); Growth.SkillPoints=I(1); Growth.Ruleset=S.Header.Ruleset;
    FLHRngState Historical; Historical.StreamId=TEXT("RNG.Growth"); Historical.Algorithm=TEXT("LH.NormalizedRollPair"); Historical.AlgorithmRevision=1;
    Historical.State.Init(0,16); Growth.RollInputs.Add(Historical); C.GrowthAwards.Add(Growth); C.Creation.AcceptedRollInputs.Add(Historical);
    FLHAreaRecord Area; Area.Area=C.ActiveEntrance.Area;
    FLHEncounterRecord Encounter; Encounter.Life.Area=Area.Area; Encounter.Life.SpawnSlot=FGuid(0,0,0,40); Encounter.Life.LifeGeneration=3;
    Encounter.Definition=Content(TEXT("Enemy.Fixture")); Encounter.State=ELHEncounterLifeState::Dead; Encounter.CurrentHealth=N(0); Encounter.RespawnRemainingSeconds=N(5);
    Encounter.KillReward.Value=FGuid(0,0,0,41); Encounter.bRewardCommitted=true; Area.Encounters.Add(Encounter);
    FLHCorpseLootRecord Corpse; Corpse.Container=Entity(S,42); Corpse.SourceLife=Encounter.Life; Corpse.Reward=Encounter.KillReward;
    Item.Id=Entity(S,43); Corpse.RemainingItems.Add(Item); Corpse.RemainingGold=I(4); Corpse.bFinalized=true; Corpse.CleanupRemainingSeconds=N(9); Area.Corpses.Add(Corpse);
    FLHObjectRecord Object; Object.Id=Entity(S,44); Object.Definition=Content(TEXT("Item.ContainerFixture")); Object.bOpened=true; Item.Id=Entity(S,45); Object.RemainingItems.Add(Item); Area.Objects.Add(Object);
    S.World.Areas.Add(Area);
    FLHQuestRecord Quest; Quest.Quest=Content(TEXT("Quest.Fixture")); Quest.Stage=TEXT("Complete"); Quest.bAccepted=Quest.bCompleted=Quest.bRewarded=true;
    Quest.EligibleKillCount=I(2); Quest.Flags.Add(Content(TEXT("Flag.Fixture"))); Quest.TurnInClaim.Value=FGuid(0,0,0,46); S.World.Quests.Add(Quest);
    FLHBossRecord Boss; Boss.Boss=Content(TEXT("Boss.Fixture")); Boss.bDefeated=true; Boss.Marks.Add(Content(TEXT("Flag.BossMark"))); Boss.DialogueFlags.Add(Content(TEXT("Topic.Fixture"))); Boss.UniqueClaim.Value=FGuid(0,0,0,47); S.World.Bosses.Add(Boss);
    S.World.ClaimedUniqueRewards={Growth.AwardId,Quest.TurnInClaim,Boss.UniqueClaim}; S.World.UnlockedPortals.Add(Entity(S,48));
    S.Session.ManaRegenFractionalSeconds=N(0.125); S.Session.SafeRespawn.Entrance=C.ActiveEntrance; S.Session.SafeRespawn.TransformResolution=ELHValueResolution::Resolved;
    S.Session.SafeRespawn.SafeTransform=FTransform(FQuat::Identity,FVector(100,200,300),FVector(1,1,1));
    FLHRngState Rng; Rng.StreamId=TEXT("RNG.Combat.Fixture"); Rng.Algorithm=TEXT("UE.FRandomStream"); Rng.AlgorithmRevision=1; Rng.State={123,0,0,0}; S.Session.GameplayRng.Add(Rng);
    FLHCooldownRecord Timer; Timer.Owner=Entity(S,4); Timer.Ability=Content(TEXT("Attack.Fixture")); Timer.RemainingSeconds=N(0.75); S.Session.Cooldowns.Add(Timer);
    FLHRequestReceipt Receipt; Receipt.Request.Epoch=S.Session.RequestEpoch; Receipt.Request.Value=FGuid(0,0,0,50); Receipt.PayloadDigest=FString::ChrN(64,'c'); Receipt.TransactionSequence=1; S.Session.RecentRequests.Add(Receipt);
    S.Session.Diagnostics.Add(TEXT("Synthetic complete transaction"));
    return S;
}
FLHSaveCompatibility Compatibility(const FLHSaveSnapshot& S) { return {S.Header.Ruleset,S.Header.ContentRevision}; }
class FMemoryStorage final : public ILHSaveStorage
{
public:
    TMap<FString,TArray<uint8>> Files;
    struct FWrite { FGuid Id; int32 Slot; TArray<uint8> Bytes; TFunction<void(bool)> Callback; };
    TArray<FWrite> Writes;
    int32 MaxOutstanding=0;
    FString Key(const FGuid& Id, int32 Slot) { return Id.ToString()+(Slot==0 ? TEXT("A") : TEXT("B")); }
    virtual TArray<FGuid> Enumerate() override
    {
        TArray<FGuid> Result; for (const auto& Pair:Files) { FGuid Id; FGuid::Parse(Pair.Key.LeftChop(1),Id); Result.AddUnique(Id); } return Result;
    }
    virtual bool Read(const FGuid& Id, int32 Slot, TArray<uint8>& Out, FLHSaveError& Error) override
    {
        const auto* Data=Files.Find(Key(Id,Slot)); if (!Data) { Error={ELHSaveReason::NotFound,TEXT("Missing fixture generation")}; return false; }
        Out=*Data; return true;
    }
    virtual void Write(const FGuid& Id, int32 Slot, TArray<uint8> Data, TFunction<void(bool)> Callback) override
    { Writes.Add({Id,Slot,MoveTemp(Data),MoveTemp(Callback)}); MaxOutstanding=FMath::Max(MaxOutstanding,Writes.Num()); }
    void Complete(bool Success, bool Truncate=false)
    {
        FWrite W=MoveTemp(Writes[0]); Writes.RemoveAt(0);
        if (Success || Truncate) { if (Truncate) W.Bytes.SetNum(W.Bytes.Num()/2); Files.Add(Key(W.Id,W.Slot),MoveTemp(W.Bytes)); }
        W.Callback(Success);
    }
    bool Seed(const FLHSaveSnapshot& S, int32 Slot)
    { TArray<uint8> Bytes; FLHSaveError Error; if (!LHSave::Encode(S,Bytes,Error)) return false; Files.Add(Key(S.Header.CharacterId.Value,Slot),MoveTemp(Bytes)); return true; }
};
// Mutate bytes produced by the production encoder, not invented save files.
int32 FindField(const TArray<uint8>& Bytes, const ANSICHAR* Name)
{
    const int32 N=FCStringAnsi::Strlen(Name);
    for (int32 I=4; I+N<=Bytes.Num(); ++I)
        if (Bytes[I-4]==N && Bytes[I-3]==0 && Bytes[I-2]==0 && Bytes[I-1]==0 && FMemory::Memcmp(Bytes.GetData()+I,Name,N)==0) return I+N;
    return INDEX_NONE;
}
void SetU32(TArray<uint8>& B, int32 Offset, uint32 Value) { for (int32 I=0; I<4; ++I) B[Offset+I]=static_cast<uint8>(Value>>(8*I)); }
uint32 U32(const TArray<uint8>& B, int32 Offset) { uint32 V=0; for (int32 I=0; I<4; ++I) V|=uint32(B[Offset+I])<<(8*I); return V; }
void Rechecksum(TArray<uint8>& Bytes)
{
    const int32 PayloadStart=10+U32(Bytes,6);
    const FString Digest=LHSave::Sha256(MakeArrayView(Bytes).Slice(PayloadStart,Bytes.Num()-PayloadStart));
    const int32 Offset=FindField(Bytes,"PayloadChecksum")+4;
    FTCHARToUTF8 U(*Digest); FMemory::Memcpy(Bytes.GetData()+Offset,U.Get(),64);
}
}

#define LH_TEST(Class, Suffix) \
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Class, "Lighthaven.Persistence." Suffix, EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
bool Class::RunTest(const FString&)

LH_TEST(FLHPersistenceRoundTrip,"RoundTrip")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); TArray<uint8> Bytes; FLHSaveError Error; FLHSaveSnapshot Loaded;
    if (!TestTrue(TEXT("Encode populated fixture"),LHSave::Encode(S,Bytes,Error))) { AddError(Error.Detail); return false; }
    if (!TestTrue(TEXT("Decode populated fixture"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error))) { AddError(Error.Detail); return false; }
    TestEqual(TEXT("UTF-8 name retained"),Loaded.Character.DisplayName,S.Character.DisplayName);
    TestEqual(TEXT("XP debt retained"),Loaded.Character.ExperienceDebt.Value,int64(7));
    TestEqual(TEXT("Corpse reward retained"),Loaded.World.Areas[0].Corpses[0].Reward.Value,S.World.Areas[0].Corpses[0].Reward.Value);
    TestEqual(TEXT("Cooldown remainder retained"),Loaded.Session.Cooldowns[0].RemainingSeconds.Value,0.75);
    TestEqual(TEXT("RNG seed bytes retained"),Loaded.Session.GameplayRng[0].State,S.Session.GameplayRng[0].State);
    TestEqual(TEXT("Creation inputs retained"),Loaded.Character.Creation.AcceptedRollInputs.Num(),1);
    TestEqual(TEXT("Growth historical rolls retained"),Loaded.Character.GrowthAwards[0].RollInputs.Num(),1);
    TestEqual(TEXT("Equipment retained"),Loaded.Character.Equipment[0].Item.InstanceId,S.Character.Equipment[0].Item.InstanceId);
    TArray<uint8> Again; TestTrue(TEXT("Reencode"),LHSave::Encode(Loaded,Again,Error)); TestTrue(TEXT("Exact roundtrip bytes"),Bytes==Again);
    return true;
}
LH_TEST(FLHPersistenceCanonical,"CanonicalBytesAndSHA256")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); FLHSaveError Error; TArray<uint8> A,B;
    TestTrue(TEXT("First encode"),LHSave::Encode(S,A,Error)); TestTrue(TEXT("Second encode"),LHSave::Encode(S,B,Error)); TestTrue(TEXT("Deterministic bytes"),A==B);
    const int32 PayloadStart=10+U32(A,6);
    const TArray<uint8> GoldenPayloadPrefix={3,0,0,0,9,0,0,0,'C','h','a','r','a','c','t','e','r',21,0,0,0};
    TestTrue(TEXT("Golden explicit struct count/name prefix"),A.Num()>=PayloadStart+GoldenPayloadPrefix.Num() && FMemory::Memcmp(A.GetData()+PayloadStart,GoldenPayloadPrefix.GetData(),GoldenPayloadPrefix.Num())==0);
    const int32 GuidOffset=FindField(A,"CharacterId")+4+4+5; // one-field struct, byte length, Value token.
    const TArray<uint8> GoldenGuid={1,0,0,0,2,0,0,0,3,0,0,0,4,0,0,0};
    TestTrue(TEXT("Golden GUID words little endian"),A.Num()>=GuidOffset+16 && FMemory::Memcmp(A.GetData()+GuidOffset,GoldenGuid.GetData(),16)==0);
    const int32 Coordinates=FindField(A,"TranslationXYZ");
    TestEqual(TEXT("Golden translation fixed array count"),U32(A,Coordinates),uint32(3));
    const uint8 GoldenDouble[8]={0,0,0,0,0,0,0x59,0x40}; // binary64 100.0, little endian.
    TestTrue(TEXT("Golden binary64 translation bytes"),FMemory::Memcmp(A.GetData()+Coordinates+4,GoldenDouble,8)==0);
    Algo::Reverse(S.World.ClaimedUniqueRewards);
    TestTrue(TEXT("Permuted set encode"),LHSave::Encode(S,B,Error)); TestTrue(TEXT("Set permutation is canonical"),A==B);
    TestEqual(TEXT("SHA256 empty vector"),LHSave::Sha256(TArray<uint8>()),FString(TEXT("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")));
    TArray<uint8> Abc={'a','b','c'};
    TestEqual(TEXT("SHA256 abc vector"),LHSave::Sha256(Abc),FString(TEXT("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")));
    TArray<uint8> Long; Long.Init('a',1000000);
    TestEqual(TEXT("SHA256 multi-block million-a vector"),LHSave::Sha256(Long),FString(TEXT("cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0")));
    auto Normalized=Fixture(); Normalized.World.Areas[0].Corpses[0].CleanupRemainingSeconds.Resolution=ELHValueResolution::Unresolved;
    Normalized.World.Areas[0].Corpses[0].CleanupRemainingSeconds.Value=123;
    TArray<uint8> UnresolvedA,UnresolvedB; TestTrue(TEXT("Encode unresolved optional timer"),LHSave::Encode(Normalized,UnresolvedA,Error));
    Normalized.World.Areas[0].Corpses[0].CleanupRemainingSeconds.Value=0;
    TestTrue(TEXT("Encode unresolved zero storage"),LHSave::Encode(Normalized,UnresolvedB,Error)); TestTrue(TEXT("Unresolved storage ignored"),UnresolvedA==UnresolvedB);
    Normalized=Fixture(); Normalized.Character.CurrentMana.Value=-0.0;
    TestTrue(TEXT("Encode negative zero"),LHSave::Encode(Normalized,UnresolvedA,Error)); Normalized.Character.CurrentMana.Value=0.0;
    TestTrue(TEXT("Encode positive zero"),LHSave::Encode(Normalized,UnresolvedB,Error)); TestTrue(TEXT("Zero sign canonical"),UnresolvedA==UnresolvedB);
    Normalized=Fixture(); Normalized.Session.SafeRespawn.SafeTransform.SetRotation(FQuat(0,0,0,-1));
    TestTrue(TEXT("Encode negative quaternion sign"),LHSave::Encode(Normalized,UnresolvedA,Error));
    Normalized.Session.SafeRespawn.SafeTransform.SetRotation(FQuat::Identity);
    TestTrue(TEXT("Encode positive quaternion sign"),LHSave::Encode(Normalized,UnresolvedB,Error)); TestTrue(TEXT("Quaternion sign canonical"),UnresolvedA==UnresolvedB);
    // Optional artifact generated by the actual encoder during native tests, never a hand-written playable save.
    const FString Directory=FPaths::ProjectSavedDir()/TEXT("Automation/Persistence");
    IPlatformFile& Files=FPlatformFileManager::Get().GetPlatformFile();
    TestTrue(TEXT("Create fixture artifact directory"),Files.CreateDirectoryTree(*Directory));
    TestTrue(TEXT("Emit encoder-produced fixture"),FFileHelper::SaveArrayToFile(A,*(Directory/TEXT("populated-current.lhs"))));
    TestTrue(TEXT("Emit fixture SHA256"),FFileHelper::SaveStringToFile(LHSave::Sha256(A),*(Directory/TEXT("populated-current.sha256"))));
    return true;
}
LH_TEST(FLHPersistenceChecksumFallback,"BadChecksumFallback")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); auto Disk=MakeShared<FMemoryStorage>(); auto Store=MakeShared<FLHSaveStore>(Disk);
    if (!TestTrue(TEXT("Seed A"),Disk->Seed(S,0))) return false;
    S.Header.TransactionSequence=2; S.Character.Gold=I(101); if (!TestTrue(TEXT("Seed B"),Disk->Seed(S,1))) return false;
    auto& B=Disk->Files.FindChecked(Disk->Key(S.Header.CharacterId.Value,1)); B.Last()^=1;
    int32 Recovered=0; Store->Events.AddLambda([&Recovered](const FLHSaveEvent& E) { if (E.Kind==ELHSaveEventKind::Recovered && E.Error.Reason==ELHSaveReason::BadChecksum) ++Recovered; });
    FLHSaveSnapshot Loaded; FLHSaveError Error;
    TestTrue(TEXT("Load previous validated generation"),Store->Load(S.Header.CharacterId,Compatibility(S),Loaded,Error));
    TestEqual(TEXT("Sequence rolls back coherently"),Loaded.Header.TransactionSequence,int64(1)); TestEqual(TEXT("Gold rolls back with world"),Loaded.Character.Gold.Value,int64(100));
    TestEqual(TEXT("Recovery visible"),Recovered,1); TestTrue(TEXT("Bad generation retained"),Disk->Files.Contains(Disk->Key(S.Header.CharacterId.Value,1)));
    return true;
}
LH_TEST(FLHPersistenceInterrupted,"InterruptedWriteFallback")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); auto Disk=MakeShared<FMemoryStorage>(); auto Store=MakeShared<FLHSaveStore>(Disk); FLHSaveError Error;
    if (!TestTrue(TEXT("Seed previous"),Disk->Seed(S,0))) return false;
    S.Header.TransactionSequence=2; TestTrue(TEXT("Start save"),Store->RequestSave(S,Compatibility(S),true,Error));
    if (!TestEqual(TEXT("One write"),Disk->Writes.Num(),1)) return false;
    Disk->Complete(false,true); TestTrue(TEXT("Failed snapshot stays dirty"),Store->IsDirty(S.Header.CharacterId));
    FLHSaveSnapshot Loaded; int32 Recovered=0;
    Store->Events.AddLambda([&Recovered](const FLHSaveEvent& E) { if (E.Kind==ELHSaveEventKind::Recovered) ++Recovered; });
    TestTrue(TEXT("Load after interrupted write"),Store->Load(S.Header.CharacterId,Compatibility(S),Loaded,Error));
    TestEqual(TEXT("Previous sequence retained"),Loaded.Header.TransactionSequence,int64(1)); TestEqual(TEXT("Recovery event"),Recovered,1);
    return true;
}
LH_TEST(FLHPersistenceFuture,"FutureVersionAndAlgorithms")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); TArray<uint8> Bytes; FLHSaveError Error; FLHSaveSnapshot Loaded;
    if (!TestTrue(TEXT("Encode fixture"),LHSave::Encode(S,Bytes,Error))) return false;
    const auto Original=Bytes; SetU32(Bytes,FindField(Bytes,"SchemaVersion"),3); FLHSaveDecodeStats Stats;
    TestFalse(TEXT("Future schema rejects"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error,&Stats));
    TestTrue(TEXT("Future reason"),Error.Reason==ELHSaveReason::FutureSchema); TestFalse(TEXT("No payload allocations"),Stats.bPayloadAllocationStarted);
    Bytes=Original; SetU32(Bytes,FindField(Bytes,"SchemaVersion"),0);
    TestFalse(TEXT("Unsupported schema rejects"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestTrue(TEXT("No ambiguous migration"),Error.Reason==ELHSaveReason::UnsupportedSchema);
    Bytes=Original; Bytes[FindField(Bytes,"ChecksumAlgorithm")+4]='X';
    TestFalse(TEXT("Unknown algorithm rejects"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestTrue(TEXT("Algorithm reason"),Error.Reason==ELHSaveReason::UnknownAlgorithm);
    Bytes=Original; Bytes[FindField(Bytes,"PayloadCodec")+4]='X';
    TestFalse(TEXT("Unknown codec rejects"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestTrue(TEXT("Codec reason"),Error.Reason==ELHSaveReason::UnknownCodec);
    return true;
}
LH_TEST(FLHPersistenceBounds,"BoundsBeforeAllocation")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); TArray<uint8> Bytes; FLHSaveError Error; FLHSaveSnapshot Loaded; FLHSaveDecodeStats Stats;
    if (!TestTrue(TEXT("Encode fixture"),LHSave::Encode(S,Bytes,Error))) return false;
    const auto Original=Bytes;
    auto NulName=S; NulName.Character.DisplayName=TEXT("Namehidden"); NulName.Character.DisplayName.GetCharArray().Insert(TCHAR(0),4);
    TestEqual(TEXT("Embedded NUL character name input length"),NulName.Character.DisplayName.Len(),11);
    TestEqual(TEXT("Embedded NUL character name input character"),NulName.Character.DisplayName[4],TCHAR(0));
    TestFalse(TEXT("Embedded NUL character name encode rejects"),LHSave::Encode(NulName,Bytes,Error));
    TestTrue(TEXT("Embedded NUL character name reason"),Error.Reason==ELHSaveReason::Malformed);
    auto NulDiagnostic=S; FString Diagnostic=TEXT("visiblehidden"); Diagnostic.GetCharArray().Insert(TCHAR(0),7);
    TestEqual(TEXT("Embedded NUL diagnostic input length"),Diagnostic.Len(),14);
    TestEqual(TEXT("Embedded NUL diagnostic input character"),Diagnostic[7],TCHAR(0));
    NulDiagnostic.Session.Diagnostics.Add(Diagnostic);
    TestFalse(TEXT("Embedded NUL diagnostic encode rejects"),LHSave::Encode(NulDiagnostic,Bytes,Error));
    TestTrue(TEXT("Embedded NUL diagnostic reason"),Error.Reason==ELHSaveReason::Malformed);
    Bytes=Original;
    SetU32(Bytes,6,LHSave::MaxHeaderBytes+1);
    TestFalse(TEXT("Oversize header"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error,&Stats));
    TestTrue(TEXT("Header oversize reason"),Error.Reason==ELHSaveReason::Oversize); TestFalse(TEXT("No payload allocation"),Stats.bPayloadAllocationStarted);
    Bytes=Original; SetU32(Bytes,FindField(Bytes,"Inventory"),513); Rechecksum(Bytes);
    TestFalse(TEXT("Collection cap before allocation"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error,&Stats));
    TestTrue(TEXT("Collection reason"),Error.Reason==ELHSaveReason::Oversize); TestFalse(TEXT("No collection allocated"),Stats.bPayloadAllocationStarted);
    Bytes=Original; SetU32(Bytes,FindField(Bytes,"Inventory"),0xffffffff); Rechecksum(Bytes);
    TestFalse(TEXT("Hostile unsigned count"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error,&Stats)); TestFalse(TEXT("No count multiplication allocation"),Stats.bPayloadAllocationStarted);
    Bytes=Original; SetU32(Bytes,FindField(Bytes,"DisplayName"),129); Rechecksum(Bytes);
    TestFalse(TEXT("Display string cap"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error,&Stats)); TestFalse(TEXT("No payload allocation for oversized string"),Stats.bPayloadAllocationStarted);
    Bytes=Original; SetU32(Bytes,FindField(Bytes,"PayloadLengthBytes"),LHSave::MaxPayloadBytes+1);
    TestFalse(TEXT("Payload envelope cap"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error,&Stats)); TestFalse(TEXT("No payload allocation for envelope cap"),Stats.bPayloadAllocationStarted);
    Bytes=Original; auto Limited=Compatibility(S); Limited.MaxGrowthAwards=0;
    TestFalse(TEXT("Ruleset transition cap before allocation"),LHSave::Decode(Bytes,S.Header.CharacterId,Limited,Loaded,Error,&Stats));
    TestTrue(TEXT("Ruleset collection bound reason"),Error.Reason==ELHSaveReason::Oversize); TestFalse(TEXT("No allocation for ruleset growth cap"),Stats.bPayloadAllocationStarted);
    return true;
}
LH_TEST(FLHPersistenceBothBad,"BothSlotsBadVisible")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); auto Disk=MakeShared<FMemoryStorage>(); auto Store=MakeShared<FLHSaveStore>(Disk);
    if (!TestTrue(TEXT("Seed A"),Disk->Seed(S,0)) || !TestTrue(TEXT("Seed B"),Disk->Seed(S,1))) return false;
    for (auto& Pair:Disk->Files) Pair.Value.SetNum(5);
    int32 Unreadable=0; Store->Events.AddLambda([&Unreadable](const FLHSaveEvent& E) { if (E.Kind==ELHSaveEventKind::Unreadable) ++Unreadable; });
    FLHSaveSnapshot Loaded; Loaded.Character.DisplayName=TEXT("Do not overwrite"); FLHSaveError Error;
    TestFalse(TEXT("Both bad rejects"),Store->Load(S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestEqual(TEXT("Visible error event"),Unreadable,1);
    TestEqual(TEXT("No silent replacement character"),Loaded.Character.DisplayName,FString(TEXT("Do not overwrite")));
    TestEqual(TEXT("Files kept"),Disk->Files.Num(),2);
    const auto Profiles=Store->Enumerate(Compatibility(S)); TestEqual(TEXT("Bad character still enumerated"),Profiles.Num(),1); if (!Profiles.IsEmpty()) TestFalse(TEXT("Continue disabled"),Profiles[0].bReadable);
    TestFalse(TEXT("Save cannot replace unreadable character"),Store->RequestSave(S,Compatibility(S),true,Error)); TestEqual(TEXT("No disk write on both bad"),Disk->Writes.Num(),0);
    return true;
}
LH_TEST(FLHPersistenceEpoch,"RequestEpochMismatch")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); TArray<uint8> Bytes; FLHSaveError Error; FLHSaveSnapshot Loaded;
    if (!TestTrue(TEXT("Encode fixture"),LHSave::Encode(S,Bytes,Error))) return false;
    const int32 Offset=FindField(Bytes,"Epoch"); Bytes[Offset]^=1; Rechecksum(Bytes);
    TestFalse(TEXT("Receipt/session mismatch rejects on load"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestTrue(TEXT("Epoch mismatch reason"),Error.Reason==ELHSaveReason::EpochMismatch);
    S.Session.RecentRequests[0].Request.Epoch=FGuid(1,1,1,1);
    TestFalse(TEXT("Mismatch cannot be written"),LHSave::Encode(S,Bytes,Error)); TestTrue(TEXT("Write epoch reason"),Error.Reason==ELHSaveReason::EpochMismatch);
    return true;
}
LH_TEST(FLHPersistenceQueue,"CoalescingFailureRetryAndImmutability")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); auto Disk=MakeShared<FMemoryStorage>(); auto Store=MakeShared<FLHSaveStore>(Disk); FLHSaveError Error;
    if (!TestTrue(TEXT("Seed A"),Disk->Seed(S,0))) return false;
    int32 Failures=0,Successes=0;
    Store->Events.AddLambda([&](const FLHSaveEvent& E) { if (E.Kind==ELHSaveEventKind::Failed) ++Failures; if (E.Kind==ELHSaveEventKind::Succeeded) ++Successes; });
    S.Character.Gold=I(101); TestTrue(TEXT("Start immutable snapshot"),Store->RequestSave(S,Compatibility(S),true,Error));
    S.Character.Gold=I(102); TestTrue(TEXT("Queue dirty"),Store->RequestSave(S,Compatibility(S),true,Error));
    S.Character.Gold=I(103); TestTrue(TEXT("Coalesce latest"),Store->RequestSave(S,Compatibility(S),true,Error)); S.Character.Gold=I(999);
    FLHSaveSnapshot Loaded;
    TestFalse(TEXT("Load blocked while write in flight"),Store->Load(S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestTrue(TEXT("Busy reason"),Error.Reason==ELHSaveReason::Busy);
    if (!TestEqual(TEXT("One initial write"),Disk->Writes.Num(),1)) return false;
    TestTrue(TEXT("Active write unaffected by later snapshot"),LHSave::Decode(Disk->Writes[0].Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestEqual(TEXT("Active gold"),Loaded.Character.Gold.Value,int64(101));
    Disk->Complete(true);
    if (!TestEqual(TEXT("Coalesced write begins"),Disk->Writes.Num(),1)) return false;
    TestTrue(TEXT("Queued immutable snapshot"),LHSave::Decode(Disk->Writes[0].Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestEqual(TEXT("Latest coalesced gold"),Loaded.Character.Gold.Value,int64(103));
    TestEqual(TEXT("Sequence increases"),Loaded.Header.TransactionSequence,int64(3));
    Disk->Complete(false,true); TestTrue(TEXT("Failure retains dirty"),Store->IsDirty(S.Header.CharacterId)); TestEqual(TEXT("Failure visible"),Failures,1);
    TestTrue(TEXT("Previous valid load survives"),Store->Load(S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestEqual(TEXT("Previous gold"),Loaded.Character.Gold.Value,int64(101));
    TestTrue(TEXT("Explicit retry"),Store->Retry(S.Header.CharacterId,Error)); if (!TestEqual(TEXT("Retry one write"),Disk->Writes.Num(),1)) return false;
    Disk->Complete(true); TestFalse(TEXT("Successful retry clears dirty"),Store->IsDirty(S.Header.CharacterId));
    TestTrue(TEXT("Load latest successful"),Store->Load(S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestEqual(TEXT("Coalesced gold durable"),Loaded.Character.Gold.Value,int64(103));
    TestEqual(TEXT("Single in-flight maximum"),Disk->MaxOutstanding,1); TestEqual(TEXT("Two success events"),Successes,2);
    TestFalse(TEXT("Save requires completed boundary"),Store->RequestSave(S,Compatibility(S),false,Error)); TestTrue(TEXT("Boundary reason"),Error.Reason==ELHSaveReason::BoundaryRequired);
    return true;
}
LH_TEST(FLHPersistenceMalformed,"MalformedAndCompatibility")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); TArray<uint8> Bytes; FLHSaveError Error; FLHSaveSnapshot Loaded;
    if (!TestTrue(TEXT("Encode fixture"),LHSave::Encode(S,Bytes,Error))) return false;
    const auto Original=Bytes;
    // A field name changed to another of equal length must reject, never fall back to defaults.
    const int32 Field=FindField(Bytes,"CurrentMana")-FCStringAnsi::Strlen("CurrentMana"); Bytes[Field]='X'; Rechecksum(Bytes);
    TestFalse(TEXT("Unknown field rejects"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestTrue(TEXT("Malformed reason"),Error.Reason==ELHSaveReason::Malformed);
    Bytes=Original; const int32 Duplicate=FindField(Bytes,"LearnedSpells")-13;
    FMemory::Memcpy(Bytes.GetData()+Duplicate,"LearnedSkills",13); Rechecksum(Bytes);
    TestFalse(TEXT("Duplicate declared field rejects"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error));
    Bytes=Original; const int32 Name=FindField(Bytes,"DisplayName")+4; Bytes[Name]=0xc0; Rechecksum(Bytes);
    TestFalse(TEXT("Malformed UTF-8 rejects"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error));
    Bytes=Original; Bytes.Add(0);
    TestFalse(TEXT("Trailing bytes reject"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility(S),Loaded,Error));
    auto C=Compatibility(S); C.ContentRevision=FString::ChrN(64,'d');
    TestFalse(TEXT("Changed catalog requires compatibility decision"),LHSave::Decode(Original,S.Header.CharacterId,C,Loaded,Error)); TestTrue(TEXT("Catalog mismatch"),Error.Reason==ELHSaveReason::IncompatibleContent);
    C=Compatibility(S); ++C.Ruleset.Revision;
    TestFalse(TEXT("Changed mechanical rules require migration"),LHSave::Decode(Original,S.Header.CharacterId,C,Loaded,Error)); TestTrue(TEXT("Rules mismatch"),Error.Reason==ELHSaveReason::IncompatibleRuleset);
    C=Compatibility(S); C.ValidateReferences=[](const FLHSaveSnapshot&,FLHSaveError& E) { E={ELHSaveReason::InvalidSnapshot,TEXT("Synthetic registry rejected missing definition")}; return false; };
    TestFalse(TEXT("Authoritative registry can reject mandatory references"),LHSave::Decode(Original,S.Header.CharacterId,C,Loaded,Error));
    // TArray rejects arguments that alias its own storage, even if capacity is available.
    const FLHItemInstance DuplicateItem=S.Character.Inventory[0];
    S.Character.Inventory.Add(DuplicateItem); TestFalse(TEXT("Duplicate inventory key rejects"),LHSave::Encode(S,Bytes,Error));
    TestTrue(TEXT("Duplicate inventory key reason"),Error.Reason==ELHSaveReason::InvalidSnapshot);
    return true;
}
LH_TEST(FLHPersistenceLocalStorage,"LocalStorageBoundsAndEnumeration")
{
    using namespace LHPersistenceTestsPrivate;
    const FString Root=FPaths::ProjectSavedDir()/TEXT("Automation/Persistence")/FGuid::NewGuid().ToString(EGuidFormats::Digits);
    IPlatformFile& Files=FPlatformFileManager::Get().GetPlatformFile();
    if (!TestTrue(TEXT("Create isolated storage"),Files.CreateDirectoryTree(*Root))) return false;
    auto Storage=LHCreateLocalSaveStorage(Root); auto S=Fixture(); FLHSaveError Error; TArray<uint8> Bytes;
    if (!TestTrue(TEXT("Encode"),LHSave::Encode(S,Bytes,Error))) return false;
    const FString Filename=Root/(S.Header.CharacterId.Value.ToString(EGuidFormats::Digits)+TEXT(".A.lhs"));
    TestTrue(TEXT("Write production encoder fixture"),FFileHelper::SaveArrayToFile(Bytes,*Filename));
    TestEqual(TEXT("Enumerate stable character"),Storage->Enumerate().Num(),1);
    TArray<uint8> Loaded; TestTrue(TEXT("Bounded read"),Storage->Read(S.Header.CharacterId.Value,0,Loaded,Error)); TestTrue(TEXT("Exact file bytes"),Loaded==Bytes);
    TUniquePtr<IFileHandle> Handle(Files.OpenWrite(*Filename));
    if (TestTrue(TEXT("Open oversize fixture"),Handle.IsValid()))
    {
        uint8 Byte=0; TestTrue(TEXT("Seek sparse excessive file"),Handle->Seek(LHSave::MaxFileBytes)); TestTrue(TEXT("Write last byte"),Handle->Write(&Byte,1)); Handle.Reset();
        TestFalse(TEXT("Reject file before allocating"),Storage->Read(S.Header.CharacterId.Value,0,Loaded,Error)); TestTrue(TEXT("Oversize reason"),Error.Reason==ELHSaveReason::Oversize); TestTrue(TEXT("No allocated file buffer"),Loaded.IsEmpty());
    }
    TestTrue(TEXT("Remove isolated test files"),Files.DeleteDirectoryRecursively(*Root));
    return true;
}
LH_TEST(FLHPersistenceSequences,"SequencesIndependentCharactersAndReadback")
{
    using namespace LHPersistenceTestsPrivate;
    auto S=Fixture(); auto Disk=MakeShared<FMemoryStorage>(); auto Store=MakeShared<FLHSaveStore>(Disk); FLHSaveError Error;
    if (!TestTrue(TEXT("Seed first character"),Disk->Seed(S,0))) return false;
    S.Header.TransactionSequence=2;
    TestTrue(TEXT("Start first write"),Store->RequestSave(S,Compatibility(S),true,Error));
    auto Other=S; Other.Header.CharacterId.Value=FGuid(100,200,300,400); Other.Character.DisplayName=TEXT("Other profile"); Other.Session.RequestEpoch=FGuid(100,100,100,100);
    Other.Session.RecentRequests[0].Request.Epoch=Other.Session.RequestEpoch;
    TestTrue(TEXT("Different character saves independently"),Store->RequestSave(Other,Compatibility(Other),true,Error));
    if (!TestEqual(TEXT("Two different character writes"),Disk->Writes.Num(),2)) return false;
    Disk->Complete(true); Disk->Complete(true);
    FLHSaveSnapshot Loaded;
    TestTrue(TEXT("Load independent second profile"),Store->Load(Other.Header.CharacterId,Compatibility(Other),Loaded,Error)); TestEqual(TEXT("Selected identity restored"),Loaded.Header.CharacterId.Value,Other.Header.CharacterId.Value);
    TestEqual(TEXT("Both characters enumerate"),Store->Enumerate(Compatibility(S)).Num(),2);
    // Simulated platform claims success but persists truncated data: success must not be emitted.
    TestTrue(TEXT("Request readback-tested write"),Store->RequestSave(S,Compatibility(S),true,Error));
    int32 Successes=0,Failures=0;
    Store->Events.AddLambda([&](const FLHSaveEvent& E) { if (E.Kind==ELHSaveEventKind::Succeeded) ++Successes; if (E.Kind==ELHSaveEventKind::Failed) ++Failures; });
    Disk->Complete(true,true); TestEqual(TEXT("Readback corruption is failure"),Failures,1); TestEqual(TEXT("No premature success"),Successes,0); TestTrue(TEXT("Readback failure retains dirty"),Store->IsDirty(S.Header.CharacterId));
    TestTrue(TEXT("Previous valid generation after corrupt success"),Store->Load(S.Header.CharacterId,Compatibility(S),Loaded,Error)); TestEqual(TEXT("Previous sequence"),Loaded.Header.TransactionSequence,int64(2));
    auto Exhausted=Fixture(); Exhausted.Header.CharacterId.Value=FGuid(2,2,2,2); Exhausted.Header.TransactionSequence=MAX_int64;
    if (!TestTrue(TEXT("Seed exhausted sequence"),Disk->Seed(Exhausted,0))) return false;
    TestFalse(TEXT("Exhaustion prevents wrap"),Store->RequestSave(Exhausted,Compatibility(Exhausted),true,Error)); TestTrue(TEXT("Exhaustion reason"),Error.Reason==ELHSaveReason::SequenceExhausted);
    TestEqual(TEXT("No pending disk write after exhaustion"),Disk->Writes.Num(),0);
    return true;
}

namespace LHPersistenceTestsPrivate
{
struct FLocalAsyncFixture
{
    FAutomationTestBase* Test = nullptr;
    FLHSaveSnapshot Snapshot;
    TSharedPtr<FLHSaveStore> Store;
    FString Directory;
    bool bDone = false;
    bool bSucceeded = false;
    double Start = FPlatformTime::Seconds();
};
class FWaitForLocalSave final : public IAutomationLatentCommand
{
    TSharedRef<FLocalAsyncFixture> Context;
public:
    explicit FWaitForLocalSave(TSharedRef<FLocalAsyncFixture> In) : Context(In) {}
    virtual bool Update() override
    {
        if (!Context->bDone)
        {
            if (FPlatformTime::Seconds()-Context->Start<15) return false;
            Context->Test->AddError(TEXT("Timed out awaiting local asynchronous save completion")); return true;
        }
        auto* Test=Context->Test;
        Test->TestTrue(TEXT("Local async save succeeded after readback"),Context->bSucceeded);
        // Reconstruct a new store: no cached snapshot, queue or actor state can help this load.
        auto Reopened=MakeShared<FLHSaveStore>(LHCreateLocalSaveStorage(Context->Directory)); FLHSaveSnapshot Loaded; FLHSaveError Error;
        if (Test->TestTrue(TEXT("Reopen independent store"),Reopened->Load(Context->Snapshot.Header.CharacterId,Compatibility(Context->Snapshot),Loaded,Error)))
        {
            Test->TestEqual(TEXT("Reopened character"),Loaded.Header.CharacterId.Value,Context->Snapshot.Header.CharacterId.Value);
            Test->TestEqual(TEXT("Reopened gold"),Loaded.Character.Gold.Value,int64(100));
        }
        Test->TestTrue(TEXT("Clean isolated async fixture"),FPlatformFileManager::Get().GetPlatformFile().DeleteDirectoryRecursively(*Context->Directory));
        return true;
    }
};
}
LH_TEST(FLHPersistenceLocalAsync,"LocalAsyncWriteAndReopen")
{
    using namespace LHPersistenceTestsPrivate;
    auto Context=MakeShared<FLocalAsyncFixture>(); Context->Test=this; Context->Snapshot=Fixture();
    Context->Directory=FPaths::ProjectSavedDir()/TEXT("Automation/Persistence")/FGuid::NewGuid().ToString(EGuidFormats::Digits);
    Context->Store=MakeShared<FLHSaveStore>(LHCreateLocalSaveStorage(Context->Directory));
    TWeakPtr<FLocalAsyncFixture> Weak=Context;
    Context->Store->Events.AddLambda([Weak](const FLHSaveEvent& Event)
    {
        auto C=Weak.Pin(); if (!C) return;
        C->bDone=true; C->bSucceeded=Event.Kind==ELHSaveEventKind::Succeeded;
        if (!C->bSucceeded) C->Test->AddError(Event.Error.Detail);
    });
    FLHSaveError Error;
    if (!TestTrue(TEXT("Request local asynchronous write"),Context->Store->RequestSave(Context->Snapshot,Compatibility(Context->Snapshot),true,Error))) { AddError(Error.Detail); return false; }
    AddCommand(new FWaitForLocalSave(Context));
    return true;
}
#undef LH_TEST
#endif
