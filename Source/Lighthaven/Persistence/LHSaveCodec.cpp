#include "LHSaveCodec.h"
#include "Core/LHCommands.h"
#include "Containers/StringConv.h"

namespace LHSaveCodecPrivate
{
bool Less(TConstArrayView<uint8> A, TConstArrayView<uint8> B)
{
    const int32 N = FMath::Min(A.Num(), B.Num());
    for (int32 I=0; I<N; ++I) if (A[I] != B[I]) return A[I] < B[I];
    return A.Num() < B.Num();
}

// Reject overlong encodings, surrogate scalars, BOM and invalid/truncated UTF-8.
bool Utf8(TConstArrayView<uint8> B, bool Ascii)
{
    for (int32 I=0; I<B.Num();)
    {
        uint32 C=B[I++];
        if (C < 0x80) { if (C==0) return false; continue; }
        if (Ascii) return false;
        int32 Extra; uint32 Minimum;
        if (C>=0xc2 && C<=0xdf) { Extra=1; Minimum=0x80; C &= 0x1f; }
        else if (C>=0xe0 && C<=0xef) { Extra=2; Minimum=0x800; C &= 0xf; }
        else if (C>=0xf0 && C<=0xf4) { Extra=3; Minimum=0x10000; C &= 7; }
        else return false;
        if (Extra > B.Num()-I) return false;
        while (Extra--) { const uint8 V=B[I++]; if ((V&0xc0)!=0x80) return false; C=(C<<6)|(V&0x3f); }
        if (C<Minimum || C>0x10ffff || (C>=0xd800 && C<=0xdfff) || C==0xfeff) return false;
    }
    return true;
}

struct FWire
{
    bool bWrite = true;
    bool bScan = false;
    TConstArrayView<uint8> Input;
    TArray<uint8> Output;
    int32 Position = 0;
    uint32 Elements = 0;
    int32 Depth = 0;
    int32 ByteLimit = LHSave::MaxPayloadBytes;
    uint32 StringLimit = 4096;
    uint32 GrowthLimit = 4096;
    FLHSaveError& Error;
    explicit FWire(FLHSaveError& E) : Error(E) {}
    FWire(TConstArrayView<uint8> B, FLHSaveError& E, bool Scan=false) : bWrite(false), bScan(Scan), Input(B), Error(E) {}
    bool Ok() const { return Error.Reason == ELHSaveReason::None; }
    int32 Remaining() const { return Input.Num()-Position; }
    void Fail(ELHSaveReason Reason, const TCHAR* Detail)
    { if (Ok()) { Error.Reason=Reason; Error.Detail=Detail; } }
    void Raw(uint8* Data, int32 N)
    {
        if (!Ok()) return;
        if (N<0 || (bWrite ? N>ByteLimit-Output.Num() : N>Remaining()))
        { Fail(bWrite ? ELHSaveReason::Oversize : ELHSaveReason::Malformed, TEXT("Truncated or excessive byte length")); return; }
        if (bWrite) Output.Append(Data,N);
        else { if (N) FMemory::Memcpy(Data,Input.GetData()+Position,N); Position+=N; }
    }
    void U32(uint32& V)
    {
        uint8 B[4]; if (bWrite) for (int I=0; I<4; ++I) B[I]=static_cast<uint8>(V>>(8*I));
        Raw(B,4); if (!bWrite && Ok()) { V=0; for (int I=0; I<4; ++I) V|=uint32(B[I])<<(8*I); }
    }
    void U64(uint64& V)
    {
        uint8 B[8]; if (bWrite) for (int I=0; I<8; ++I) B[I]=static_cast<uint8>(V>>(8*I));
        Raw(B,8); if (!bWrite && Ok()) { V=0; for (int I=0; I<8; ++I) V|=uint64(B[I])<<(8*I); }
    }
    void Struct(uint32 Expected)
    { uint32 N=Expected; U32(N); if (N!=Expected) Fail(ELHSaveReason::Malformed,TEXT("Wrong field count")); }
    void Name(const ANSICHAR* Expected)
    {
        const uint32 Size=FCStringAnsi::Strlen(Expected); uint32 N=Size; U32(N);
        if (!Ok()) return;
        if (bWrite) Raw(reinterpret_cast<uint8*>(const_cast<ANSICHAR*>(Expected)),Size);
        else if (N!=Size || N>static_cast<uint32>(Remaining()) || FMemory::Memcmp(Input.GetData()+Position,Expected,Size)!=0)
            Fail(ELHSaveReason::Malformed,TEXT("Missing, duplicate, unknown or unsorted field"));
        else Position+=Size;
    }
    void String(FString& V, uint32 Limit=4096, bool Ascii=false, bool Materialize=false)
    {
        if (!Ok()) return;
        if (bWrite)
        {
            // Conversion must not silently replace malformed native Unicode with U+FFFD.
            for (int32 I=0; I<V.Len(); ++I)
            {
                const uint32 C=static_cast<uint32>(V[I]);
                // Even explicit-length conversion can stop at NUL on some platforms.
                // Check the native storage before conversion so suffixes cannot disappear.
                if (C==0)
                { Fail(ELHSaveReason::Malformed,TEXT("Embedded NUL in string")); return; }
                if constexpr (sizeof(TCHAR)==2)
                {
                    if (C>=0xd800 && C<=0xdbff)
                    {
                        if (++I>=V.Len() || static_cast<uint32>(V[I])<0xdc00 || static_cast<uint32>(V[I])>0xdfff)
                        { Fail(ELHSaveReason::Malformed,TEXT("Malformed native Unicode")); return; }
                    }
                    else if (C>=0xdc00 && C<=0xdfff)
                    { Fail(ELHSaveReason::Malformed,TEXT("Malformed native Unicode")); return; }
                }
                else if (C>0x10ffff || (C>=0xd800 && C<=0xdfff))
                { Fail(ELHSaveReason::Malformed,TEXT("Malformed native Unicode")); return; }
            }
            FTCHARToUTF8 U(*V,V.Len()); uint32 N=U.Length();
            if (N>Limit || !Utf8(MakeArrayView(reinterpret_cast<const uint8*>(U.Get()),U.Length()),Ascii))
            { Fail(ELHSaveReason::Oversize,TEXT("String exceeds limit or is invalid UTF-8/token")); return; }
            U32(N); Raw(const_cast<uint8*>(reinterpret_cast<const uint8*>(U.Get())),N);
        }
        else
        {
            uint32 N=0; U32(N); if (!Ok()) return;
            if (N>Limit) { Fail(ELHSaveReason::Oversize,TEXT("String byte limit")); return; }
            if (N>static_cast<uint32>(Remaining())) { Fail(ELHSaveReason::Malformed,TEXT("Truncated string")); return; }
            auto B=Input.Slice(Position,N);
            if (!Utf8(B,Ascii)) { Fail(ELHSaveReason::Malformed,TEXT("Invalid UTF-8/token")); return; }
            if (!bScan || Materialize)
            {
                FUTF8ToTCHAR U(reinterpret_cast<const UTF8CHAR*>(B.GetData()),N);
                V=FString(U.Length(),U.Get());
            }
            Position+=N;
        }
    }
};
struct FDepth
{
    FWire& W;
    explicit FDepth(FWire& In) : W(In) { if (++W.Depth>16) W.Fail(ELHSaveReason::Oversize,TEXT("Nesting depth limit")); }
    ~FDepth() { --W.Depth; }
};
static void Visit(FWire& W, uint8& V) { W.Raw(&V,1); }
static void Visit(FWire& W, bool& V)
{ uint8 B=V ? 1 : 0; W.Raw(&B,1); if (B>1) W.Fail(ELHSaveReason::Malformed,TEXT("Invalid bool")); if (!W.bWrite) V=B==1; }
static void Visit(FWire& W, int32& V)
{ uint32 U=0; if (W.bWrite) FMemory::Memcpy(&U,&V,4); W.U32(U); if (!W.bWrite) FMemory::Memcpy(&V,&U,4); }
static void Visit(FWire& W, int64& V)
{ uint64 U=0; if (W.bWrite) FMemory::Memcpy(&U,&V,8); W.U64(U); if (!W.bWrite) FMemory::Memcpy(&V,&U,8); }
static void Visit(FWire& W, double& V)
{
    uint64 U=0; if (W.bWrite) { if (V==0) V=0; FMemory::Memcpy(&U,&V,8); }
    W.U64(U); if (!W.bWrite) FMemory::Memcpy(&V,&U,8);
    if (!FMath::IsFinite(V)) W.Fail(ELHSaveReason::Malformed,TEXT("Nonfinite number"));
}
static void Visit(FWire& W, FString& V) { W.String(V,W.StringLimit); }
static void Visit(FWire& W, FName& V)
{
    FString S=W.bWrite && !V.IsNone() ? V.ToString() : FString();
    W.String(S,128,true);
    if (!W.bWrite && !W.bScan && W.Ok()) V=S.IsEmpty() ? NAME_None : FName(*S);
}
static void Visit(FWire& W, FGuid& V) { W.U32(V.A); W.U32(V.B); W.U32(V.C); W.U32(V.D); }
static void Visit(FWire& W, FTransform& V)
{
    FDepth Depth(W); W.Struct(3);
    FQuat Q=V.GetRotation(); FVector S=V.GetScale3D(), T=V.GetTranslation();
    if (W.bWrite)
    {
        const double First=Q.W!=0 ? Q.W : Q.X!=0 ? Q.X : Q.Y!=0 ? Q.Y : Q.Z;
        if (First<0) Q=FQuat(-Q.X,-Q.Y,-Q.Z,-Q.W);
    }
    auto Coordinates=[&W](const ANSICHAR* Name, double* P, int32 Count)
    { W.Name(Name); uint32 N=Count; W.U32(N); if (N!=static_cast<uint32>(Count)) W.Fail(ELHSaveReason::Malformed,TEXT("Transform coordinate count")); for (int32 I=0; I<Count; ++I) Visit(W,P[I]); };
    double R[4]={Q.X,Q.Y,Q.Z,Q.W}, Scale[3]={S.X,S.Y,S.Z}, Translation[3]={T.X,T.Y,T.Z};
    Coordinates("RotationXYZW",R,4); Coordinates("ScaleXYZ",Scale,3); Coordinates("TranslationXYZ",Translation,3);
    Q=FQuat(R[0],R[1],R[2],R[3]); S=FVector(Scale[0],Scale[1],Scale[2]); T=FVector(Translation[0],Translation[1],Translation[2]);
    if (FMath::Abs(Q.SizeSquared()-1)>1e-6 || S.X<=0 || S.Y<=0 || S.Z<=0)
        W.Fail(ELHSaveReason::InvalidSnapshot,TEXT("Invalid transform quaternion or scale"));
    if (!W.bWrite && W.Ok()) V=FTransform(Q,T,S);
}

#include "LHSaveWireV1.inl"

void PayloadV1(FWire& W, FLHSaveSnapshot& V)
{
    FDepth Depth(W); W.Struct(3);
    Field(W,"Character",V.Character); Field(W,"Session",V.Session); Field(W,"World",V.World);
}
#include "LHSaveWireV2.inl"

bool Hex(const FString& S)
{
    if (S.Len()!=64) return false;
    for (TCHAR C:S) if (!((C>='0' && C<='9') || (C>='a' && C<='f'))) return false;
    return true;
}
bool Reject(FLHSaveError& E, ELHSaveReason R, const TCHAR* D) { E={R,D}; return false; }
}

