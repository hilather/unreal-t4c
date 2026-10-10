#pragma once
#include "CoreMinimal.h"
#include "UI/LHUIPresenter.h"

// User preferences live in LighthavenPresentation.ini, never in an FLHSaveSnapshot.
struct LIGHTHAVEN_API FLHPresentationSettings
{
    float Master=1, Effects=1, UI=1, TextScale=1;
    bool HighContrast=false;
    void Load(const FString& File);
    bool Save(const FString& File) const;
    void Normalize();
    float Gain(bool IsUI) const { return Master*(IsUI?UI:Effects); }
    static FLHPresentationSettings& Get();
    static FString UserFile();
};
// Cosmetic state accepts copies only; no command, RNG, GAS or award interface.
struct LIGHTHAVEN_API FLHPresentationCues
{
    bool Enabled=true, Initialized=false;
    float Health=0, Mana=0, Remaining=0;
    FString Notice;
    FLHUIHud Previous;
    int64 Level=0;
    int32 ResourceEvents=0, HitEvents=0;
    void Observe(const FLHUIHud& Hud, const FLHInteger& EarnedLevel, float Delta);
    void Hit(bool Landed, double Damage);
};

// Silent runtime sound seams; gain is applied before delivery to an optional sink.
struct LIGHTHAVEN_API FLHPresentationAudio
{
    static TFunction<void(FName,float)> Sink;
    static void Cue(FName Id,bool IsUI);
};
