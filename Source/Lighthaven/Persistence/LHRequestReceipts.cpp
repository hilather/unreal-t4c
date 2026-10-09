#include "LHSaveCodec.h"
namespace LHRequestReceiptsPrivate
{
const FLHRequestId* Request(FName Command, const UScriptStruct* Type, const void* Payload)
{
    if (!Payload) return nullptr;
#define LH_RECEIPT(T, Token) if (Command.ToString()==TEXT(Token) && Type==T::StaticStruct()) return &static_cast<const T*>(Payload)->Request;
    LH_RECEIPT(FLHCreateCharacterRequest,"CreateCharacter")
    LH_RECEIPT(FLHAllocateAttributePointsRequest,"AllocateAttributePoints")
    LH_RECEIPT(FLHEquipItemRequest,"EquipItem")
    LH_RECEIPT(FLHUseItemRequest,"UseItem")
    LH_RECEIPT(FLHTrainSkillRequest,"TrainSkill")
    LH_RECEIPT(FLHLearnSpellRequest,"LearnSpell")
    LH_RECEIPT(FLHBuyItemRequest,"BuyItem")
    LH_RECEIPT(FLHSellItemRequest,"SellItem")
    LH_RECEIPT(FLHUseAbilityRequest,"UseAbility")
    LH_RECEIPT(FLHInteractRequest,"Interact")
    LH_RECEIPT(FLHTakeLootRequest,"TakeLoot")
#undef LH_RECEIPT
    return nullptr;
}
bool DigestValid(const FString& Digest)
{
    if (Digest.Len()!=64) return false;
    for (TCHAR C:Digest) if (!((C>='0' && C<='9') || (C>='a' && C<='f'))) return false;
    return true;
}
}
bool LHSave::BeginRequest(const FLHSaveSnapshot& S,FName Command,const UScriptStruct* Type,const void* Payload,FLHCommandResult& Out,FString& Digest)
{
    Out={}; Digest.Reset();
    const auto* ID=LHRequestReceiptsPrivate::Request(Command,Type,Payload);
    if (!ID) return false;
    Out.Request=*ID;
    if (!ID->Value.IsValid() || !ID->Epoch.IsValid() || ID->Epoch!=S.Session.RequestEpoch) return false;
    Digest=RequestDigest(Command,Type,Payload);
    if (Digest.IsEmpty()) return false;
    for (const auto& R:S.Session.RecentRequests) if (R.Request.Value==ID->Value)
    {
        if (R.PayloadDigest!=Digest) { Out.Reason=ELHCommandReason::ReusedRequestId; return false; }
        Out.Disposition=ELHCommandDisposition::Accepted; Out.Reason=ELHCommandReason::None;
        Out.CommittedSequence=R.TransactionSequence; Out.bReplay=true; return false;
    }
    if (S.Session.RecentRequests.Num()>=4096 || S.Header.TransactionSequence<0 || S.Header.TransactionSequence==MAX_int64)
    { Out.Reason=ELHCommandReason::Busy; return false; }
    return true;
}
void LHSave::CommitRequest(FLHSaveSnapshot& S,const FLHRequestId& ID,const FString& Digest,FLHCommandResult& Out)
{
    Out={}; Out.Request=ID;
    if (!ID.Value.IsValid() || !ID.Epoch.IsValid() || ID.Epoch!=S.Session.RequestEpoch || !LHRequestReceiptsPrivate::DigestValid(Digest)) return;
    for (const auto& R:S.Session.RecentRequests) if (R.Request.Value==ID.Value)
    {
        if (R.PayloadDigest!=Digest) { Out.Reason=ELHCommandReason::ReusedRequestId; return; }
        Out.Disposition=ELHCommandDisposition::Accepted; Out.Reason=ELHCommandReason::None;
        Out.CommittedSequence=R.TransactionSequence; Out.bReplay=true; return;
    }
    if (S.Session.RecentRequests.Num()>=4096 || S.Header.TransactionSequence<0 || S.Header.TransactionSequence==MAX_int64)
    { Out.Reason=ELHCommandReason::Busy; return; }
    FLHRequestReceipt R; R.Request=ID; R.PayloadDigest=Digest; R.TransactionSequence=++S.Header.TransactionSequence;
    S.Session.RecentRequests.Add(R); Out.Disposition=ELHCommandDisposition::Accepted;
    Out.Reason=ELHCommandReason::None; Out.CommittedSequence=R.TransactionSequence;
}