namespace LHSaveCodecPrivate
{
bool DecodeCanonicalPayload(TConstArrayView<uint8> P, const FLHSaveCompatibility& Compatibility,
    FLHSaveSnapshot& V, FLHSaveError& Error, FLHSaveDecodeStats* Stats)
{
    if (Compatibility.MaxGrowthAwards<0 || Compatibility.MaxGrowthAwards>4096) return Reject(Error,ELHSaveReason::IncompatibleRuleset,TEXT("Invalid ruleset growth capacity"));
    FWire Scan(P,Error,true); Scan.GrowthLimit=Compatibility.MaxGrowthAwards; FLHSaveSnapshot Scratch; PayloadV1(Scan,Scratch);
    if (!Scan.Ok()) return false;
    if (Scan.Remaining()!=0) return Reject(Error,ELHSaveReason::Malformed,TEXT("Trailing payload bytes"));
    if (Stats) Stats->bPayloadAllocationStarted=true;
    FWire Read(P,Error); Read.GrowthLimit=Compatibility.MaxGrowthAwards; PayloadV1(Read,V);
    if (!Read.Ok()) return false;
    FWire Canonical(Error); PayloadV1(Canonical,V); if (!Canonical.Ok()) return false;
    if (Canonical.Output.Num()!=P.Num() || FMemory::Memcmp(Canonical.Output.GetData(),P.GetData(),P.Num())!=0)
        return Reject(Error,ELHSaveReason::Malformed,TEXT("Payload is not canonical"));
    return true;
}
bool DecodeV1AndMigrate(TConstArrayView<uint8> P, const FLHSaveCompatibility& C,
    FLHSaveSnapshot& V, FLHSaveError& E, FLHSaveDecodeStats* Stats)
{
    if (!DecodeCanonicalPayload(P,C,V,E,Stats)) return false;
    // Layout and durable facts are identical. Only the contract version changes.
    V.Header.SchemaVersion=LHSave::CurrentSchemaVersion;
    return LHSave::Validate(V,E);
}
bool DecodeIdentityV2(TConstArrayView<uint8> P, const FLHSaveCompatibility& C,
    FLHSaveSnapshot& V, FLHSaveError& E, FLHSaveDecodeStats* Stats)
{
    return DecodeCanonicalPayload(P,C,V,E,Stats) && LHSave::Validate(V,E);
}
struct FSchemaDispatch
{
    int32 Version;
    bool (*DecodeAndMigrate)(TConstArrayView<uint8>, const FLHSaveCompatibility&, FLHSaveSnapshot&, FLHSaveError&, FLHSaveDecodeStats*);
};
const FSchemaDispatch* FindSchema(int32 Version)
{
    // Each future entry needs a bounded version-specific decoder and an explicit, unambiguous migration.
    static const FSchemaDispatch Dispatch[] = {{1,&DecodeV1AndMigrate},{2,&DecodeIdentityV2}};
    for (const auto& Entry : Dispatch) if (Entry.Version==Version) return &Entry;
    return nullptr;
}
}
bool LHSave::SupportsVersion(int32 Version) { return LHSaveCodecPrivate::FindSchema(Version)!=nullptr; }

