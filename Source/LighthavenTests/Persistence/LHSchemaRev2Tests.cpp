#include "Misc/AutomationTest.h"
#include "Persistence/LHSaveStore.h"
#include "Core/LHCommands.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace LHSchemaRev2TestsPrivate
{
#include "LHSchemaRev1Fixture.inl"
TArray<uint8> Frozen() { TArray<uint8> B; B.Append(LHSchemaRev1Bytes,UE_ARRAY_COUNT(LHSchemaRev1Bytes)); return B; }
FLHCharacterId Character() { FLHCharacterId C; C.Value=FGuid(1,2,3,4); return C; }
FLHSaveCompatibility Compatibility()
{
    FLHSaveCompatibility C;
    C.Ruleset.Id.Value=TEXT("Ruleset.PersistenceFixture"); C.Ruleset.Revision=1;
    C.Ruleset.HashAlgorithm=TEXT("SHA256"); C.Ruleset.ContentHash=FString::ChrN(64,'a');
    C.ContentRevision=FString::ChrN(64,'b'); return C;
}
FLHUseItemRequest Request()
{
    FLHUseItemRequest R; R.Request.Value=FGuid(1,2,3,4); R.Request.Epoch=FGuid(5,6,7,8);
    R.Item.RunId=FGuid(1,2,3,4); R.Item.Area.Content.Value=TEXT("Area.TempleB1"); R.Item.InstanceId=FGuid(9,10,11,12); return R;
}
class FStorage final : public ILHSaveStorage
{
public:
    TArray<uint8> Slots[2]; int32 PendingSlot=INDEX_NONE; TArray<uint8> Pending;
    TFunction<void(bool)> Completion;
    virtual TArray<FGuid> Enumerate() override { return {Character().Value}; }
    virtual bool Read(const FGuid&,int32 Slot,TArray<uint8>& Out,FLHSaveError& E) override
    { if (Slots[Slot].IsEmpty()) { E={ELHSaveReason::NotFound,TEXT("Missing")}; return false; } Out=Slots[Slot]; return true; }
    virtual void Write(const FGuid&,int32 Slot,TArray<uint8> B,TFunction<void(bool)> C) override
    { PendingSlot=Slot; Pending=MoveTemp(B); Completion=MoveTemp(C); }
    void Complete(bool Success)
    {
        // A failed write may leave corrupt bytes in its target slot.
        Slots[PendingSlot]=Pending; if (!Success) Slots[PendingSlot].SetNum(5);
        auto C=MoveTemp(Completion); PendingSlot=INDEX_NONE; C(Success);
    }
};
}
#define LH_REV2_TEST(Class,Name) \
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Class,"Lighthaven.Persistence." Name,EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
bool Class::RunTest(const FString&)

