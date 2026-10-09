#include "Persistence/LHSaveCodec.h"
#include "Core/LHCommands.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace LHIdentityTestsPrivate
{
static FLHRequestId Request() { FLHRequestId R; R.Value=FGuid(1,2,3,4); R.Epoch=FGuid(5,6,7,8); return R; }
static FLHInteger Integer() { FLHInteger I; I.Resolution=ELHValueResolution::Resolved; I.Value=1; I.Provenance.Status=ELHProvenanceStatus::Prototype; return I; }
static FLHAttributeBlock Attributes()
{ FLHAttributeBlock A; A.Agility=A.Endurance=A.Intelligence=A.Strength=A.Wisdom=Integer(); return A; }
static FLHCreateCharacterRequest Create()
{
    FLHCreateCharacterRequest R; R.Request=Request(); R.DisplayName=TEXT("Vector"); R.PreviewToken=FGuid(9,10,11,12);
    FLHContentId A; A.Value=TEXT("Presentation.Player"); R.AppearanceIds.Add(A);
    R.Creation.AcceptedAttributes=Attributes(); R.Creation.GenerationRevision=Integer();
    R.Creation.GenerationPolicy.Value=TEXT("CreationPolicy.Prototype"); return R;
}
static FLHAllocateAttributePointsRequest Allocate()
{ FLHAllocateAttributePointsRequest R; R.Request=Request(); R.Points=Attributes(); return R; }
static FLHEquipItemRequest Equip()
{
    FLHEquipItemRequest R; R.Request=Request(); R.Item.RunId=FGuid(1,2,3,4); R.Item.InstanceId=FGuid(9,10,11,12);
    R.Item.Area.Content.Value=TEXT("Area.TempleB1"); R.Slot=ELHEquipmentSlot::MainHand; return R;
}
template<class T> static FString Digest(FName Kind,const T& R) { return LHSave::RequestDigest(Kind,T::StaticStruct(),&R); }
template<class T> static void Changed(FAutomationTestBase& Test,FName Kind,const T& Base,const T& Mutation)
{
    const FString D=Digest(Kind,Mutation);
    Test.TestEqual(TEXT("Mutation remains encodable"),D.Len(),64);
    Test.TestTrue(TEXT("Single field changes digest"),D!=Digest(Kind,Base));
}
template<class T> static void RequestMutations(FAutomationTestBase& Test,FName Kind,const T& Base)
{
    for (int32 I=0;I<8;++I)
    {
        T R=Base; uint32* Word=nullptr;
        switch (I) { case 0:Word=&R.Request.Value.A;break; case 1:Word=&R.Request.Value.B;break;
        case 2:Word=&R.Request.Value.C;break;case 3:Word=&R.Request.Value.D;break;
        case 4:Word=&R.Request.Epoch.A;break;case 5:Word=&R.Request.Epoch.B;break;
        case 6:Word=&R.Request.Epoch.C;break;default:Word=&R.Request.Epoch.D;break; }
        ++*Word; Changed(Test,Kind,Base,R);
    }
}
// Every provenance leaf participates, including evidence text that does not affect mechanics.
template<class T, class F> static void IntegerMutations(FAutomationTestBase& Test,FName Kind,const T& Base,F Select)
{
    for (int32 I=0;I<8;++I)
    {
        T R=Base; FLHInteger& V=Select(R);
        switch (I) { case 0:++V.Value;break;case 1:V.Provenance.FieldPath=TEXT("Evidence.Value");break;
        case 2:V.Provenance.Notes=TEXT("Changed");break;case 3:V.Provenance.RetrievedDate=TEXT("2026-10-08");break;
        case 4:V.Provenance.SourceBaseline=TEXT("Synthetic");break;case 5:V.Provenance.SourceUrl=TEXT("https://example.invalid");break;
        case 6:V.Provenance.Status=ELHProvenanceStatus::Disputed;break;default:V.Provenance.ValueAsRecorded=TEXT("1");break; }
        Changed(Test,Kind,Base,R);
    }
    T R=Base; Select(R).Resolution=ELHValueResolution::Unresolved;
    Test.TestTrue(TEXT("Unresolved required value rejects"),Digest(Kind,R).IsEmpty());
}
}
#define LH_ID_TEST(Class, Name) IMPLEMENT_SIMPLE_AUTOMATION_TEST(Class, "Lighthaven.Persistence." Name, EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) bool Class::RunTest(const FString& Parameters)
LH_ID_TEST(FLHRequestGolden,"Digest.GoldenAndMutations")
{
    using namespace LHIdentityTestsPrivate;
    const auto C=Create(); const auto A=Allocate(); const auto E=Equip();
    TestEqual(TEXT("Create fixed SHA256"),Digest(TEXT("CreateCharacter"),C),FString(TEXT("eb68ba16bd205d593fb83975056e24fcf244015e43053f5a5128d5d46350eba5")));
    TestEqual(TEXT("Allocate fixed SHA256"),Digest(TEXT("AllocateAttributePoints"),A),FString(TEXT("1df0db1b1c9614724111a99653fc7d62b0378470eb53f066aadb11c94c128251")));
    TestEqual(TEXT("Equip fixed SHA256"),Digest(TEXT("EquipItem"),E),FString(TEXT("5d15bb10e4c4f3c904cca613003d17ebfd3a03c14ed5afe8bb372d11abbc1778")));
    TestEqual(TEXT("Determinism"),Digest(TEXT("CreateCharacter"),C),Digest(TEXT("CreateCharacter"),C));
    TestTrue(TEXT("Distinct commands with identical request GUIDs"),Digest(TEXT("CreateCharacter"),C)!=Digest(TEXT("AllocateAttributePoints"),A) && Digest(TEXT("AllocateAttributePoints"),A)!=Digest(TEXT("EquipItem"),E));
    RequestMutations(*this,TEXT("CreateCharacter"),C); RequestMutations(*this,TEXT("AllocateAttributePoints"),A); RequestMutations(*this,TEXT("EquipItem"),E);
    for (auto Member:{&FLHAttributeBlock::Agility,&FLHAttributeBlock::Endurance,&FLHAttributeBlock::Intelligence,&FLHAttributeBlock::Strength,&FLHAttributeBlock::Wisdom})
    {
        IntegerMutations(*this,TEXT("CreateCharacter"),C,[Member](auto& R)->FLHInteger& { return R.Creation.AcceptedAttributes.*Member; });
        IntegerMutations(*this,TEXT("AllocateAttributePoints"),A,[Member](auto& R)->FLHInteger& { return R.Points.*Member; });
    }
    IntegerMutations(*this,TEXT("CreateCharacter"),C,[](auto& R)->FLHInteger& { return R.Creation.GenerationRevision; });
    auto M=C; M.DisplayName=TEXT("Vector2"); Changed(*this,TEXT("CreateCharacter"),C,M);
    M=C; M.AppearanceIds[0].Value=TEXT("Presentation.Other"); Changed(*this,TEXT("CreateCharacter"),C,M);
    for (int32 I=0;I<4;++I) { M=C; switch(I) { case 0:++M.PreviewToken.A;break;case 1:++M.PreviewToken.B;break;case 2:++M.PreviewToken.C;break;default:++M.PreviewToken.D;break; } Changed(*this,TEXT("CreateCharacter"),C,M); }
    M=C; M.Creation.GenerationPolicy.Value=TEXT("CreationPolicy.Other"); Changed(*this,TEXT("CreateCharacter"),C,M);
    FLHQuestionAnswer Q; Q.Question.Value=TEXT("Question.One"); Q.Answer.Value=TEXT("Answer.One");
    M=C; M.Creation.QuestionAnswers.Add(Q); Changed(*this,TEXT("CreateCharacter"),C,M);
    auto QBase=M; M.Creation.QuestionAnswers[0].Question.Value=TEXT("Question.Two"); Changed(*this,TEXT("CreateCharacter"),QBase,M);
    M=QBase; M.Creation.QuestionAnswers[0].Answer.Value=TEXT("Answer.Two"); Changed(*this,TEXT("CreateCharacter"),QBase,M);
    FLHRngState Roll; Roll.StreamId=TEXT("Creation.Roll"); Roll.Algorithm=TEXT("UE.FRandomStream"); Roll.AlgorithmRevision=1; Roll.State={1,0,0,0};
    M=C; M.Creation.AcceptedRollInputs.Add(Roll); Changed(*this,TEXT("CreateCharacter"),C,M);
    auto RollBase=M; M.Creation.AcceptedRollInputs[0].State[0]=2; Changed(*this,TEXT("CreateCharacter"),RollBase,M);
    M=RollBase; M.Creation.AcceptedRollInputs[0].StreamId=TEXT("Creation.Other"); Changed(*this,TEXT("CreateCharacter"),RollBase,M);
    M=RollBase; M.Creation.AcceptedRollInputs[0].AlgorithmRevision=2; TestTrue(TEXT("Invalid RNG revision"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    M=RollBase; M.Creation.AcceptedRollInputs[0].Algorithm=TEXT("Unsupported"); TestTrue(TEXT("Invalid RNG algorithm"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    for (int32 I=0;I<8;++I)
    { auto R=E; FGuid& G=I<4 ? R.Item.RunId : R.Item.InstanceId; switch(I%4) {case 0:++G.A;break;case 1:++G.B;break;case 2:++G.C;break;default:++G.D;break;} Changed(*this,TEXT("EquipItem"),E,R); }
    auto EM=E; EM.Item.Area.Content.Value=TEXT("Area.TempleB2"); Changed(*this,TEXT("EquipItem"),E,EM);
    EM=E; EM.Slot=ELHEquipmentSlot::OffHand; Changed(*this,TEXT("EquipItem"),E,EM);
    EM=E; EM.bUnequip=true; Changed(*this,TEXT("EquipItem"),E,EM);
    return true;
}
LH_ID_TEST(FLHRequestMalformed,"Digest.CanonicalAndRejection")
{
    using namespace LHIdentityTestsPrivate;
    auto C=Create(); FLHContentId Extra; Extra.Value=TEXT("Presentation.Other"); C.AppearanceIds.Add(Extra);
    auto M=C; M.AppearanceIds.Swap(0,1); TestEqual(TEXT("Set permutations canonical"),Digest(TEXT("CreateCharacter"),M),Digest(TEXT("CreateCharacter"),C));
    FLHRngState R; R.StreamId=TEXT("Creation.One"); R.Algorithm=TEXT("UE.FRandomStream"); R.AlgorithmRevision=1; R.State={0,0,0,0};
    C.Creation.AcceptedRollInputs.Add(R); R.StreamId=TEXT("Creation.Two"); C.Creation.AcceptedRollInputs.Add(R);
    M=C; M.Creation.AcceptedRollInputs.Swap(0,1); TestEqual(TEXT("Roll stream set canonical"),Digest(TEXT("CreateCharacter"),M),Digest(TEXT("CreateCharacter"),C));
    FLHQuestionAnswer Q; Q.Question.Value=TEXT("Question.One"); Q.Answer.Value=TEXT("Answer.One"); C.Creation.QuestionAnswers.Add(Q); Q.Question.Value=TEXT("Question.Two"); C.Creation.QuestionAnswers.Add(Q);
    M=C; M.Creation.QuestionAnswers.Swap(0,1); Changed(*this,TEXT("CreateCharacter"),C,M);
    TArray<uint8> Bytes; FLHSaveError Error;
    TestTrue(TEXT("Encode public bytes"),LHSave::EncodeCanonicalRequest(TEXT("CreateCharacter"),C.StaticStruct(),&C,Bytes,Error));
    TestTrue(TEXT("Canonical bytes accepted"),LHSave::ValidateCanonicalRequestBytes(TEXT("CreateCharacter"),Bytes,Error));
    auto Bad=Bytes; // Swap complete outer Command and Domain field/value records (both strings).
    const int32 CommandSize=4+7+4+15, DomainSize=4+6+4+10;
    Bad.Reset(); Bad.Append(Bytes.GetData(),4); Bad.Append(Bytes.GetData()+4+CommandSize,DomainSize); Bad.Append(Bytes.GetData()+4,CommandSize); Bad.Append(Bytes.GetData()+4+CommandSize+DomainSize,Bytes.Num()-4-CommandSize-DomainSize);
    TestFalse(TEXT("Reordered fields reject"),LHSave::ValidateCanonicalRequestBytes(TEXT("CreateCharacter"),Bad,Error));
    Bad=Bytes; Bad[8]='X'; TestFalse(TEXT("Unknown field"),LHSave::ValidateCanonicalRequestBytes(TEXT("CreateCharacter"),Bad,Error));
    Bad=Bytes; Bad.Add(0); TestFalse(TEXT("Trailing bytes"),LHSave::ValidateCanonicalRequestBytes(TEXT("CreateCharacter"),Bad,Error));
    Bad=Bytes; Bad.Pop(); TestFalse(TEXT("Truncation"),LHSave::ValidateCanonicalRequestBytes(TEXT("CreateCharacter"),Bad,Error));
    M=C; const auto Duplicate=M.AppearanceIds[0]; M.AppearanceIds.Add(Duplicate); TestTrue(TEXT("Duplicate set key"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    M=C; M.Request.Value={}; TestTrue(TEXT("Zero request"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    M=C; M.Request.Epoch={}; TestTrue(TEXT("Zero epoch"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    M=C; M.PreviewToken={}; TestTrue(TEXT("Zero token"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    M=C; M.DisplayName=FString::ChrN(129,'a'); TestTrue(TEXT("Display bound"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    M=C; M.DisplayName=TEXT("Namehidden"); M.DisplayName.GetCharArray().Insert(TCHAR(0),4);
    TestEqual(TEXT("Embedded NUL canonical input length"),M.DisplayName.Len(),11);
    TestEqual(TEXT("Embedded NUL canonical input character"),M.DisplayName[4],TCHAR(0));
    TestTrue(TEXT("Embedded NUL rejects"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    TestFalse(TEXT("Embedded NUL canonical encode rejects"),LHSave::EncodeCanonicalRequest(TEXT("CreateCharacter"),M.StaticStruct(),&M,Bytes,Error));
    TestTrue(TEXT("Embedded NUL canonical reason"),Error.Reason==ELHSaveReason::Malformed);
    M=C; M.DisplayName.AppendChar(static_cast<TCHAR>(0xd800)); TestTrue(TEXT("Malformed native Unicode rejects"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    M=C; M.DisplayName=TEXT("\ufeffVector"); TestTrue(TEXT("BOM rejects"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    M=C; M.AppearanceIds.SetNum(17); TestTrue(TEXT("Appearance bound"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    M=C; M.Creation.QuestionAnswers.SetNum(5); TestTrue(TEXT("Answer bound"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    M=C; M.AppearanceIds[0].Value=TEXT("bad/id"); TestTrue(TEXT("ID grammar"),Digest(TEXT("CreateCharacter"),M).IsEmpty());
    TestTrue(TEXT("Type mismatch"),LHSave::RequestDigest(TEXT("EquipItem"),C.StaticStruct(),&C).IsEmpty());
    TestTrue(TEXT("TrainSkill mismatched type"),LHSave::RequestDigest(TEXT("TrainSkill"),C.StaticStruct(),&C).IsEmpty());
    TestTrue(TEXT("Null"),LHSave::RequestDigest(TEXT("CreateCharacter"),C.StaticStruct(),nullptr).IsEmpty());
    auto A=Allocate(); A.Points.Strength.Value=MAX_int64; TestTrue(TEXT("Allocation overflow"),Digest(TEXT("AllocateAttributePoints"),A).IsEmpty());
    auto E=Equip(); E.Slot=static_cast<ELHEquipmentSlot>(255); TestTrue(TEXT("Invalid slot enum"),Digest(TEXT("EquipItem"),E).IsEmpty());
    return true;
}
LH_ID_TEST(FLHRewardVectors,"RewardId.GoldenAndRejection")
{
    FGuid Run(1,2,3,4); FLHCharacterId Character; Character.Value=FGuid(5,6,7,8);
    FLHSaveError Error; FLHRewardId Growth,Enemy,Other;
    TestTrue(TEXT("Growth maps"),LHSave::GrowthRewardId(Run,Character,2,Growth,Error));
    TestEqual(TEXT("Growth fixed GUID words"),Growth.Value,FGuid(0x889a7581,0xfc986cfa,0x8c80ea64,0xf23f1782));
    TestEqual(TEXT("Callback deterministic"),LHSave::GrowthId(Run,Character,2).Value,Growth.Value);
    FLHSpawnLifeId Life; Life.Area.Content.Value=TEXT("Area.TempleB1"); Life.SpawnSlot=FGuid(5,6,7,8);
    TestTrue(TEXT("Enemy maps"),LHSave::EnemyLifeRewardId(Run,Life,Enemy,Error));
    TestEqual(TEXT("Enemy fixed GUID words"),Enemy.Value,FGuid(0x7065279d,0x56081e61,0xe6e30945,0x0e20e19e));
    LHSave::EnemyLifeRewardId(Run,Life,Other,Error); TestEqual(TEXT("Enemy deterministic"),Other.Value,Enemy.Value);
    TestTrue(TEXT("Separate source domains"),Growth.Value!=Enemy.Value);
    LHSave::GrowthRewardId(FGuid(2,2,3,4),Character,2,Other,Error); TestTrue(TEXT("Run scoped"),Other.Value!=Growth.Value);
    auto C=Character; ++C.Value.A; LHSave::GrowthRewardId(Run,C,2,Other,Error); TestTrue(TEXT("Character scoped"),Other.Value!=Growth.Value);
    LHSave::GrowthRewardId(Run,Character,3,Other,Error); TestTrue(TEXT("Level scoped"),Other.Value!=Growth.Value);
    auto L=Life; L.Area.Content.Value=TEXT("Area.TempleB2"); LHSave::EnemyLifeRewardId(Run,L,Other,Error); TestTrue(TEXT("Area scoped"),Other.Value!=Enemy.Value);
    L=Life; ++L.SpawnSlot.A; LHSave::EnemyLifeRewardId(Run,L,Other,Error); TestTrue(TEXT("Slot scoped"),Other.Value!=Enemy.Value);
    L=Life; ++L.LifeGeneration; LHSave::EnemyLifeRewardId(Run,L,Other,Error); TestTrue(TEXT("Generation scoped"),Other.Value!=Enemy.Value);
    TestFalse(TEXT("Zero digest mapping"),LHSave::RewardIdFromDigest(FString::ChrN(64,'0'),Other,Error)); TestFalse(TEXT("No output on failure"),Other.Value.IsValid());
    TestFalse(TEXT("Zero prefix with nonzero tail still rejects"),LHSave::RewardIdFromDigest(FString::ChrN(32,'0')+FString::ChrN(32,'f'),Other,Error));
    TestFalse(TEXT("Malformed hash"),LHSave::RewardIdFromDigest(TEXT("invalid"),Other,Error));
    TestFalse(TEXT("Zero run"),LHSave::GrowthRewardId({},Character,2,Other,Error));
    TestFalse(TEXT("Zero character"),LHSave::GrowthRewardId(Run,{},2,Other,Error));
    TestFalse(TEXT("Invalid growth level"),LHSave::GrowthRewardId(Run,Character,1,Other,Error));
    L=Life; L.LifeGeneration=-1; TestFalse(TEXT("Negative generation"),LHSave::EnemyLifeRewardId(Run,L,Other,Error));
    L=Life; L.SpawnSlot={}; TestFalse(TEXT("Zero slot"),LHSave::EnemyLifeRewardId(Run,L,Other,Error));
    L=Life; L.Area.Content.Value=TEXT("Area.Frontend"); TestFalse(TEXT("Unplayable area"),LHSave::EnemyLifeRewardId(Run,L,Other,Error));
    return true;
}
#undef LH_ID_TEST
#endif
