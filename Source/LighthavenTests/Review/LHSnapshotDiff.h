#pragma once
#include "Framework/LHWave2Session.h"
#include "UObject/UnrealType.h"
#include "Misc/AutomationTest.h"

// Diagnostic only: recursively report reflected leaves, including array lengths.
// The production codec remains the equality oracle in callers.
namespace LHSnapshotDiff
{
inline void Property(FAutomationTestBase& Test,const FProperty* P,const void* A,const void* B,const FString& Path)
{
    if (P->Identical(A,B,PPF_None)) return;
    if (const auto* S=CastField<FStructProperty>(P))
    {
        bool Reflected=false;
        for (TFieldIterator<FProperty> It(S->Struct);It;++It)
        {
            Reflected=true;
            Property(Test,*It,It->ContainerPtrToValuePtr<void>(A),It->ContainerPtrToValuePtr<void>(B),Path+TEXT(".")+It->GetName());
        }
        if (Reflected) return; // Native structs such as FTransform use ExportText below.
    }
    if (const auto* Array=CastField<FArrayProperty>(P))
    {
        FScriptArrayHelper X(Array,A),Y(Array,B);
        if(X.Num()!=Y.Num()) Test.AddInfo(FString::Printf(TEXT("F9 diff %s.Num: %d -> %d"),*Path,X.Num(),Y.Num()));
        for(int32 I=0;I<FMath::Min(X.Num(),Y.Num());++I)
            Property(Test,Array->Inner,X.GetRawPtr(I),Y.GetRawPtr(I),Path+FString::Printf(TEXT("[%d]"),I));
        for(int32 I=FMath::Min(X.Num(),Y.Num());I<FMath::Max(X.Num(),Y.Num());++I)
        {
            FString Value; const bool Removed=I<X.Num();
            Array->Inner->ExportTextItem_Direct(Value,Removed?X.GetRawPtr(I):Y.GetRawPtr(I),nullptr,nullptr,PPF_None);
            Test.AddInfo(FString::Printf(TEXT("F9 diff %s[%d] %s: %s"),*Path,I,Removed?TEXT("removed"):TEXT("added"),*Value));
        }
        return;
    }
    if (const auto* Number=CastField<FDoubleProperty>(P))
    {
        Test.AddInfo(FString::Printf(TEXT("F9 diff %s: %.17g -> %.17g"),*Path,Number->GetPropertyValue(A),Number->GetPropertyValue(B))); return;
    }
    FString X,Y; P->ExportTextItem_Direct(X,A,nullptr,nullptr,PPF_None); P->ExportTextItem_Direct(Y,B,nullptr,nullptr,PPF_None);
    Test.AddInfo(FString::Printf(TEXT("F9 diff %s: %s -> %s"),*Path,*X,*Y));
}
inline void Snapshot(FAutomationTestBase& Test,const FLHSaveSnapshot& A,const FLHSaveSnapshot& B,const FString& Label)
{
    FLHSaveSnapshot X,Y; TArray<uint8> Bytes; FLHSaveError Error;
    auto Normalize=[&](const FLHSaveSnapshot& S,FLHSaveSnapshot& Out) {
        return LHSave::Encode(S,Bytes,Error) && LHSave::Decode(Bytes,S.Header.CharacterId,FLHWave2Session::Compatibility(),Out,Error);
    };
    if (!Normalize(A,X) || !Normalize(B,Y))
    { Test.AddError(TEXT("Snapshot diff normalization failed: ")+Error.Detail); return; }
    // Payload checksum is a consequence of changed leaves, not another cause.
    X.Header.PayloadChecksum=Y.Header.PayloadChecksum; X.Header.PayloadLengthBytes=Y.Header.PayloadLengthBytes;
    for(TFieldIterator<FProperty> It(FLHSaveSnapshot::StaticStruct());It;++It)
        Property(Test,*It,It->ContainerPtrToValuePtr<void>(&X),It->ContainerPtrToValuePtr<void>(&Y),Label+TEXT(".")+It->GetName());
}
}