LH_REV2_TEST(FLHUseItemDigestTest,"UseItemDigest")
{
    using namespace LHSchemaRev2TestsPrivate;
    auto R=Request(); FLHSaveError E; TArray<uint8> B; FString Digest;
    if (!TestTrue(TEXT("Encode UseItem"),LHSave::EncodeCanonicalRequest(TEXT("UseItem"),FLHUseItemRequest::StaticStruct(),&R,B,E))) return false;
    TestEqual(TEXT("Frozen request length"),B.Num(),359);
    TestTrue(TEXT("Digest"),LHSave::CanonicalRequestDigest(TEXT("UseItem"),FLHUseItemRequest::StaticStruct(),&R,Digest,E));
    TestEqual(TEXT("Frozen LHRequest1 digest"),Digest,FString(TEXT("454d96b1305633a0d79293e4b503d2a3b8c3fb1552541f8a84708e86e8449f37")));
    TestTrue(TEXT("Accept frozen wire"),LHSave::ValidateCanonicalRequestBytes(TEXT("UseItem"),B,E));
    TestFalse(TEXT("Wrong command"),LHSave::ValidateCanonicalRequestBytes(TEXT("EquipItem"),B,E));
    auto InvalidEpoch=B;
    const ANSICHAR* Epoch="Epoch";
    for (int32 I=4; I+5+16<=InvalidEpoch.Num(); ++I)
        if (InvalidEpoch[I-4]==5 && FMemory::Memcmp(InvalidEpoch.GetData()+I,Epoch,5)==0)
        { FMemory::Memzero(InvalidEpoch.GetData()+I+5,16); break; }
    TestFalse(TEXT("Invalid epoch in canonical bytes rejects"),LHSave::ValidateCanonicalRequestBytes(TEXT("UseItem"),InvalidEpoch,E));
    auto Truncated=B; Truncated.Pop();
    TestFalse(TEXT("Truncated request rejects"),LHSave::ValidateCanonicalRequestBytes(TEXT("UseItem"),Truncated,E));

    TestFalse(TEXT("Wrong declared type"),LHSave::EncodeCanonicalRequest(TEXT("UseItem"),FLHEquipItemRequest::StaticStruct(),&R,B,E));
    R.Item.InstanceId={}; TestFalse(TEXT("Invalid item"),LHSave::EncodeCanonicalRequest(TEXT("UseItem"),FLHUseItemRequest::StaticStruct(),&R,B,E));
    R=Request(); R.Request.Epoch={}; TestFalse(TEXT("Invalid epoch"),LHSave::EncodeCanonicalRequest(TEXT("UseItem"),FLHUseItemRequest::StaticStruct(),&R,B,E));
    R=Request(); R.Target.RunId=FGuid(1,2,3,4); TestFalse(TEXT("Partial target rejects"),LHSave::EncodeCanonicalRequest(TEXT("UseItem"),FLHUseItemRequest::StaticStruct(),&R,B,E));
    R=Request(); R.Target=R.Item; TestTrue(TEXT("Explicit target"),LHSave::EncodeCanonicalRequest(TEXT("UseItem"),FLHUseItemRequest::StaticStruct(),&R,B,E));
    return true;
}
LH_REV2_TEST(FLHSchemaRev1MigratesTest,"SchemaRev1Migrates")
{
    using namespace LHSchemaRev2TestsPrivate;
    auto B=Frozen(); FLHSaveSnapshot S; FLHSaveError E;
    TestEqual(TEXT("Frozen production file SHA256"),LHSave::Sha256(B),FString(LHSchemaRev1Sha256));
    if (!TestTrue(TEXT("Bounded v1 migration"),LHSave::Decode(B,Character(),Compatibility(),S,E))) { AddError(E.Detail); return false; }
    TestEqual(TEXT("Migrated schema"),S.Header.SchemaVersion,2); TestTrue(TEXT("Validated migrated snapshot"),LHSave::Validate(S,E));
    TArray<uint8> Out; if (!TestTrue(TEXT("Encode migrated v2"),LHSave::Encode(S,Out,E))) return false;
    // The complete canonical file must differ only in the schema int32.
    // This verifies ALL character/world/session facts, not a selection of fields.
    const ANSICHAR* Name="SchemaVersion"; int32 Offset=INDEX_NONE;
    for (int32 I=4; I+13<B.Num(); ++I)
        if (B[I-4]==13 && B[I-3]==0 && B[I-2]==0 && B[I-1]==0 && FMemory::Memcmp(B.GetData()+I,Name,13)==0) { Offset=I+13; break; }
    if (!TestTrue(TEXT("Schema field exists"),Offset!=INDEX_NONE)) return false;
    B[Offset]=2; TestTrue(TEXT("Exact content preservation"),B==Out);
    FLHSaveSnapshot Again; TestTrue(TEXT("Migrated v2 roundtrip"),LHSave::Decode(Out,Character(),Compatibility(),Again,E));
    B=Frozen(); B.Last()^=1; TestFalse(TEXT("v1 checksum corruption"),LHSave::Decode(B,Character(),Compatibility(),Again,E));
    TestTrue(TEXT("Checksum reason"),E.Reason==ELHSaveReason::BadChecksum);
    B=Frozen(); B.SetNum(B.Num()-1); TestFalse(TEXT("v1 truncation"),LHSave::Decode(B,Character(),Compatibility(),Again,E));
    S.Header.SchemaVersion=1; TestFalse(TEXT("Current encoder cannot emit rev1"),LHSave::Encode(S,Out,E));
    return true;
}
LH_REV2_TEST(FLHSchemaRev2RoundTripTest,"SchemaRev2RoundTrip")
{
    using namespace LHSchemaRev2TestsPrivate;
    FLHSaveSnapshot S; FLHSaveError E;
    if (!TestTrue(TEXT("Load fixture"),LHSave::Decode(Frozen(),Character(),Compatibility(),S,E))) return false;
    auto R=Request(); R.Request.Epoch=S.Session.RequestEpoch;
    FLHRequestReceipt Receipt; Receipt.Request=R.Request; Receipt.TransactionSequence=1;
    Receipt.PayloadDigest=LHSave::RequestDigest(TEXT("UseItem"),FLHUseItemRequest::StaticStruct(),&R); S.Session.RecentRequests.Add(Receipt);
    TArray<uint8> A,B; TestTrue(TEXT("Encode v2 receipt"),LHSave::Encode(S,A,E));
    FLHSaveSnapshot Loaded; if (!TestTrue(TEXT("Decode v2 receipt"),LHSave::Decode(A,Character(),Compatibility(),Loaded,E))) return false;
    TestTrue(TEXT("Reencode"),LHSave::Encode(Loaded,B,E)); TestTrue(TEXT("Exact v2 bytes"),A==B);
    S.Session.RecentRequests.Last().Request.Epoch=FGuid(90,91,92,93);
    TestFalse(TEXT("Wrong session epoch payload rejects"),LHSave::Encode(S,B,E)); TestTrue(TEXT("Epoch reason"),E.Reason==ELHSaveReason::EpochMismatch);
    return true;
}
LH_REV2_TEST(FLHMigrationOriginalSlotTest,"MigrationKeepsOriginalSlot")
{
    using namespace LHSchemaRev2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); Disk->Slots[0]=Frozen(); const auto Original=Disk->Slots[0];
    auto Store=MakeShared<FLHSaveStore>(Disk); FLHSaveSnapshot S; FLHSaveError E;
    if (!TestTrue(TEXT("Load/migrate A"),Store->Load(Character(),Compatibility(),S,E))) return false;
    TestTrue(TEXT("Load does not rewrite source"),Disk->Slots[0]==Original && Disk->Slots[1].IsEmpty());
    TestTrue(TEXT("Request v2 save"),Store->RequestSave(S,Compatibility(),true,E));
    if (!TestEqual(TEXT("Writes opposite slot"),Disk->PendingSlot,1)) return false;
    Disk->Complete(false); TestTrue(TEXT("v1 generation survives failed v2 write"),Disk->Slots[0]==Original);
    TestTrue(TEXT("Still loadable after failed migration write"),Store->Load(Character(),Compatibility(),S,E));
    TestTrue(TEXT("Retry"),Store->Retry(Character(),E)); if (!TestEqual(TEXT("Retry stays in B"),Disk->PendingSlot,1)) return false;
    Disk->Complete(true); TestTrue(TEXT("Successful v2 write retains original A"),Disk->Slots[0]==Original);
    TestTrue(TEXT("Load newer v2 generation"),Store->Load(Character(),Compatibility(),S,E)); TestEqual(TEXT("New sequence"),S.Header.TransactionSequence,int64(2));
    return true;
}
#undef LH_REV2_TEST
#endif