bool LHSave::Encode(const FLHSaveSnapshot& Snapshot, TArray<uint8>& Bytes, FLHSaveError& Error)
{
    using namespace LHSaveCodecPrivate;
    Error={}; Bytes.Reset();
    if (!Validate(Snapshot,Error)) return false;
    FLHSaveSnapshot V=Snapshot;
    FWire P(Error); PayloadV2(P,V); if (!P.Ok()) return false;
    V.Header.PayloadLengthBytes=P.Output.Num(); V.Header.PayloadChecksum=Sha256(P.Output);
    FWire H(Error); H.ByteLimit=MaxHeaderBytes; Visit(H,V.Header); if (!H.Ok()) return false;
    FWire File(Error); File.ByteLimit=MaxFileBytes;
    uint8 Magic[6]={'L','H','S','a','v','e'}; File.Raw(Magic,6);
    uint32 N=H.Output.Num(); File.U32(N); File.Raw(H.Output.GetData(),H.Output.Num()); File.Raw(P.Output.GetData(),P.Output.Num());
    if (!File.Ok()) return false;
    Bytes=MoveTemp(File.Output); return true;
}

bool LHSave::Decode(TConstArrayView<uint8> Bytes, const FLHCharacterId& Character,
    const FLHSaveCompatibility& Compatibility, FLHSaveSnapshot& Snapshot, FLHSaveError& Error, FLHSaveDecodeStats* Stats)
{
    using namespace LHSaveCodecPrivate;
    Error={}; if (Stats) *Stats={};
    if (Bytes.Num()>MaxFileBytes) return Reject(Error,ELHSaveReason::Oversize,TEXT("File limit"));
    FWire File(Bytes,Error); uint8 Magic[6]={}; File.Raw(Magic,6);
    if (!File.Ok() || FMemory::Memcmp(Magic,"LHSave",6)!=0) return Reject(Error,ELHSaveReason::Malformed,TEXT("Preamble magic"));
    uint32 N=0; File.U32(N);
    if (!File.Ok()) return false;
    if (N>MaxHeaderBytes) return Reject(Error,ELHSaveReason::Oversize,TEXT("Header limit"));
    if (N>static_cast<uint32>(File.Remaining())) return Reject(Error,ELHSaveReason::Malformed,TEXT("Truncated envelope"));
    FLHSaveSnapshot V; FWire H(Bytes.Slice(File.Position,N),Error); Visit(H,V.Header);
    if (!H.Ok()) return false;
    if (H.Remaining()!=0) return Reject(Error,ELHSaveReason::Malformed,TEXT("Trailing envelope bytes"));
    File.Position+=N;
    if (V.Header.SchemaVersion>CurrentSchemaVersion) return Reject(Error,ELHSaveReason::FutureSchema,TEXT("Future schema; compatible game required"));
    if (!SupportsVersion(V.Header.SchemaVersion)) return Reject(Error,ELHSaveReason::UnsupportedSchema,TEXT("No unambiguous migration"));
    if (V.Header.ChecksumAlgorithm!=TEXT("SHA256") || V.Header.Ruleset.HashAlgorithm!=TEXT("SHA256"))
        return Reject(Error,ELHSaveReason::UnknownAlgorithm,TEXT("Unsupported checksum/rules hash algorithm"));
    if (V.Header.PayloadCodec!=TEXT("LHCanonicalBinary1")) return Reject(Error,ELHSaveReason::UnknownCodec,TEXT("Unsupported payload codec"));
    if (V.Header.PayloadLengthBytes<0 || V.Header.PayloadLengthBytes>MaxPayloadBytes)
        return Reject(Error,ELHSaveReason::Oversize,TEXT("Payload limit"));
    if (V.Header.PayloadLengthBytes!=File.Remaining()) return Reject(Error,ELHSaveReason::Malformed,TEXT("Truncated payload or trailing bytes"));
    if (V.Header.CharacterId.Value!=Character.Value) return Reject(Error,ELHSaveReason::InvalidSnapshot,TEXT("Character slot mismatch"));
    if (V.Header.Ruleset.Id.Value.ToString()!=Compatibility.Ruleset.Id.Value.ToString() ||
        V.Header.Ruleset.Revision!=Compatibility.Ruleset.Revision || V.Header.Ruleset.ContentHash!=Compatibility.Ruleset.ContentHash ||
        Compatibility.Ruleset.HashAlgorithm!=TEXT("SHA256"))
        return Reject(Error,ELHSaveReason::IncompatibleRuleset,TEXT("Rules require explicit migration/new character policy"));
    if (V.Header.ContentRevision!=Compatibility.ContentRevision) return Reject(Error,ELHSaveReason::IncompatibleContent,TEXT("Compatible gameplay catalog required"));
    auto P=Bytes.Slice(File.Position,File.Remaining());
    if (!Hex(V.Header.PayloadChecksum) || Sha256(P)!=V.Header.PayloadChecksum) return Reject(Error,ELHSaveReason::BadChecksum,TEXT("Payload SHA256 mismatch"));
    // Preserve original header for canonical comparison after migration.
    auto OriginalHeader=V.Header;
    const FSchemaDispatch* Dispatch=FindSchema(V.Header.SchemaVersion);
    if (!Dispatch || !Dispatch->DecodeAndMigrate(P,Compatibility,V,Error,Stats)) return false;
    FWire CanonicalHeader(Error); CanonicalHeader.ByteLimit=MaxHeaderBytes; Visit(CanonicalHeader,OriginalHeader);
    if (!CanonicalHeader.Ok() || CanonicalHeader.Output.Num()!=static_cast<int32>(N) ||
        FMemory::Memcmp(CanonicalHeader.Output.GetData(),Bytes.GetData()+10,N)!=0)
        return Reject(Error,ELHSaveReason::Malformed,TEXT("Envelope is not canonical"));
    if (Compatibility.ValidateReferences && !Compatibility.ValidateReferences(V,Error))
    {
        if (Error.Reason==ELHSaveReason::None) return Reject(Error,ELHSaveReason::InvalidSnapshot,TEXT("Authoritative registry rejected snapshot references"));
        return false;
    }
    Snapshot=MoveTemp(V); return true;
}

