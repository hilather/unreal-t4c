#include "Misc/AutomationTest.h"
#include "Persistence/LHSaveCodec.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHNewCommandTestsPrivate
{
FLHRequestId Request() { FLHRequestId R; R.Value=FGuid(1,2,3,4); R.Epoch=FGuid(5,6,7,8); return R; }
FLHEntityId Entity() { FLHEntityId E; E.RunId=FGuid(9,10,11,12); E.Area.Content.Value=TEXT("Area.TempleB1"); E.InstanceId=FGuid(13,14,15,16); return E; }
FLHInteger I() { FLHInteger V; V.Resolution=ELHValueResolution::Resolved; V.Value=2; return V; }
template<class T> void Vector(FAutomationTestBase& Test,const TCHAR* Command,const T& R,const TCHAR* Expected)
{
    TArray<uint8> B; FLHSaveError E;
    Test.TestTrue(TEXT("New command canonical encode"),LHSave::EncodeCanonicalRequest(Command,T::StaticStruct(),&R,B,E));
    Test.TestTrue(TEXT("New command canonical decode"),LHSave::ValidateCanonicalRequestBytes(Command,B,E));
    const FString Digest=LHSave::RequestDigest(Command,T::StaticStruct(),&R);
    Test.AddInfo(FString(Command)+TEXT(" vector ")+Digest);
    if (FCString::Strlen(Expected)) Test.TestEqual(Command,Digest,FString(Expected));
    Test.TestTrue(TEXT("Mismatched command type rejects"),LHSave::RequestDigest(Command,FLHCreateCharacterRequest::StaticStruct(),&R).IsEmpty());
    auto Changed=R; Changed.Request.Value.D++;
    Test.TestTrue(TEXT("Request identity bound"),Digest!=LHSave::RequestDigest(Command,T::StaticStruct(),&Changed));
    B.Add(0); Test.TestFalse(TEXT("Trailing bytes reject"),LHSave::ValidateCanonicalRequestBytes(Command,B,E));
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHNewCommandDigests,"Lighthaven.Persistence.NewCommandDigests",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHNewCommandDigests::RunTest(const FString&)
{
    using namespace LHNewCommandTestsPrivate;
    FLHTrainSkillRequest Train; Train.Request=Request(); Train.Trainer=Entity(); Train.Skill.Value=TEXT("Skill.Attack"); Train.Points=I(); Vector(*this,TEXT("TrainSkill"),Train,TEXT("9e9df2e53ce852bb72f9b9685ca16a231a0d40fc7ce4b11b901b8f13f45dec66"));
    FLHLearnSpellRequest Learn; Learn.Request=Request(); Learn.Trainer=Entity(); Learn.Spell.Value=TEXT("Spell.Light"); Vector(*this,TEXT("LearnSpell"),Learn,TEXT("12a98c2e08d0d82b363f5c8ef5e4e04d258fbdc51be21b2785f157617c043606"));
    FLHBuyItemRequest Buy; Buy.Request=Request(); Buy.Vendor=Entity(); Buy.Offer.Value=TEXT("Offer.Potion"); Buy.Quantity=I(); Vector(*this,TEXT("BuyItem"),Buy,TEXT("f3260c33633b57773ddc6cb6aea914fb33f941cf36aa90f43d3e96545ee2aa1d"));
    FLHSellItemRequest Sell; Sell.Request=Request(); Sell.Vendor=Entity(); Sell.Item=Entity(); Sell.Item.InstanceId.D++; Sell.Quantity=I(); Vector(*this,TEXT("SellItem"),Sell,TEXT("05d9272085ad9763ae3574a827319935ff4f8afdb1c00c8bfbe10868a8473e48"));
    FLHUseAbilityRequest Ability; Ability.Request=Request(); Ability.Ability.Value=TEXT("Ability.FireDart"); Ability.Target=Entity(); Vector(*this,TEXT("UseAbility"),Ability,TEXT("c034c60ddc2c888ceeeb19279beaf8cbdbd301888cf056c22d205478d18002d9"));
    FLHInteractRequest Interact; Interact.Request=Request(); Interact.Target=Entity(); Interact.Topic.Value=TEXT("Topic.Heal"); Vector(*this,TEXT("Interact"),Interact,TEXT("e6be638407c94c978f82fa938fb43f17cfb5be304fd927dc60b998121c713bb6"));
    FLHTakeLootRequest Loot; Loot.Request=Request(); Loot.Container=Entity(); Loot.Item=Sell.Item; Loot.Kind=ELHLootTransferKind::Item; Loot.Quantity=I(); Vector(*this,TEXT("TakeLoot"),Loot,TEXT("4750de32ca8857421976c55bc925988132db336cbf48788cf76ecbe9a3cd4819"));
    Loot.Kind=ELHLootTransferKind::Gold; TestTrue(TEXT("Mixed gold item rejects"),LHSave::RequestDigest(TEXT("TakeLoot"),Loot.StaticStruct(),&Loot).IsEmpty());
    Loot.Item={}; TestFalse(TEXT("Gold sentinel valid"),LHSave::RequestDigest(TEXT("TakeLoot"),Loot.StaticStruct(),&Loot).IsEmpty());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHRequestReceiptReplay,"Lighthaven.Persistence.RequestReceiptReplay",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHRequestReceiptReplay::RunTest(const FString&)
{
    using namespace LHNewCommandTestsPrivate;
    FLHSaveSnapshot S; S.Header.TransactionSequence=7; S.Session.RequestEpoch=Request().Epoch;
    FLHTrainSkillRequest R; R.Request=Request(); R.Trainer=Entity(); R.Skill.Value=TEXT("Skill.Attack"); R.Points=I();
    FLHCommandResult Out; FString D;
    TestTrue(TEXT("New request begins"),LHSave::BeginRequest(S,TEXT("TrainSkill"),R.StaticStruct(),&R,Out,D));
    LHSave::CommitRequest(S,R.Request,D,Out); TestEqual(TEXT("Commit sequence"),Out.CommittedSequence,int64(8));
    S.Header.TransactionSequence=12;
    TestFalse(TEXT("Replay stops domain dispatch"),LHSave::BeginRequest(S,TEXT("TrainSkill"),R.StaticStruct(),&R,Out,D));
    TestTrue(TEXT("Replay accepted"),Out.bReplay && Out.Disposition==ELHCommandDisposition::Accepted);
    TestEqual(TEXT("Original sequence"),Out.CommittedSequence,int64(8));
    R.Points.Value++; TestFalse(TEXT("Reused id different digest"),LHSave::BeginRequest(S,TEXT("TrainSkill"),R.StaticStruct(),&R,Out,D)); TestTrue(TEXT("Structured reused reason"),Out.Reason==ELHCommandReason::ReusedRequestId);
    R.Request.Epoch.D++; TestFalse(TEXT("Wrong epoch"),LHSave::BeginRequest(S,TEXT("TrainSkill"),R.StaticStruct(),&R,Out,D)); TestFalse(TEXT("Not replay"),Out.bReplay);
    R.Request=Request(); R.Request.Value.D++; S.Header.TransactionSequence=MAX_int64;
    TestFalse(TEXT("Sequence overflow"),LHSave::BeginRequest(S,TEXT("TrainSkill"),R.StaticStruct(),&R,Out,D)); TestTrue(TEXT("Overflow busy"),Out.Reason==ELHCommandReason::Busy);
    S.Header.TransactionSequence=12; S.Session.RecentRequests.SetNum(4096);
    TestFalse(TEXT("Receipt cap"),LHSave::BeginRequest(S,TEXT("TrainSkill"),R.StaticStruct(),&R,Out,D)); TestTrue(TEXT("Cap busy"),Out.Reason==ELHCommandReason::Busy);
    const auto Before=S; LHSave::CommitRequest(S,R.Request,D,Out); TestEqual(TEXT("Cap commit leaves sequence"),S.Header.TransactionSequence,Before.Header.TransactionSequence);
    return true;
}
#endif
