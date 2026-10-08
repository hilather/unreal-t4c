#include "LHSaveCodec.h"
#include "Containers/StringConv.h"

namespace
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
            FTCHARToUTF8 U(*V); uint32 N=U.Length();
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

void Payload(FWire& W, FLHSaveSnapshot& V)
{
    FDepth Depth(W); W.Struct(3);
    Field(W,"Character",V.Character); Field(W,"Session",V.Session); Field(W,"World",V.World);
}
bool Hex(const FString& S)
{
    if (S.Len()!=64) return false;
    for (TCHAR C:S) if (!((C>='0' && C<='9') || (C>='a' && C<='f'))) return false;
    return true;
}
bool Reject(FLHSaveError& E, ELHSaveReason R, const TCHAR* D) { E={R,D}; return false; }
}

namespace
{
bool DecodeIdentityV1(TConstArrayView<uint8> P, const FLHSaveCompatibility& Compatibility,
    FLHSaveSnapshot& V, FLHSaveError& Error, FLHSaveDecodeStats* Stats)
{
    if (Compatibility.MaxGrowthAwards<0 || Compatibility.MaxGrowthAwards>4096) return Reject(Error,ELHSaveReason::IncompatibleRuleset,TEXT("Invalid ruleset growth capacity"));
    FWire Scan(P,Error,true); Scan.GrowthLimit=Compatibility.MaxGrowthAwards; FLHSaveSnapshot Scratch; Payload(Scan,Scratch);
    if (!Scan.Ok()) return false;
    if (Scan.Remaining()!=0) return Reject(Error,ELHSaveReason::Malformed,TEXT("Trailing payload bytes"));
    if (Stats) Stats->bPayloadAllocationStarted=true;
    FWire Read(P,Error); Read.GrowthLimit=Compatibility.MaxGrowthAwards; Payload(Read,V);
    if (!Read.Ok() || !LHSave::Validate(V,Error)) return false;
    FWire Canonical(Error); Payload(Canonical,V); if (!Canonical.Ok()) return false;
    if (Canonical.Output.Num()!=P.Num() || FMemory::Memcmp(Canonical.Output.GetData(),P.GetData(),P.Num())!=0)
        return Reject(Error,ELHSaveReason::Malformed,TEXT("Payload is not canonical v1"));
    return true;
}
struct FSchemaDispatch
{
    int32 Version;
    bool (*DecodeAndMigrate)(TConstArrayView<uint8>, const FLHSaveCompatibility&, FLHSaveSnapshot&, FLHSaveError&, FLHSaveDecodeStats*);
};
const FSchemaDispatch* FindSchema(int32 Version)
{
    // Each future entry needs a bounded version-specific decoder and an explicit, unambiguous migration.
    static const FSchemaDispatch Dispatch[] = {{1,&DecodeIdentityV1}};
    for (const auto& Entry : Dispatch) if (Entry.Version==Version) return &Entry;
    return nullptr;
}
}
bool LHSave::SupportsVersion(int32 Version) { return FindSchema(Version)!=nullptr; }

bool LHSave::Encode(const FLHSaveSnapshot& Snapshot, TArray<uint8>& Bytes, FLHSaveError& Error)
{
    Error={}; Bytes.Reset();
    if (!Validate(Snapshot,Error)) return false;
    FLHSaveSnapshot V=Snapshot;
    FWire P(Error); Payload(P,V); if (!P.Ok()) return false;
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
    if (V.Header.SchemaVersion>1) return Reject(Error,ELHSaveReason::FutureSchema,TEXT("Future schema; compatible game required"));
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
    // Version dispatch is explicit; v1 is identity, never a reinterpretation of another layout.
    const FSchemaDispatch* Dispatch=FindSchema(V.Header.SchemaVersion);
    if (!Dispatch || !Dispatch->DecodeAndMigrate(P,Compatibility,V,Error,Stats)) return false;
    FWire CanonicalHeader(Error); CanonicalHeader.ByteLimit=MaxHeaderBytes; Visit(CanonicalHeader,V.Header);
    if (!CanonicalHeader.Ok() || CanonicalHeader.Output.Num()!=static_cast<int32>(N) ||
        FMemory::Memcmp(CanonicalHeader.Output.GetData(),Bytes.GetData()+10,N)!=0)
        return Reject(Error,ELHSaveReason::Malformed,TEXT("Envelope is not canonical v1"));
    if (Compatibility.ValidateReferences && !Compatibility.ValidateReferences(V,Error))
    {
        if (Error.Reason==ELHSaveReason::None) return Reject(Error,ELHSaveReason::InvalidSnapshot,TEXT("Authoritative registry rejected snapshot references"));
        return false;
    }
    Snapshot=MoveTemp(V); return true;
}