namespace LHSaveValidationPrivate
{
bool Id(FName N, bool Dotted);
bool Area(const FLHAreaId& A);
bool Attributes(const FLHAttributeBlock& A);
bool Rng(const FLHRngState& R, bool Gameplay);
}
namespace LHSaveCodecPrivate
{
static bool Valid(const FLHCreateCharacterRequest& R)
{
    using namespace LHSaveValidationPrivate;
    if (R.DisplayName.IsEmpty() || R.DisplayName.Len()>128 || R.AppearanceIds.Num()>16 ||
        R.Creation.QuestionAnswers.Num()>4 || R.Creation.AcceptedRollInputs.Num()>16 || !R.PreviewToken.IsValid() || !Attributes(R.Creation.AcceptedAttributes) ||
        R.Creation.GenerationRevision.Resolution!=ELHValueResolution::Resolved || R.Creation.GenerationRevision.Value<1 ||
        !Id(R.Creation.GenerationPolicy.Value,true)) return false;
    for (const auto& A:R.AppearanceIds) if (!Id(A.Value,true)) return false;
    for (const auto& Q:R.Creation.QuestionAnswers) if (!Id(Q.Question.Value,true) || !Id(Q.Answer.Value,true)) return false;
    for (const auto& I:R.Creation.AcceptedRollInputs) if (!Rng(I,false)) return false;
    return true;
}
static bool Valid(const FLHAllocateAttributePointsRequest& R)
{
    if (!LHSaveValidationPrivate::Attributes(R.Points)) return false;
    int64 Total=0;
    for (const auto* P:{&R.Points.Agility,&R.Points.Endurance,&R.Points.Intelligence,&R.Points.Strength,&R.Points.Wisdom})
    { if (P->Value>MAX_int64-Total) return false; Total+=P->Value; }
    return Total>0;
}
static bool Valid(const FLHEquipItemRequest& R)
{
    return R.Item.RunId.IsValid() && R.Item.InstanceId.IsValid() && LHSaveValidationPrivate::Area(R.Item.Area) &&
        R.Slot!=ELHEquipmentSlot::Unspecified;
}
static bool Valid(const FLHUseItemRequest& R)
{
    auto Entity=[](const FLHEntityId& E) { return E.RunId.IsValid() && E.InstanceId.IsValid() && LHSaveValidationPrivate::Area(E.Area); };
    const bool Self=!R.Target.RunId.IsValid() && R.Target.Area.Content.Value.IsNone() && !R.Target.InstanceId.IsValid();
    return Entity(R.Item) && (Self || Entity(R.Target));
}
static void RequestFields(FWire& W, FLHCreateCharacterRequest& R)
{
    FDepth Depth(W); W.Struct(5);
    W.Name("AppearanceIds"); Array(W,R.AppearanceIds,16,false);
    Field(W,"Creation",R.Creation);
    W.Name("DisplayName"); W.String(R.DisplayName,128);
    Field(W,"PreviewToken",R.PreviewToken); Field(W,"Request",R.Request);
}
static void RequestFields(FWire& W, FLHAllocateAttributePointsRequest& R)
{
    FDepth Depth(W); W.Struct(2); Field(W,"Points",R.Points); Field(W,"Request",R.Request);
}
static void RequestFields(FWire& W, FLHEquipItemRequest& R)
{
    FDepth Depth(W); W.Struct(4); Field(W,"Item",R.Item); Field(W,"Request",R.Request);
    Field(W,"Slot",R.Slot); Field(W,"bUnequip",R.bUnequip);
}
static bool EntityRequest(const FLHEntityId& E)
{ return E.RunId.IsValid() && E.InstanceId.IsValid() && LHSaveValidationPrivate::Area(E.Area); }
static bool PositiveRequest(const FLHInteger& I)
{ return I.Resolution==ELHValueResolution::Resolved && I.Value>0; }
static bool Valid(const FLHTrainSkillRequest& R) { return EntityRequest(R.Trainer) && LHSaveValidationPrivate::Id(R.Skill.Value,true) && PositiveRequest(R.Points); }
static bool Valid(const FLHLearnSpellRequest& R) { return EntityRequest(R.Trainer) && LHSaveValidationPrivate::Id(R.Spell.Value,true); }
static bool Valid(const FLHBuyItemRequest& R) { return EntityRequest(R.Vendor) && LHSaveValidationPrivate::Id(R.Offer.Value,true) && PositiveRequest(R.Quantity); }
static bool Valid(const FLHSellItemRequest& R) { return EntityRequest(R.Vendor) && EntityRequest(R.Item) && PositiveRequest(R.Quantity); }
static bool Valid(const FLHUseAbilityRequest& R) { return LHSaveValidationPrivate::Id(R.Ability.Value,true) && EntityRequest(R.Target); }
static bool Valid(const FLHInteractRequest& R) { return EntityRequest(R.Target) && LHSaveValidationPrivate::Id(R.Topic.Value,true); }
static bool Valid(const FLHTakeLootRequest& R) { return EntityRequest(R.Container) && LHValidateLootTransferPayload(R)==ELHCommandReason::None; }
static void RequestFields(FWire& W, FLHTrainSkillRequest& R)
{ FDepth D(W); W.Struct(4); Field(W,"Points",R.Points); Field(W,"Request",R.Request); Field(W,"Skill",R.Skill); Field(W,"Trainer",R.Trainer); }
static void RequestFields(FWire& W, FLHLearnSpellRequest& R)
{ FDepth D(W); W.Struct(3); Field(W,"Request",R.Request); Field(W,"Spell",R.Spell); Field(W,"Trainer",R.Trainer); }
static void RequestFields(FWire& W, FLHBuyItemRequest& R)
{ FDepth D(W); W.Struct(4); Field(W,"Offer",R.Offer); Field(W,"Quantity",R.Quantity); Field(W,"Request",R.Request); Field(W,"Vendor",R.Vendor); }
static void RequestFields(FWire& W, FLHSellItemRequest& R)
{ FDepth D(W); W.Struct(4); Field(W,"Item",R.Item); Field(W,"Quantity",R.Quantity); Field(W,"Request",R.Request); Field(W,"Vendor",R.Vendor); }
static void RequestFields(FWire& W, FLHUseAbilityRequest& R)
{ FDepth D(W); W.Struct(3); Field(W,"Ability",R.Ability); Field(W,"Request",R.Request); Field(W,"Target",R.Target); }
static void RequestFields(FWire& W, FLHInteractRequest& R)
{ FDepth D(W); W.Struct(3); Field(W,"Request",R.Request); Field(W,"Target",R.Target); Field(W,"Topic",R.Topic); }
static void RequestFields(FWire& W, FLHTakeLootRequest& R)
{ FDepth D(W); W.Struct(5); Field(W,"Container",R.Container); Field(W,"Item",R.Item); W.Name("Kind"); FName Kind=R.Kind==ELHLootTransferKind::Item?FName(TEXT("Item")):R.Kind==ELHLootTransferKind::Gold?FName(TEXT("Gold")):NAME_None; Visit(W,Kind); if (!W.bWrite) R.Kind=Kind.ToString()==TEXT("Item")?ELHLootTransferKind::Item:Kind.ToString()==TEXT("Gold")?ELHLootTransferKind::Gold:ELHLootTransferKind::Unspecified; Field(W,"Quantity",R.Quantity); Field(W,"Request",R.Request); }
template<class T> static bool RequestWire(FName Command, const void* Input, TConstArrayView<uint8> Bytes,
    TArray<uint8>& Out, FLHSaveError& Error, bool Read)
{
    T R;
    if (!Read)
    {
        const T& Source=*static_cast<const T*>(Input);
        if (!Valid(Source)) return Reject(Error,ELHSaveReason::Malformed,TEXT("Invalid request fields"));
        R=Source;
    }
    auto Envelope=[Command](FWire& W,T& V)
    {
        FDepth Depth(W); W.Struct(3);
        W.Name("Command"); W.Name(TCHAR_TO_ANSI(*Command.ToString()));
        W.Name("Domain"); W.Name("LHRequest1"); W.Name("Request"); RequestFields(W,V);
    };
    if (Read)
    {
        if (Bytes.Num()>LHSave::MaxPayloadBytes) return Reject(Error,ELHSaveReason::Oversize,TEXT("Request byte limit"));
        T Scratch; FWire Scan(Bytes,Error,true); Envelope(Scan,Scratch);
        if (!Scan.Ok() || Scan.Remaining()!=0) return Reject(Error,ELHSaveReason::Malformed,TEXT("Malformed request bytes"));
        FWire W(Bytes,Error); Envelope(W,R); if (!W.Ok()) return false;
    }
    if (!R.Request.Value.IsValid() || !R.Request.Epoch.IsValid() || !Valid(R))
        return Reject(Error,ELHSaveReason::Malformed,TEXT("Invalid request fields"));
    FWire W(Error); Envelope(W,R); if (!W.Ok()) return false;
    if (Read && (W.Output.Num()!=Bytes.Num() || FMemory::Memcmp(W.Output.GetData(),Bytes.GetData(),Bytes.Num())!=0))
        return Reject(Error,ELHSaveReason::Malformed,TEXT("Noncanonical request bytes"));
    Out=MoveTemp(W.Output); return true;
}
static bool DispatchRequest(FName Command,const UScriptStruct* Type,const void* Input,TConstArrayView<uint8> Bytes,
    TArray<uint8>& Out,FLHSaveError& Error,bool Read)
{
    // ToString comparison preserves case-sensitive command tokens; FName equality would not.
#define LH_REQUEST(T, Token) if (Command.ToString()==TEXT(Token) && (Read || Type==T::StaticStruct())) \
    return RequestWire<T>(Command,Input,Bytes,Out,Error,Read);
    LH_REQUEST(FLHCreateCharacterRequest,"CreateCharacter")
    LH_REQUEST(FLHAllocateAttributePointsRequest,"AllocateAttributePoints")
    LH_REQUEST(FLHEquipItemRequest,"EquipItem")
    LH_REQUEST(FLHUseItemRequest,"UseItem")
    LH_REQUEST(FLHTrainSkillRequest,"TrainSkill")
    LH_REQUEST(FLHLearnSpellRequest,"LearnSpell")
    LH_REQUEST(FLHBuyItemRequest,"BuyItem")
    LH_REQUEST(FLHSellItemRequest,"SellItem")
    LH_REQUEST(FLHUseAbilityRequest,"UseAbility")
    LH_REQUEST(FLHInteractRequest,"Interact")
    LH_REQUEST(FLHTakeLootRequest,"TakeLoot")
#undef LH_REQUEST
    return Reject(Error,ELHSaveReason::Malformed,TEXT("Unsupported command or mismatched request type"));
}
}
bool LHSave::EncodeCanonicalRequest(FName Command,const UScriptStruct* Type,const void* Request,TArray<uint8>& Bytes,FLHSaveError& Error)
{
    Error={}; Bytes.Reset();
    if (!Request) return LHSaveCodecPrivate::Reject(Error,ELHSaveReason::Malformed,TEXT("Null request"));
    return LHSaveCodecPrivate::DispatchRequest(Command,Type,Request,{},Bytes,Error,false);
}
bool LHSave::CanonicalRequestDigest(FName Command,const UScriptStruct* Type,const void* Request,FString& Digest,FLHSaveError& Error)
{
    Digest.Reset(); TArray<uint8> Bytes;
    if (!EncodeCanonicalRequest(Command,Type,Request,Bytes,Error)) return false;
    Digest=Sha256(Bytes); return true;
}
FString LHSave::RequestDigest(FName Command,const UScriptStruct* Type,const void* Request)
{ FString Digest; FLHSaveError Error; CanonicalRequestDigest(Command,Type,Request,Digest,Error); return Digest; }
bool LHSave::ValidateCanonicalRequestBytes(FName Command,TConstArrayView<uint8> Bytes,FLHSaveError& Error)
{ Error={}; TArray<uint8> Out; return LHSaveCodecPrivate::DispatchRequest(Command,nullptr,nullptr,Bytes,Out,Error,true); }
bool LHSave::RewardIdFromDigest(const FString& Digest,FLHRewardId& Reward,FLHSaveError& Error)
{
    Error={}; Reward={};
    if (!LHSaveCodecPrivate::Hex(Digest)) return LHSaveCodecPrivate::Reject(Error,ELHSaveReason::Malformed,TEXT("Invalid reward SHA256"));
    uint32 Words[4]={};
    auto Nibble=[](TCHAR C)->uint32 { return C<='9' ? C-'0' : C-'a'+10; };
    for (int32 I=0;I<16;++I) Words[I/4]|=((Nibble(Digest[2*I])<<4)|Nibble(Digest[2*I+1]))<<(8*(I%4));
    FGuid Guid(Words[0],Words[1],Words[2],Words[3]);
    if (!Guid.IsValid()) return LHSaveCodecPrivate::Reject(Error,ELHSaveReason::Malformed,TEXT("All-zero reward identity"));
    Reward.Value=Guid; return true;
}
bool LHSave::GrowthRewardId(const FGuid& Run,const FLHCharacterId& Character,int64 ToLevel,FLHRewardId& Reward,FLHSaveError& Error)
{
    using namespace LHSaveCodecPrivate;
    Error={}; Reward={};
    if (!Run.IsValid() || !Character.Value.IsValid() || ToLevel<2) return Reject(Error,ELHSaveReason::Malformed,TEXT("Invalid growth source"));
    FWire W(Error); W.Struct(5); W.Name("Domain"); W.Name("LHReward1"); W.Name("Purpose"); W.Name("LevelGrowth");
    FGuid R=Run; Field(W,"RunId",R); W.Name("SourceKey"); W.Struct(2);
    FLHCharacterId C=Character; Field(W,"CharacterId",C); Field(W,"ToLevel",ToLevel);
    W.Name("SourceKind"); W.Name("Growth");
    return W.Ok() && RewardIdFromDigest(Sha256(W.Output),Reward,Error);
}
bool LHSave::EnemyLifeRewardId(const FGuid& Run,const FLHSpawnLifeId& Life,FLHRewardId& Reward,FLHSaveError& Error)
{
    using namespace LHSaveCodecPrivate;
    Error={}; Reward={};
    if (!Run.IsValid() || !Life.SpawnSlot.IsValid() || Life.LifeGeneration<0 || !LHSaveValidationPrivate::Area(Life.Area))
        return Reject(Error,ELHSaveReason::Malformed,TEXT("Invalid enemy life source"));
    FWire W(Error); W.Struct(5); W.Name("Domain"); W.Name("LHReward1"); W.Name("Purpose"); W.Name("KillSettlement");
    FGuid R=Run; Field(W,"RunId",R); FLHSpawnLifeId L=Life; Field(W,"SourceKey",L);
    W.Name("SourceKind"); W.Name("EnemyLife");
    return W.Ok() && RewardIdFromDigest(Sha256(W.Output),Reward,Error);
}
FLHRewardId LHSave::GrowthId(const FGuid& Run,const FLHCharacterId& Character,int64 ToLevel)
{ FLHRewardId Reward; FLHSaveError Error; GrowthRewardId(Run,Character,ToLevel,Reward,Error); return Reward; }

