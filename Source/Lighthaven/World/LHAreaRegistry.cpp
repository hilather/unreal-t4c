#include "LHAreaRegistry.h"
namespace LHAreaRegistryPrivate
{
FLHAreaId Area(const TCHAR* Name) { FLHAreaId A; A.Content.Value=Name; return A; }
FLHEntranceId Entrance(const TCHAR* Name, const TCHAR* Local) { FLHEntranceId E; E.Area=Area(Name); E.LocalId=Local; return E; }
FLHFieldProvenance Prototype()
{
    FLHFieldProvenance P; P.Status=ELHProvenanceStatus::Prototype;
    P.SourceBaseline=TEXT("V-01 / LH_Prototype_v1 / W3-04"); P.RetrievedDate=TEXT("2026-10-08");
    P.Notes=TEXT("Layout ground pivot, not verified in play. Host collision/enemy-safety validation required."); return P;
}
TArray<FLHAreaDefinition> MakeRegistry()
{
    TArray<FLHAreaDefinition> Result;
    { FLHAreaDefinition A; A.Id=Area(TEXT("Area.LighthavenTempleDistrict")); A.Map=FSoftObjectPath(TEXT("/Game/Lighthaven/Maps/L_LighthavenTempleDistrict.L_LighthavenTempleDistrict"));
      { FLHEntranceDefinition E; E.Id=Entrance(TEXT("Area.LighthavenTempleDistrict"),TEXT("Temple.SafeSpawn")); E.TransformResolution=ELHValueResolution::Resolved; E.SafeTransform=FTransform(FRotator(0,90,0),FVector(800,500,0)); E.Provenance=Prototype(); A.Entrances.Add(E); }
      { FLHEntranceDefinition E; E.Id=Entrance(TEXT("Area.LighthavenTempleDistrict"),TEXT("Temple.Descent")); E.TransformResolution=ELHValueResolution::Resolved; E.SafeTransform=FTransform(FRotator(0,0,0),FVector(-400,500,0)); E.Provenance=Prototype(); A.Entrances.Add(E); }
      A.SafeFallback=A.Entrances[0].Id;
      Result.Add(MoveTemp(A)); }
    { FLHAreaDefinition A; A.Id=Area(TEXT("Area.TempleB1")); A.Map=FSoftObjectPath(TEXT("/Game/Lighthaven/Maps/L_TempleB1.L_TempleB1"));
      { FLHEntranceDefinition E; E.Id=Entrance(TEXT("Area.TempleB1"),TEXT("Entry")); E.TransformResolution=ELHValueResolution::Resolved; E.SafeTransform=FTransform(FRotator(0,90,0),FVector(450.0,-1650.0,0)); E.Provenance=Prototype(); A.Entrances.Add(E); }
      { FLHEntranceDefinition E; E.Id=Entrance(TEXT("Area.TempleB1"),TEXT("Descent")); E.TransformResolution=ELHValueResolution::Resolved; E.SafeTransform=FTransform(FRotator(0,-90,0),FVector(4500,1400,0)); E.Provenance=Prototype(); A.Entrances.Add(E); }
      A.SafeFallback=A.Entrances[0].Id;
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.01"); FGuid::Parse(TEXT("bf36087d7eb24f6ea2e7c07103873227"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(300.0,300.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.02"); FGuid::Parse(TEXT("88644d7a3ae645af974dd7511596167a"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(600.0,300.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.03"); FGuid::Parse(TEXT("a617647a10b64634999dc71df2bbf36c"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1200.0,300.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.04"); FGuid::Parse(TEXT("547c0e1111e247048dd80a5e413dd6a2"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1500.0,400.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.05"); FGuid::Parse(TEXT("1b3a7b8385c846d796a985398cc44012"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(300.0,1400.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.06"); FGuid::Parse(TEXT("7923806dc5d04f61972176663d2cabc4"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(600.0,1500.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.07"); FGuid::Parse(TEXT("381587dd28a641e297175e1bfa57f081"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1200.0,1400.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.08"); FGuid::Parse(TEXT("c31db6d3e1484dccb8e787a805838d47"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1500.0,1500.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.09"); FGuid::Parse(TEXT("09a280c69426461ca75880a5753d08a5"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(3300.0,400.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.10"); FGuid::Parse(TEXT("5fda812916144bf193c7e431431c843d"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(3800.0,400.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.11"); FGuid::Parse(TEXT("e8ff0750a4ba4684bd95c61e49cb5d20"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(3400.0,1900.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.BrownRat.12"); FGuid::Parse(TEXT("64462184aec1481caa2275b644109639"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(3700.0,2300.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.Bat.01"); FGuid::Parse(TEXT("8c95497840044140bc1ddaac935bcc96"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Bat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(900.0,1500.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.Bat.02"); FGuid::Parse(TEXT("2cefccd5562c42c0a2ef1750ad337d83"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Bat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(5100.0,500.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.Bat.03"); FGuid::Parse(TEXT("6be9983f4c4b4a6b9eec217e9a157d99"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Bat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(5200.0,1800.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.GreenSlime.01"); FGuid::Parse(TEXT("c8ab0a145a494ba19167de5e0dc0ac08"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GreenSlime"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1400.0,800.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B1.Spawn.GreenSlime.02"); FGuid::Parse(TEXT("04da4b221a6a45cd970feec6dd56925c"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GreenSlime"); S.Anchor=FTransform(FRotator(0,90,0),FVector(3500.0,1400.0,0.0)); A.Spawns.Add(S); }
      Result.Add(MoveTemp(A)); }
    { FLHAreaDefinition A; A.Id=Area(TEXT("Area.TempleB2")); A.Map=FSoftObjectPath(TEXT("/Game/Lighthaven/Maps/L_TempleB2.L_TempleB2"));
      { FLHEntranceDefinition E; E.Id=Entrance(TEXT("Area.TempleB2"),TEXT("Entry")); E.TransformResolution=ELHValueResolution::Resolved; E.SafeTransform=FTransform(FRotator(0,90,0),FVector(500,-5450.0,0)); E.Provenance=Prototype(); A.Entrances.Add(E); }
      { FLHEntranceDefinition E; E.Id=Entrance(TEXT("Area.TempleB2"),TEXT("Descent")); E.TransformResolution=ELHValueResolution::Resolved; E.SafeTransform=FTransform(FRotator(0,-90,0),FVector(5300,5750.0,0)); E.Provenance=Prototype(); A.Entrances.Add(E); }
      A.SafeFallback=A.Entrances[0].Id;
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.BrownRat.01"); FGuid::Parse(TEXT("8fccefae310d497ab6835868ddab59fe"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(350.0,-3000.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.BrownRat.02"); FGuid::Parse(TEXT("dc32dec46f4e47b3956ba0eaaeaaaed5"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1800.0,-3000.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.BrownRat.03"); FGuid::Parse(TEXT("780c95e4a9e04062981410323b06c569"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(400.0,-1600.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.BrownRat.04"); FGuid::Parse(TEXT("7427f49b5c6d460db1b2d34bbd806fa7"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1800.0,-1600.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.BrownRat.05"); FGuid::Parse(TEXT("7cafb7e2e46a42eb94057523f3071764"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-2600.0,-2700.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.BrownRat.06"); FGuid::Parse(TEXT("b0fde1a58f1c4cc29a7bf5e47b34a06f"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-1200.0,-2000.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.Bat.01"); FGuid::Parse(TEXT("62e451557a55420a807312bd2822f137"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Bat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(500.0,500.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.Bat.02"); FGuid::Parse(TEXT("c0fff622fdc74acebae524e74468db1f"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Bat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1800.0,1500.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.Bat.03"); FGuid::Parse(TEXT("bd7c1f9c7e054bd9a60c4016a5828952"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Bat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-3500.0,4800.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.GreenSlime.01"); FGuid::Parse(TEXT("3bcdce577e6b4c4ca1c730e049d78902"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GreenSlime"); S.Anchor=FTransform(FRotator(0,90,0),FVector(400.0,-2300.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.GreenSlime.02"); FGuid::Parse(TEXT("a5bf72cced8a45a9840bbb17511f9e10"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GreenSlime"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1800.0,400.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.GreenSlime.03"); FGuid::Parse(TEXT("38f4d26127fa443888f6ee802f40d53d"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GreenSlime"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-2400.0,-1800.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.GiantBat.01"); FGuid::Parse(TEXT("e8bb9d7772d646748418036e61ad75b3"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-2400.0,1500.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.GiantBat.02"); FGuid::Parse(TEXT("47eb779f8caf4f0baa7b6ed065523e9e"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-1300.0,4200.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.GiantBat.03"); FGuid::Parse(TEXT("f62d45d285c9459480b521c922fbc677"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(600.0,5800.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.GiantBat.04"); FGuid::Parse(TEXT("54e69b68096d41d68b5b453bef535737"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(2900.0,6000.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.UndeadBat.01"); FGuid::Parse(TEXT("b5b7640c1c784a7c8fa0180afbc6a501"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.UndeadBat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(200.0,5200.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.UndeadBat.02"); FGuid::Parse(TEXT("a7418fd006094ce09dd427ae0fa5ecbe"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.UndeadBat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1500.0,6000.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.UndeadBat.03"); FGuid::Parse(TEXT("38712490d2f847c6b12166a9fdbb420a"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.UndeadBat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(2500.0,5200.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.UndeadBat.04"); FGuid::Parse(TEXT("f71969ee6e904ba8aeed9a301ec60432"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.UndeadBat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(500.0,1500.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.GiantSpider.01"); FGuid::Parse(TEXT("2190358c5b7943389836ab0ed25c0430"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantSpider"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-2300.0,500.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.GiantSpider.02"); FGuid::Parse(TEXT("09855e71862c4a76bfc3ddf705e9717a"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantSpider"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-3400.0,4100.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.GiantSpider.03"); FGuid::Parse(TEXT("09ea0a1bf15b42dba4a8289d88530a12"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantSpider"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1700.0,5400.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.DungeonBat.01"); FGuid::Parse(TEXT("d85b1bdbbae144da9c4e1ee2d0f9b3fe"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.DungeonBat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-1400.0,3500.0,0.0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B2.Spawn.DungeonBat.02"); FGuid::Parse(TEXT("c6e036fff619409380a1e82f685ea6e9"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.DungeonBat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-1200.0,-2700.0,0.0)); A.Spawns.Add(S); }
      Result.Add(MoveTemp(A)); }
    { FLHAreaDefinition A; A.Id=Area(TEXT("Area.TempleB3")); A.Map=FSoftObjectPath(TEXT("/Game/Lighthaven/Maps/L_TempleB3.L_TempleB3"));
      { FLHEntranceDefinition E; E.Id=Entrance(TEXT("Area.TempleB3"),TEXT("Entry")); E.TransformResolution=ELHValueResolution::Resolved; E.SafeTransform=FTransform(FRotator(0,-90,0),FVector(3200,500,0)); E.Provenance=Prototype(); A.Entrances.Add(E); }
      { FLHEntranceDefinition E; E.Id=Entrance(TEXT("Area.TempleB3"),TEXT("Descent")); E.TransformResolution=ELHValueResolution::Resolved; E.SafeTransform=FTransform(FRotator(0,-90,0),FVector(4700,6100,0)); E.Provenance=Prototype(); A.Entrances.Add(E); }
      A.SafeFallback=A.Entrances[0].Id;
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Rat.01"); FGuid::Parse(TEXT("54a31c7602124540b025b755c08a77aa"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(1200.0,-800.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Rat.02"); FGuid::Parse(TEXT("0dedc33dd30c4ade9ce75ba6634fdfa9"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,180,0),FVector(1650.0,-350.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Rat.03"); FGuid::Parse(TEXT("021f5d18a69d41d28d8afd268a1021dc"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,-90,0),FVector(1500.0,1700.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Rat.04"); FGuid::Parse(TEXT("3d8814563e6740e9a019ed6918535bda"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-2000.0,2700.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Slime.01"); FGuid::Parse(TEXT("8be7c543236d43cb8bc303f7792e7476"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GreenSlime"); S.Anchor=FTransform(FRotator(0,0,0),FVector(-600.0,-100.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Slime.02"); FGuid::Parse(TEXT("b14e1fe5a2d5481b85b63e576bd28136"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GreenSlime"); S.Anchor=FTransform(FRotator(0,180,0),FVector(500.0,2600.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Slime.03"); FGuid::Parse(TEXT("699b1293d51d4ed9ad01e4cc963f3db8"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GreenSlime"); S.Anchor=FTransform(FRotator(0,180,0),FVector(5500.0,2900.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.GiantBat.01"); FGuid::Parse(TEXT("24e196e104014f229642545fd0d87425"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,0,0),FVector(-500.0,800.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.GiantBat.02"); FGuid::Parse(TEXT("49747db56bd6499f8f05ffc72c5a16ed"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,0,0),FVector(-3800.0,3000.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.GiantBat.03"); FGuid::Parse(TEXT("17223428f081481d88184dd54bee4728"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,-90,0),FVector(2400.0,4300.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.GiantBat.04"); FGuid::Parse(TEXT("9489c162999a4169b3d6797408586245"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,180,0),FVector(5500.0,3800.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Goblin.01"); FGuid::Parse(TEXT("7f7b3de0395b4066974cc9147725ce9e"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Goblin"); S.Anchor=FTransform(FRotator(0,180,0),FVector(2100.0,200.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Goblin.02"); FGuid::Parse(TEXT("8fec7d37439a49a4894116b6329b0e64"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Goblin"); S.Anchor=FTransform(FRotator(0,180,0),FVector(200.0,-300.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Goblin.03"); FGuid::Parse(TEXT("59fb1b644dd24b1386bba14002c4b39e"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Goblin"); S.Anchor=FTransform(FRotator(0,90,0),FVector(-400.0,2600.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Goblin.04"); FGuid::Parse(TEXT("b2b308c1474d4bd0b2fc3e6c74015972"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Goblin"); S.Anchor=FTransform(FRotator(0,0,0),FVector(400.0,3400.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Goblin.05"); FGuid::Parse(TEXT("87dce5536f3b434393177010b9e33a7d"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Goblin"); S.Anchor=FTransform(FRotator(0,180,0),FVector(3300.0,3400.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Goblin.06"); FGuid::Parse(TEXT("43a64c60c9ff44db986cb1217c777d39"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Goblin"); S.Anchor=FTransform(FRotator(0,90,0),FVector(4500.0,2800.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.GoblinWarrior.01"); FGuid::Parse(TEXT("4af96671aaf341099816b0be719c5235"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GoblinWarrior"); S.Anchor=FTransform(FRotator(0,180,0),FVector(-1600.0,3300.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.GoblinWarrior.02"); FGuid::Parse(TEXT("f8aaabcd3b71411a8e8332048d1f5042"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GoblinWarrior"); S.Anchor=FTransform(FRotator(0,180,0),FVector(-3200.0,3400.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.GoblinWarrior.03"); FGuid::Parse(TEXT("c129733c4c6a49b693c7fb2933399050"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GoblinWarrior"); S.Anchor=FTransform(FRotator(0,-90,0),FVector(3300.0,4300.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Atrocity.01"); FGuid::Parse(TEXT("f9bea13ecb9545018333d02c12cf97c1"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Atrocity"); S.Anchor=FTransform(FRotator(0,90,0),FVector(2000.0,-900.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B3.Atrocity.02"); FGuid::Parse(TEXT("a17f538064804509a4c3e2ac6433c4c3"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Atrocity"); S.Anchor=FTransform(FRotator(0,-90,0),FVector(4500.0,3800.0,0)); A.Spawns.Add(S); }
      Result.Add(MoveTemp(A)); }
    { FLHAreaDefinition A; A.Id=Area(TEXT("Area.TempleB4")); A.Map=FSoftObjectPath(TEXT("/Game/Lighthaven/Maps/L_TempleB4.L_TempleB4"));
      { FLHEntranceDefinition E; E.Id=Entrance(TEXT("Area.TempleB4"),TEXT("Entry")); E.TransformResolution=ELHValueResolution::Resolved; E.SafeTransform=FTransform(FRotator(0,0,0),FVector(650.0,600,0)); E.Provenance=Prototype(); A.Entrances.Add(E); }
      A.SafeFallback=A.Entrances[0].Id;
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.Rat.01"); FGuid::Parse(TEXT("e87bcae47524436599aeedda21e227b6"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(500.0,2700.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.Rat.02"); FGuid::Parse(TEXT("816ebf68462f47d29c423f89560e44ba"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,0,0),FVector(-2300.0,2800.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.Rat.03"); FGuid::Parse(TEXT("3970e648b6c94e1dbcf0593d12666d9a"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.BrownRat"); S.Anchor=FTransform(FRotator(0,90,0),FVector(4900.0,2800.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.Slime.01"); FGuid::Parse(TEXT("4298282572b5480786ca87fbb372f71d"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GreenSlime"); S.Anchor=FTransform(FRotator(0,-90,0),FVector(-1300.0,3900.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.Slime.02"); FGuid::Parse(TEXT("fe33cf57f68344da8a0aa58ea68e1e9b"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GreenSlime"); S.Anchor=FTransform(FRotator(0,180,0),FVector(5100.0,3800.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.GiantBat.01"); FGuid::Parse(TEXT("eb513875896a403395200d9ab85a281e"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,180,0),FVector(1800.0,2700.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.GiantBat.02"); FGuid::Parse(TEXT("a3d68bab72d443d28a352603268352f2"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,-90,0),FVector(-2200.0,3800.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.GiantBat.03"); FGuid::Parse(TEXT("470c8615d01d4c77a0f2759a092bb526"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,-90,0),FVector(4500.0,4100.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.GiantBat.04"); FGuid::Parse(TEXT("98ff591eb49645989ae2f664a19c7a3d"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.GiantBat"); S.Anchor=FTransform(FRotator(0,180,0),FVector(4200.0,8000.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.Atrocity.01"); FGuid::Parse(TEXT("1b57ddae8d4d4f1e82679888899ceef2"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Atrocity"); S.Anchor=FTransform(FRotator(0,180,0),FVector(-1400.0,2900.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.Atrocity.02"); FGuid::Parse(TEXT("b30be138283c4991b4a4ee35793d1604"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Atrocity"); S.Anchor=FTransform(FRotator(0,90,0),FVector(4800.0,3400.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.Atrocity.03"); FGuid::Parse(TEXT("56d2c71d54284bbdb45c42d28d0a37d4"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Atrocity"); S.Anchor=FTransform(FRotator(0,90,0),FVector(3800.0,5700.0,0)); A.Spawns.Add(S); }
      { FLHSpawnAuthoring S; S.Alias=TEXT("B4.Balork.01"); FGuid::Parse(TEXT("77146f171dc04cd289f7357ba15ce45c"),S.SpawnId); S.Enemy.Value=TEXT("Enemy.Balork"); S.Anchor=FTransform(FRotator(0,-90,0),FVector(1200.0,6600.0,0)); A.Spawns.Add(S); }
      Result.Add(MoveTemp(A)); }
    { FLHPortalDefinition P; P.Portal.Area=Result[0].Id; FGuid::Parse(TEXT("b1019c84a8c44227b9d5609e785c0afc"),P.Portal.InstanceId); P.Source=Entrance(TEXT("Area.LighthavenTempleDistrict"),TEXT("Temple.Descent")); P.Destination=Entrance(TEXT("Area.TempleB1"),TEXT("Entry")); P.Provenance=Prototype(); Result[0].Portals.Add(P); }
    { FLHPortalDefinition P; P.Portal.Area=Result[1].Id; FGuid::Parse(TEXT("585327fd66244ba8a3cd35fac8035623"),P.Portal.InstanceId); P.Source=Entrance(TEXT("Area.TempleB1"),TEXT("Entry")); P.Destination=Entrance(TEXT("Area.LighthavenTempleDistrict"),TEXT("Temple.Descent")); P.Provenance=Prototype(); Result[1].Portals.Add(P); }
    { FLHPortalDefinition P; P.Portal.Area=Result[1].Id; FGuid::Parse(TEXT("bab1ef9966b94cdb8aa430e4d377dd2a"),P.Portal.InstanceId); P.Source=Entrance(TEXT("Area.TempleB1"),TEXT("Descent")); P.Destination=Entrance(TEXT("Area.TempleB2"),TEXT("Entry")); P.Provenance=Prototype(); Result[1].Portals.Add(P); }
    { FLHPortalDefinition P; P.Portal.Area=Result[2].Id; FGuid::Parse(TEXT("cf7f7eb1cb924a75ae845e923ec4bf53"),P.Portal.InstanceId); P.Source=Entrance(TEXT("Area.TempleB2"),TEXT("Entry")); P.Destination=Entrance(TEXT("Area.TempleB1"),TEXT("Descent")); P.Provenance=Prototype(); Result[2].Portals.Add(P); }
    { FLHPortalDefinition P; P.Portal.Area=Result[2].Id; FGuid::Parse(TEXT("c57ef8301c6f4e1b8b563183d6b3359c"),P.Portal.InstanceId); P.Source=Entrance(TEXT("Area.TempleB2"),TEXT("Descent")); P.Destination=Entrance(TEXT("Area.TempleB3"),TEXT("Entry")); P.Provenance=Prototype(); Result[2].Portals.Add(P); }
    { FLHPortalDefinition P; P.Portal.Area=Result[3].Id; FGuid::Parse(TEXT("522322ad6eca4555b718b6e41d377583"),P.Portal.InstanceId); P.Source=Entrance(TEXT("Area.TempleB3"),TEXT("Entry")); P.Destination=Entrance(TEXT("Area.TempleB2"),TEXT("Descent")); P.Provenance=Prototype(); Result[3].Portals.Add(P); }
    { FLHPortalDefinition P; P.Portal.Area=Result[3].Id; FGuid::Parse(TEXT("065d9c0b4e9840248486f0ee9a3a040a"),P.Portal.InstanceId); P.Source=Entrance(TEXT("Area.TempleB3"),TEXT("Descent")); P.Destination=Entrance(TEXT("Area.TempleB4"),TEXT("Entry")); P.Provenance=Prototype(); Result[3].Portals.Add(P); }
    { FLHPortalDefinition P; P.Portal.Area=Result[4].Id; FGuid::Parse(TEXT("cf173017211e4a26b53e833eb475e2b4"),P.Portal.InstanceId); P.Source=Entrance(TEXT("Area.TempleB4"),TEXT("Entry")); P.Destination=Entrance(TEXT("Area.TempleB3"),TEXT("Descent")); P.Provenance=Prototype(); Result[4].Portals.Add(P); }
    return Result;
}
}
namespace LHWorld
{
const TArray<FLHAreaDefinition>& Registry() { static const auto R=LHAreaRegistryPrivate::MakeRegistry(); return R; }
bool SameArea(const FLHAreaId& A,const FLHAreaId& B) { return A.Content.Value.ToString()==B.Content.Value.ToString(); }
bool SameEntrance(const FLHEntranceId& A,const FLHEntranceId& B) { return SameArea(A.Area,B.Area) && A.LocalId.ToString()==B.LocalId.ToString(); }
bool SafeTransform(const FTransform& T)
{
    return !T.ContainsNaN() && T.GetRotation().IsNormalized() && T.GetScale3D().Equals(FVector::OneVector);
}
const FLHAreaDefinition* FindArea(const FLHAreaId& Id) { return Registry().FindByPredicate([&](const auto& A) { return SameArea(A.Id,Id); }); }
const FLHEntranceDefinition* FindEntrance(const FLHEntranceId& Id)
{
    const auto* A=FindArea(Id.Area); return A ? A->Entrances.FindByPredicate([&](const auto& E) { return SameEntrance(E.Id,Id); }) : nullptr;
}
const FLHPortalDefinition* FindPortal(const FLHEntityId& Id)
{
    const auto* A=FindArea(Id.Area); return A ? A->Portals.FindByPredicate([&](const auto& P) { return P.Portal.InstanceId==Id.InstanceId; }) : nullptr;
}
bool ValidateRegistry(TArray<FString>& Errors)
{
    Errors.Reset(); TSet<FName> Areas; TSet<FGuid> Ids;
    if (Registry().Num()!=5) Errors.Add(TEXT("Expected five playable areas"));
    for (const auto& A:Registry())
    {
        if (Areas.Contains(A.Id.Content.Value) || !A.Map.IsValid()) Errors.Add(TEXT("Duplicate area or invalid map path"));
        Areas.Add(A.Id.Content.Value);
        if (!FindEntrance(A.SafeFallback)) Errors.Add(TEXT("Missing fallback entrance"));
        TSet<FName> Entrances;
        for (const auto& E:A.Entrances)
        {
            if (!SameArea(E.Id.Area,A.Id) || E.Id.LocalId.IsNone() || Entrances.Contains(E.Id.LocalId) || E.TransformResolution!=ELHValueResolution::Resolved || !SafeTransform(E.SafeTransform)) Errors.Add(TEXT("Invalid entrance transform/identity"));
            Entrances.Add(E.Id.LocalId);
        }
        for (const auto& S:A.Spawns)
        {
            if (!S.SpawnId.IsValid() || Ids.Contains(S.SpawnId) || S.Enemy.Value.IsNone() || !SafeTransform(S.Anchor)) Errors.Add(TEXT("Invalid/duplicate spawn"));
            Ids.Add(S.SpawnId);
        }
        for (const auto& P:A.Portals)
        {
            if (!P.Portal.InstanceId.IsValid() || Ids.Contains(P.Portal.InstanceId) || P.Portal.RunId.IsValid() || !SameArea(P.Source.Area,A.Id) || !SameArea(P.Portal.Area,A.Id) || !FindEntrance(P.Source) || !FindEntrance(P.Destination)) Errors.Add(TEXT("Invalid portal identity/endpoints"));
            Ids.Add(P.Portal.InstanceId);
            int32 Pairs=0;
            for (const auto& B:Registry()) for (const auto& Reverse:B.Portals)
                if (SameEntrance(P.Source,Reverse.Destination) && SameEntrance(P.Destination,Reverse.Source)) ++Pairs;
            if (Pairs!=1) Errors.Add(TEXT("Portal needs exactly one reverse edge"));
        }
    }
    return Errors.IsEmpty();
}
}
