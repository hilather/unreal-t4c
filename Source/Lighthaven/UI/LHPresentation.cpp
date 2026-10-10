#include "UI/LHPresentation.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
namespace LHPresentationPrivate
{
const TCHAR* Section=TEXT("Lighthaven.Presentation");
float Fraction(const FLHNumber& N,const FLHNumber& M)
{
    return N.Resolution==ELHValueResolution::Resolved && M.Resolution==ELHValueResolution::Resolved && FMath::IsFinite(N.Value) && FMath::IsFinite(M.Value) && M.Value>0 ? float(FMath::Clamp(N.Value/M.Value,0.,1.)) : 0;
}
float Valid(float V,float Min,float Max,float Default) { return FMath::IsFinite(V)?FMath::Clamp(V,Min,Max):Default; }
}
void FLHPresentationSettings::Normalize()
{
    using namespace LHPresentationPrivate;
    Master=Valid(Master,0,1,1); Effects=Valid(Effects,0,1,1); UI=Valid(UI,0,1,1); TextScale=Valid(TextScale,1,1.5,1);
}
void FLHPresentationSettings::Load(const FString& File)
{
    using namespace LHPresentationPrivate;
    *this={}; FConfigFile Config; Config.Read(File);
    Config.GetFloat(Section,TEXT("Master"),Master); Config.GetFloat(Section,TEXT("Effects"),Effects);
    Config.GetFloat(Section,TEXT("UI"),UI); Config.GetFloat(Section,TEXT("TextScale"),TextScale);
    Config.GetBool(Section,TEXT("HighContrast"),HighContrast); Normalize();
}
bool FLHPresentationSettings::Save(const FString& File) const
{
    using namespace LHPresentationPrivate;
    auto V=*this; V.Normalize(); FConfigFile Config; Config.Read(File); Config.bCanSaveAllSections=true;
    Config.SetFloat(Section,TEXT("Master"),V.Master); Config.SetFloat(Section,TEXT("Effects"),V.Effects);
    Config.SetFloat(Section,TEXT("UI"),V.UI); Config.SetFloat(Section,TEXT("TextScale"),V.TextScale);
    Config.SetBool(Section,TEXT("HighContrast"),V.HighContrast);
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true); return Config.Write(File);
}
FString FLHPresentationSettings::UserFile() { return FPaths::GeneratedConfigDir()/TEXT("LighthavenPresentation.ini"); }
FLHPresentationSettings& FLHPresentationSettings::Get()
{
    static FLHPresentationSettings V; static bool Loaded=false;
    if(!Loaded) { V.Load(UserFile()); Loaded=true; } return V;
}
void FLHPresentationCues::Observe(const FLHUIHud& H,const FLHInteger& L,float Delta)
{
    using namespace LHPresentationPrivate;
    Remaining=FMath::Max(0.f,Remaining-Delta); if(Remaining==0) Notice.Empty();
    const float HP=Fraction(H.Health,H.MaxHealth), MP=Fraction(H.Mana,H.MaxMana);
    if(Initialized)
    {
        if(H.Health.Resolution==ELHValueResolution::Resolved && Previous.Health.Resolution==ELHValueResolution::Resolved && H.Health.Value!=Previous.Health.Value) { ++ResourceEvents; if(Enabled){Notice=TEXT("Health changed"); Remaining=1.5f;} }
        if(H.Mana.Resolution==ELHValueResolution::Resolved && Previous.Mana.Resolution==ELHValueResolution::Resolved && H.Mana.Value!=Previous.Mana.Value) { ++ResourceEvents; if(Enabled){Notice=TEXT("Mana changed"); Remaining=1.5f;} }
        if(Enabled && H.bDead && !Previous.bDead) { Notice=TEXT("Defeated — open death menu to return"); Remaining=3; }
        if(Enabled && L.Resolution==ELHValueResolution::Resolved && L.Value>Level) { Notice=TEXT("Level up — Level ")+FString::Printf(TEXT("%lld"),L.Value); Remaining=3; }
        Health=FMath::FInterpConstantTo(Health,HP,Delta,2); Mana=FMath::FInterpConstantTo(Mana,MP,Delta,2);
    }
    else { Health=HP; Mana=MP; }
    Previous=H; if(L.Resolution==ELHValueResolution::Resolved) Level=L.Value; Initialized=true;
}
void FLHPresentationCues::Hit(bool Landed,double Damage)
{
    if(!Enabled) return;
    ++HitEvents; FLHPresentationAudio::Cue("Combat.Impact",false); Notice=Landed?FString::Printf(TEXT("Impact: %.0f damage"),Damage):TEXT("Miss"); Remaining=1.5f;
}

TFunction<void(FName,float)> FLHPresentationAudio::Sink;
void FLHPresentationAudio::Cue(FName Id,bool IsUI)
{
    const float Gain=FLHPresentationSettings::Get().Gain(IsUI);
    if(Sink && Gain>0) Sink(Id,Gain);
}
