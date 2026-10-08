#include "LHSaveCodec.h"

// FIPS 180-4 SHA-256. Kept portable: GenericPlatformMisc has no Linux SHA256 implementation.
FString LHSave::Sha256(TConstArrayView<uint8> Bytes)
{
    static constexpr uint32 K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
    uint32 H[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    auto R = [](uint32 X, uint32 N) { return (X >> N) | (X << (32-N)); };
    const uint64 Size = static_cast<uint64>(Bytes.Num());
    const uint64 Blocks = (Size + 9 + 63) / 64;
    for (uint64 Block = 0; Block < Blocks; ++Block)
    {
        uint8 B[64] = {};
        for (uint64 I = 0; I < 64; ++I)
        {
            const uint64 P = Block*64 + I;
            if (P < Size) B[I] = Bytes[static_cast<int32>(P)];
            else if (P == Size) B[I] = 0x80;
            if (P >= Blocks*64-8) B[I] = static_cast<uint8>((Size*8) >> ((Blocks*64-1-P)*8));
        }
        uint32 W[64];
        for (int I=0; I<16; ++I) W[I] = (uint32(B[I*4])<<24)|(uint32(B[I*4+1])<<16)|(uint32(B[I*4+2])<<8)|B[I*4+3];
        for (int I=16; I<64; ++I)
        {
            const uint32 X=W[I-15], Y=W[I-2];
            W[I]=W[I-16]+(R(X,7)^R(X,18)^(X>>3))+W[I-7]+(R(Y,17)^R(Y,19)^(Y>>10));
        }
        uint32 A=H[0], C=H[2], D=H[3], E=H[4], F=H[5], G=H[6], HH=H[7], BB=H[1];
        for (int I=0; I<64; ++I)
        {
            const uint32 T1=HH+(R(E,6)^R(E,11)^R(E,25))+((E&F)^(~E&G))+K[I]+W[I];
            const uint32 T2=(R(A,2)^R(A,13)^R(A,22))+((A&BB)^(A&C)^(BB&C));
            HH=G; G=F; F=E; E=D+T1; D=C; C=BB; BB=A; A=T1+T2;
        }
        H[0]+=A; H[1]+=BB; H[2]+=C; H[3]+=D; H[4]+=E; H[5]+=F; H[6]+=G; H[7]+=HH;
    }
    FString Result;
    for (uint32 Word : H) Result += FString::Printf(TEXT("%08x"), Word);
    return Result;
}