TArray<uint8> LHSave::CanonicalValue(FString Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(FName Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(int32 Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(int64 Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(bool Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(FLHInteger Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(FLHNumber Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(FLHContentId Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(FLHAttributeBlock Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(FLHQuestionAnswer Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(FLHFieldProvenance Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(FLHMechanicalField Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(FTransform Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}
TArray<uint8> LHSave::CanonicalValue(ELHEquipmentSlot Value, FLHSaveError& Error)
{
    Error={}; LHSaveCodecPrivate::FWire Wire(Error); LHSaveCodecPrivate::Visit(Wire,Value);
    return Wire.Ok()?MoveTemp(Wire.Output):TArray<uint8>();
}

bool LHSave::QuestTurnInRewardId(const FGuid& Run,const FLHContentId& Quest,FName Stage,FLHRewardId& Reward,FLHSaveError& Error)
{
    using namespace LHSaveCodecPrivate; Error={}; Reward={};
    if (!Run.IsValid() || !LHSaveValidationPrivate::Id(Quest.Value,true) || !LHSaveValidationPrivate::Id(Stage,false)) return Reject(Error,ELHSaveReason::Malformed,TEXT("Invalid quest source"));
    FWire W(Error); W.Struct(5); W.Name("Domain"); W.Name("LHReward1"); W.Name("Purpose"); W.Name("QuestTurnIn");
    FGuid R=Run; Field(W,"RunId",R); W.Name("SourceKey"); W.Struct(2); auto Q=Quest; Field(W,"QuestId",Q); Field(W,"Stage",Stage); W.Name("SourceKind"); W.Name("Quest");
    return W.Ok() && RewardIdFromDigest(Sha256(W.Output),Reward,Error);
}
bool LHSave::BossUniqueRewardId(const FGuid& Run,const FLHContentId& Boss,FLHRewardId& Reward,FLHSaveError& Error)
{
    using namespace LHSaveCodecPrivate; Error={}; Reward={};
    if (!Run.IsValid() || !LHSaveValidationPrivate::Id(Boss.Value,true)) return Reject(Error,ELHSaveReason::Malformed,TEXT("Invalid boss source"));
    FWire W(Error); W.Struct(5); W.Name("Domain"); W.Name("LHReward1"); W.Name("Purpose"); W.Name("BossUnique");
    FGuid R=Run; Field(W,"RunId",R); W.Name("SourceKey"); W.Struct(2); auto B=Boss; Field(W,"BossId",B); FName Objective=TEXT("PermanentObjective"); Field(W,"Objective",Objective); W.Name("SourceKind"); W.Name("Boss");
    return W.Ok() && RewardIdFromDigest(Sha256(W.Output),Reward,Error);
}
