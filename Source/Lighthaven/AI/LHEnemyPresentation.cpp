#include "AI/LHEnemyPresentation.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
namespace LHEnemyPresentationPrivate {
struct FPiece { const TCHAR* Id; const TCHAR* Mesh; FVector Size, Center; FQuat Rotation; FColor Color; bool Weapon; };
const FPiece Pieces[] = {
{TEXT("BrownRat"),TEXT("Sphere"),FVector(35.0,24.0,23.0),FVector(-4.0,0.0,12.5),FQuat::Identity,FColor(0x59,0x45,0x37),false},
{TEXT("BrownRat"),TEXT("Sphere"),FVector(16.0,18.0,16.0),FVector(10.0,0.0,12.0),FQuat::Identity,FColor(0x59,0x45,0x37),false},
{TEXT("BrownRat"),TEXT("Cone"),FVector(10.0,10.0,6.5),FVector(19.25,0.0,12.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(6.5,0.0,0.0).GetSafeNormal()),FColor(0x9A,0x78,0x65),false},
{TEXT("BrownRat"),TEXT("Cube"),FVector(24.0,14.0,2.0),FVector(-5.0,0.0,23.0),FQuat::Identity,FColor(0x80,0x68,0x50),false},
{TEXT("BrownRat"),TEXT("Cylinder"),FVector(5.0,5.0,45.17742799230607),FVector(-42.5,0.0,6.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-45.0,0.0,-4.0).GetSafeNormal()),FColor(0x9A,0x78,0x65),false},
{TEXT("BrownRat"),TEXT("Sphere"),FVector(7.0,7.0,7.0),FVector(4.0,-10.5,21.0),FQuat::Identity,FColor(0x9A,0x78,0x65),false},
{TEXT("BrownRat"),TEXT("Cylinder"),FVector(5.0,5.0,6.0),FVector(-12.0,-9.0,3.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,6.0).GetSafeNormal()),FColor(0x9A,0x78,0x65),false},
{TEXT("BrownRat"),TEXT("Cylinder"),FVector(5.0,5.0,6.0),FVector(10.0,-9.0,3.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,6.0).GetSafeNormal()),FColor(0x9A,0x78,0x65),false},
{TEXT("BrownRat"),TEXT("Sphere"),FVector(7.0,7.0,7.0),FVector(4.0,10.5,21.0),FQuat::Identity,FColor(0x9A,0x78,0x65),false},
{TEXT("BrownRat"),TEXT("Cylinder"),FVector(5.0,5.0,6.0),FVector(-12.0,9.0,3.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,6.0).GetSafeNormal()),FColor(0x9A,0x78,0x65),false},
{TEXT("BrownRat"),TEXT("Cylinder"),FVector(5.0,5.0,6.0),FVector(10.0,9.0,3.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,6.0).GetSafeNormal()),FColor(0x9A,0x78,0x65),false},
{TEXT("Bat"),TEXT("Sphere"),FVector(24.75,20.0,36.0),FVector(0.0,0.0,100.0),FQuat::Identity,FColor(0x51,0x42,0x36),false},
{TEXT("Bat"),TEXT("Sphere"),FVector(12.0,14.0,12.0),FVector(16.5,0.0,100.0),FQuat::Identity,FColor(0xAF,0x91,0x69),false},
{TEXT("Bat"),TEXT("Cone"),FVector(10.0,10.0,20.0),FVector(2.0,-7.0,125.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,20.0).GetSafeNormal()),FColor(0x51,0x42,0x36),false},
{TEXT("Bat"),TEXT("Cube"),FVector(34.0,21.0,5.0),FVector(0.0,-20.5,100.0),FQuat::Identity,FColor(0x92,0x74,0x51),false},
{TEXT("Bat"),TEXT("Cone"),FVector(24.0,24.0,15.0),FVector(0.0,-32.5,100.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,-15.0,0.0).GetSafeNormal()),FColor(0x92,0x74,0x51),false},
{TEXT("Bat"),TEXT("Cone"),FVector(10.0,10.0,20.0),FVector(2.0,7.0,125.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,20.0).GetSafeNormal()),FColor(0x51,0x42,0x36),false},
{TEXT("Bat"),TEXT("Cube"),FVector(34.0,21.0,5.0),FVector(0.0,20.5,100.0),FQuat::Identity,FColor(0x92,0x74,0x51),false},
{TEXT("Bat"),TEXT("Cone"),FVector(24.0,24.0,15.0),FVector(0.0,32.5,100.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,15.0,0.0).GetSafeNormal()),FColor(0x92,0x74,0x51),false},
{TEXT("DungeonBat"),TEXT("Sphere"),FVector(30.25,20.0,36.0),FVector(0.0,0.0,110.0),FQuat::Identity,FColor(0x49,0x4B,0x48),false},
{TEXT("DungeonBat"),TEXT("Sphere"),FVector(12.0,14.0,12.0),FVector(21.5,0.0,110.0),FQuat::Identity,FColor(0xB7,0xA9,0x87),false},
{TEXT("DungeonBat"),TEXT("Cone"),FVector(10.0,10.0,20.0),FVector(2.0,-7.0,135.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,20.0).GetSafeNormal()),FColor(0x49,0x4B,0x48),false},
{TEXT("DungeonBat"),TEXT("Cube"),FVector(44.0,39.0,6.0),FVector(-2.0,-30.5,110.0),FQuat::Identity,FColor(0x66,0x64,0x53),false},
{TEXT("DungeonBat"),TEXT("Cylinder"),FVector(10.0,10.0,42.40283009422838),FVector(1.5,-15.0,123.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-27.0,-30.0,-13.0).GetSafeNormal()),FColor(0xB7,0xA9,0x87),false},
{TEXT("DungeonBat"),TEXT("Cone"),FVector(10.0,10.0,20.0),FVector(2.0,7.0,135.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,20.0).GetSafeNormal()),FColor(0x49,0x4B,0x48),false},
{TEXT("DungeonBat"),TEXT("Cube"),FVector(44.0,39.0,6.0),FVector(-2.0,30.5,110.0),FQuat::Identity,FColor(0x66,0x64,0x53),false},
{TEXT("DungeonBat"),TEXT("Cylinder"),FVector(10.0,10.0,42.40283009422838),FVector(1.5,15.0,123.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-27.0,30.0,-13.0).GetSafeNormal()),FColor(0xB7,0xA9,0x87),false},
{TEXT("GiantBat"),TEXT("Sphere"),FVector(46.75,32.0,52.0),FVector(0.0,0.0,130.0),FQuat::Identity,FColor(0x45,0x37,0x2D),false},
{TEXT("GiantBat"),TEXT("Sphere"),FVector(12.0,14.0,12.0),FVector(36.5,0.0,130.0),FQuat::Identity,FColor(0xB2,0x98,0x67),false},
{TEXT("GiantBat"),TEXT("Cone"),FVector(10.0,10.0,20.0),FVector(2.0,-7.0,155.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,20.0).GetSafeNormal()),FColor(0x45,0x37,0x2D),false},
{TEXT("GiantBat"),TEXT("Cube"),FVector(64.0,41.0,8.0),FVector(-3.0,-36.5,130.0),FQuat::Identity,FColor(0x77,0x56,0x38),false},
{TEXT("GiantBat"),TEXT("Cone"),FVector(44.0,44.0,34.0),FVector(-3.0,-68.0,130.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,-34.0,0.0).GetSafeNormal()),FColor(0x77,0x56,0x38),false},
{TEXT("GiantBat"),TEXT("Cube"),FVector(12.0,45.0,3.0),FVector(23.0,-38.5,136.0),FQuat::Identity,FColor(0xB2,0x98,0x67),false},
{TEXT("GiantBat"),TEXT("Cone"),FVector(10.0,10.0,20.0),FVector(2.0,7.0,155.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,20.0).GetSafeNormal()),FColor(0x45,0x37,0x2D),false},
{TEXT("GiantBat"),TEXT("Cube"),FVector(64.0,41.0,8.0),FVector(-3.0,36.5,130.0),FQuat::Identity,FColor(0x77,0x56,0x38),false},
{TEXT("GiantBat"),TEXT("Cone"),FVector(44.0,44.0,34.0),FVector(-3.0,68.0,130.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,34.0,0.0).GetSafeNormal()),FColor(0x77,0x56,0x38),false},
{TEXT("GiantBat"),TEXT("Cube"),FVector(12.0,45.0,3.0),FVector(23.0,38.5,136.0),FQuat::Identity,FColor(0xB2,0x98,0x67),false},
{TEXT("UndeadBat"),TEXT("Sphere"),FVector(33.0,20.0,36.0),FVector(0.0,0.0,115.0),FQuat::Identity,FColor(0x41,0x45,0x44),false},
{TEXT("UndeadBat"),TEXT("Sphere"),FVector(12.0,14.0,12.0),FVector(24.0,0.0,115.0),FQuat::Identity,FColor(0xB6,0xB4,0x9F),false},
{TEXT("UndeadBat"),TEXT("Cone"),FVector(10.0,10.0,20.0),FVector(2.0,-7.0,140.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,20.0).GetSafeNormal()),FColor(0x41,0x45,0x44),false},
{TEXT("UndeadBat"),TEXT("Cube"),FVector(44.0,20.0,6.0),FVector(0.0,-21.0,115.0),FQuat::Identity,FColor(0x64,0x68,0x5D),false},
{TEXT("UndeadBat"),TEXT("Cube"),FVector(24.0,24.0,6.0),FVector(10.0,-43.0,115.0),FQuat::Identity,FColor(0xB6,0xB4,0x9F),false},
{TEXT("UndeadBat"),TEXT("Cube"),FVector(20.0,12.0,6.0),FVector(-12.0,-37.0,115.0),FQuat::Identity,FColor(0xB6,0xB4,0x9F),false},
{TEXT("UndeadBat"),TEXT("Cone"),FVector(10.0,10.0,20.0),FVector(2.0,7.0,140.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,20.0).GetSafeNormal()),FColor(0x41,0x45,0x44),false},
{TEXT("UndeadBat"),TEXT("Cube"),FVector(44.0,20.0,6.0),FVector(0.0,21.0,115.0),FQuat::Identity,FColor(0x64,0x68,0x5D),false},
{TEXT("UndeadBat"),TEXT("Cube"),FVector(24.0,24.0,6.0),FVector(10.0,43.0,115.0),FQuat::Identity,FColor(0xB6,0xB4,0x9F),false},
{TEXT("UndeadBat"),TEXT("Cube"),FVector(20.0,12.0,6.0),FVector(-12.0,37.0,115.0),FQuat::Identity,FColor(0xB6,0xB4,0x9F),false},
{TEXT("GreenSlime"),TEXT("Sphere"),FVector(90.0,90.0,16.0),FVector(0.0,0.0,8.0),FQuat::Identity,FColor(0x26,0x3E,0x23),false},
{TEXT("GreenSlime"),TEXT("Sphere"),FVector(56.0,54.0,26.0),FVector(-10.0,-12.0,13.0),FQuat::Identity,FColor(0x3F,0x72,0x2E),false},
{TEXT("GreenSlime"),TEXT("Sphere"),FVector(62.0,58.0,40.0),FVector(12.0,8.0,20.0),FQuat::Identity,FColor(0x70,0x9B,0x40),false},
{TEXT("GiantSpider"),TEXT("Sphere"),FVector(66.0,62.0,50.0),FVector(-25.0,0.0,40.0),FQuat::Identity,FColor(0x66,0x64,0x5E),false},
{TEXT("GiantSpider"),TEXT("Sphere"),FVector(42.0,46.0,32.0),FVector(28.0,0.0,37.0),FQuat::Identity,FColor(0x66,0x64,0x5E),false},
{TEXT("GiantSpider"),TEXT("Sphere"),FVector(18.0,26.0,18.0),FVector(51.0,0.0,32.0),FQuat::Identity,FColor(0x8F,0x49,0x3B),false},
{TEXT("GiantSpider"),TEXT("Cube"),FVector(44.0,16.0,3.0),FVector(-25.0,0.0,63.0),FQuat::Identity,FColor(0x95,0x90,0x83),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,46.87216658103186),FVector(-46.0,-41.0,44.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-12.0,-42.0,17.0).GetSafeNormal()),FColor(0x66,0x64,0x5E),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,51.73973328110612),FVector(-55.5,-71.0,29.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-7.0,-18.0,-48.0).GetSafeNormal()),FColor(0x3D,0x3D,0x39),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,46.195237849804386),FVector(-19.5,-41.0,44.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-9.0,-42.0,17.0).GetSafeNormal()),FColor(0x66,0x64,0x5E),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,51.273774973177076),FVector(-24.5,-71.0,29.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-1.0,-18.0,-48.0).GetSafeNormal()),FColor(0x3D,0x3D,0x39),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,46.400431032480725),FVector(20.0,-41.0,44.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(10.0,-42.0,17.0).GetSafeNormal()),FColor(0x66,0x64,0x5E),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,52.04805471869242),FVector(29.5,-71.0,29.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(9.0,-18.0,-48.0).GetSafeNormal()),FColor(0x3D,0x3D,0x39),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,51.273774973177076),FVector(46.0,-41.0,44.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(24.0,-42.0,17.0).GetSafeNormal()),FColor(0x66,0x64,0x5E),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,55.39855593785816),FVector(68.5,-71.0,29.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(21.0,-18.0,-48.0).GetSafeNormal()),FColor(0x3D,0x3D,0x39),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,46.87216658103186),FVector(-46.0,41.0,44.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-12.0,42.0,17.0).GetSafeNormal()),FColor(0x66,0x64,0x5E),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,51.73973328110612),FVector(-55.5,71.0,29.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-7.0,18.0,-48.0).GetSafeNormal()),FColor(0x3D,0x3D,0x39),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,46.195237849804386),FVector(-19.5,41.0,44.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-9.0,42.0,17.0).GetSafeNormal()),FColor(0x66,0x64,0x5E),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,51.273774973177076),FVector(-24.5,71.0,29.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-1.0,18.0,-48.0).GetSafeNormal()),FColor(0x3D,0x3D,0x39),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,46.400431032480725),FVector(20.0,41.0,44.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(10.0,42.0,17.0).GetSafeNormal()),FColor(0x66,0x64,0x5E),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,52.04805471869242),FVector(29.5,71.0,29.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(9.0,18.0,-48.0).GetSafeNormal()),FColor(0x3D,0x3D,0x39),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,51.273774973177076),FVector(46.0,41.0,44.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(24.0,42.0,17.0).GetSafeNormal()),FColor(0x66,0x64,0x5E),false},
{TEXT("GiantSpider"),TEXT("Cylinder"),FVector(8.0,8.0,55.39855593785816),FVector(68.5,71.0,29.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(21.0,18.0,-48.0).GetSafeNormal()),FColor(0x3D,0x3D,0x39),false},
{TEXT("Goblin"),TEXT("Cylinder"),FVector(13.0,13.0,52.0),FVector(0.0,-12.0,32.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,52.0).GetSafeNormal()),FColor(0x8B,0x32,0x27),false},
{TEXT("Goblin"),TEXT("Sphere"),FVector(24.0,16.0,12.0),FVector(6.0,-12.0,6.0),FQuat::Identity,FColor(0x48,0x39,0x2C),false},
{TEXT("Goblin"),TEXT("Cylinder"),FVector(12.0,12.0,33.34666400106613),FVector(7.0,-25.0,72.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(14.0,-4.0,-30.0).GetSafeNormal()),FColor(0x8B,0x32,0x27),false},
{TEXT("Goblin"),TEXT("Cone"),FVector(13.0,13.0,19.697715603592208),FVector(0.0,-21.0,115.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,-18.0,8.0).GetSafeNormal()),FColor(0x8B,0x32,0x27),false},
{TEXT("Goblin"),TEXT("Cylinder"),FVector(13.0,13.0,52.0),FVector(0.0,12.0,32.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,52.0).GetSafeNormal()),FColor(0x8B,0x32,0x27),false},
{TEXT("Goblin"),TEXT("Sphere"),FVector(24.0,16.0,12.0),FVector(6.0,12.0,6.0),FQuat::Identity,FColor(0x48,0x39,0x2C),false},
{TEXT("Goblin"),TEXT("Cylinder"),FVector(12.0,12.0,33.34666400106613),FVector(7.0,25.0,72.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(14.0,4.0,-30.0).GetSafeNormal()),FColor(0x8B,0x32,0x27),false},
{TEXT("Goblin"),TEXT("Cone"),FVector(13.0,13.0,19.697715603592208),FVector(0.0,21.0,115.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,18.0,8.0).GetSafeNormal()),FColor(0x8B,0x32,0x27),false},
{TEXT("Goblin"),TEXT("Sphere"),FVector(34.0,40.0,52.0),FVector(0.0,0.0,79.0),FQuat::Identity,FColor(0x8B,0x32,0x27),false},
{TEXT("Goblin"),TEXT("Cube"),FVector(36.0,44.0,15.0),FVector(0.0,0.0,56.0),FQuat::Identity,FColor(0x48,0x39,0x2C),false},
{TEXT("Goblin"),TEXT("Sphere"),FVector(28.0,26.0,28.0),FVector(4.0,0.0,111.0),FQuat::Identity,FColor(0x8B,0x32,0x27),false},
{TEXT("Goblin"),TEXT("Cone"),FVector(10.0,10.0,12.0),FVector(22.0,0.0,111.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(12.0,0.0,0.0).GetSafeNormal()),FColor(0x8B,0x32,0x27),false},
{TEXT("Goblin"),TEXT("Cylinder"),FVector(6.0,6.0,160.0),FVector(24.0,30.0,95.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,160.0).GetSafeNormal()),FColor(0x48,0x39,0x2C),true},
{TEXT("Goblin"),TEXT("Cone"),FVector(14.0,14.0,20.0),FVector(24.0,30.0,175.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,20.0).GetSafeNormal()),FColor(0xB3,0xAB,0xA0),true},
{TEXT("GoblinWarrior"),TEXT("Cylinder"),FVector(13.0,13.0,52.0),FVector(0.0,-12.0,32.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,52.0).GetSafeNormal()),FColor(0x79,0x31,0x27),false},
{TEXT("GoblinWarrior"),TEXT("Sphere"),FVector(24.0,16.0,12.0),FVector(6.0,-12.0,6.0),FQuat::Identity,FColor(0x4E,0x40,0x31),false},
{TEXT("GoblinWarrior"),TEXT("Cylinder"),FVector(12.0,12.0,33.34666400106613),FVector(7.0,-25.0,72.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(14.0,-4.0,-30.0).GetSafeNormal()),FColor(0x79,0x31,0x27),false},
{TEXT("GoblinWarrior"),TEXT("Cone"),FVector(13.0,13.0,19.697715603592208),FVector(0.0,-21.0,115.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,-18.0,8.0).GetSafeNormal()),FColor(0x79,0x31,0x27),false},
{TEXT("GoblinWarrior"),TEXT("Cylinder"),FVector(13.0,13.0,52.0),FVector(0.0,12.0,32.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,52.0).GetSafeNormal()),FColor(0x79,0x31,0x27),false},
{TEXT("GoblinWarrior"),TEXT("Sphere"),FVector(24.0,16.0,12.0),FVector(6.0,12.0,6.0),FQuat::Identity,FColor(0x4E,0x40,0x31),false},
{TEXT("GoblinWarrior"),TEXT("Cylinder"),FVector(12.0,12.0,33.34666400106613),FVector(7.0,25.0,72.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(14.0,4.0,-30.0).GetSafeNormal()),FColor(0x79,0x31,0x27),false},
{TEXT("GoblinWarrior"),TEXT("Cone"),FVector(13.0,13.0,19.697715603592208),FVector(0.0,21.0,115.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,18.0,8.0).GetSafeNormal()),FColor(0x79,0x31,0x27),false},
{TEXT("GoblinWarrior"),TEXT("Sphere"),FVector(34.0,40.0,52.0),FVector(0.0,0.0,79.0),FQuat::Identity,FColor(0x79,0x31,0x27),false},
{TEXT("GoblinWarrior"),TEXT("Cube"),FVector(36.0,44.0,15.0),FVector(0.0,0.0,56.0),FQuat::Identity,FColor(0x4E,0x40,0x31),false},
{TEXT("GoblinWarrior"),TEXT("Sphere"),FVector(28.0,26.0,28.0),FVector(4.0,0.0,111.0),FQuat::Identity,FColor(0x79,0x31,0x27),false},
{TEXT("GoblinWarrior"),TEXT("Cone"),FVector(10.0,10.0,12.0),FVector(22.0,0.0,111.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(12.0,0.0,0.0).GetSafeNormal()),FColor(0x79,0x31,0x27),false},
{TEXT("GoblinWarrior"),TEXT("Cube"),FVector(32.0,28.0,20.0),FVector(-3.0,-29.0,100.0),FQuat::Identity,FColor(0x4E,0x40,0x31),false},
{TEXT("GoblinWarrior"),TEXT("Cube"),FVector(32.0,28.0,20.0),FVector(-3.0,29.0,100.0),FQuat::Identity,FColor(0x4E,0x40,0x31),false},
{TEXT("GoblinWarrior"),TEXT("Cube"),FVector(18.0,68.0,4.0),FVector(-11.0,0.0,111.0),FQuat::Identity,FColor(0xC0,0xAC,0x86),false},
{TEXT("GoblinWarrior"),TEXT("Cylinder"),FVector(8.0,8.0,190.0),FVector(28.0,34.0,115.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,190.0).GetSafeNormal()),FColor(0x4E,0x40,0x31),true},
{TEXT("GoblinWarrior"),TEXT("Cone"),FVector(16.0,16.0,27.0),FVector(28.0,25.0,196.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,27.0).GetSafeNormal()),FColor(0xB8,0xB1,0xA3),true},
{TEXT("GoblinWarrior"),TEXT("Cone"),FVector(16.0,16.0,27.0),FVector(28.0,43.0,196.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,27.0).GetSafeNormal()),FColor(0xB8,0xB1,0xA3),true},
{TEXT("Atrocity"),TEXT("Sphere"),FVector(110.0,110.0,100.0),FVector(-10.0,0.0,137.0),FQuat::Identity,FColor(0x34,0x30,0x38),false},
{TEXT("Atrocity"),TEXT("Sphere"),FVector(42.0,46.0,42.0),FVector(53.0,0.0,114.0),FQuat::Identity,FColor(0x58,0x50,0x5A),false},
{TEXT("Atrocity"),TEXT("Cone"),FVector(20.0,20.0,23.08679276123039),FVector(81.0,0.0,106.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(22.0,0.0,-7.0).GetSafeNormal()),FColor(0x34,0x30,0x38),false},
{TEXT("Atrocity"),TEXT("Sphere"),FVector(70.0,60.0,60.0),FVector(-12.0,-42.0,151.0),FQuat::Identity,FColor(0x58,0x50,0x5A),false},
{TEXT("Atrocity"),TEXT("Cylinder"),FVector(32.0,32.0,74.0),FVector(-20.0,-32.0,57.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,74.0).GetSafeNormal()),FColor(0x34,0x30,0x38),false},
{TEXT("Atrocity"),TEXT("Sphere"),FVector(52.0,40.0,24.0),FVector(0.0,-32.0,12.0),FQuat::Identity,FColor(0x34,0x30,0x38),false},
{TEXT("Atrocity"),TEXT("Cylinder"),FVector(30.0,30.0,53.749418601506754),FVector(7.0,-64.0,119.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(28.0,-16.0,-43.0).GetSafeNormal()),FColor(0x58,0x50,0x5A),false},
{TEXT("Atrocity"),TEXT("Cylinder"),FVector(24.0,24.0,47.80167361086848),FVector(40.0,-72.0,83.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(38.0,0.0,-29.0).GetSafeNormal()),FColor(0x34,0x30,0x38),false},
{TEXT("Atrocity"),TEXT("Cone"),FVector(10.0,10.0,33.12099032335839),FVector(77.5,-62.0,62.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(29.0,0.0,-16.0).GetSafeNormal()),FColor(0xBF,0xA4,0x4D),false},
{TEXT("Atrocity"),TEXT("Cone"),FVector(10.0,10.0,33.12099032335839),FVector(77.5,-69.0,62.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(29.0,0.0,-16.0).GetSafeNormal()),FColor(0xBF,0xA4,0x4D),false},
{TEXT("Atrocity"),TEXT("Cone"),FVector(10.0,10.0,33.12099032335839),FVector(77.5,-76.0,62.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(29.0,0.0,-16.0).GetSafeNormal()),FColor(0xBF,0xA4,0x4D),false},
{TEXT("Atrocity"),TEXT("Sphere"),FVector(70.0,60.0,60.0),FVector(-12.0,42.0,151.0),FQuat::Identity,FColor(0x58,0x50,0x5A),false},
{TEXT("Atrocity"),TEXT("Cylinder"),FVector(32.0,32.0,74.0),FVector(-20.0,32.0,57.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,74.0).GetSafeNormal()),FColor(0x34,0x30,0x38),false},
{TEXT("Atrocity"),TEXT("Sphere"),FVector(52.0,40.0,24.0),FVector(0.0,32.0,12.0),FQuat::Identity,FColor(0x34,0x30,0x38),false},
{TEXT("Atrocity"),TEXT("Cylinder"),FVector(30.0,30.0,53.749418601506754),FVector(7.0,64.0,119.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(28.0,16.0,-43.0).GetSafeNormal()),FColor(0x58,0x50,0x5A),false},
{TEXT("Atrocity"),TEXT("Cylinder"),FVector(24.0,24.0,47.80167361086848),FVector(40.0,72.0,83.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(38.0,0.0,-29.0).GetSafeNormal()),FColor(0x34,0x30,0x38),false},
{TEXT("Atrocity"),TEXT("Cone"),FVector(10.0,10.0,33.12099032335839),FVector(77.5,62.0,62.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(29.0,0.0,-16.0).GetSafeNormal()),FColor(0xBF,0xA4,0x4D),false},
{TEXT("Atrocity"),TEXT("Cone"),FVector(10.0,10.0,33.12099032335839),FVector(77.5,69.0,62.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(29.0,0.0,-16.0).GetSafeNormal()),FColor(0xBF,0xA4,0x4D),false},
{TEXT("Atrocity"),TEXT("Cone"),FVector(10.0,10.0,33.12099032335839),FVector(77.5,76.0,62.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(29.0,0.0,-16.0).GetSafeNormal()),FColor(0xBF,0xA4,0x4D),false},
{TEXT("Atrocity"),TEXT("Cone"),FVector(20.0,20.0,28.284271247461902),FVector(-35.0,0.0,180.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-20.0,0.0,20.0).GetSafeNormal()),FColor(0xBF,0xA4,0x4D),false},
{TEXT("Balork"),TEXT("Sphere"),FVector(100.0,120.0,140.0),FVector(0.0,0.0,165.0),FQuat::Identity,FColor(0x6B,0x25,0x22),false},
{TEXT("Balork"),TEXT("Sphere"),FVector(58.0,64.0,56.0),FVector(15.0,0.0,232.0),FQuat::Identity,FColor(0x6B,0x25,0x22),false},
{TEXT("Balork"),TEXT("Cone"),FVector(24.0,24.0,25.179356624028344),FVector(52.5,0.0,227.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(25.0,0.0,-3.0).GetSafeNormal()),FColor(0x29,0x1F,0x22),false},
{TEXT("Balork"),TEXT("Cylinder"),FVector(38.0,38.0,95.0),FVector(0.0,-35.0,67.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,95.0).GetSafeNormal()),FColor(0x6B,0x25,0x22),false},
{TEXT("Balork"),TEXT("Sphere"),FVector(70.0,45.0,30.0),FVector(18.0,-35.0,15.0),FQuat::Identity,FColor(0x29,0x1F,0x22),false},
{TEXT("Balork"),TEXT("Cone"),FVector(24.0,24.0,60.27437266367855),FVector(7.0,-29.5,281.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-10.0,-13.0,58.0).GetSafeNormal()),FColor(0xB7,0xA1,0x81),false},
{TEXT("Balork"),TEXT("Cube"),FVector(100.0,105.0,12.0),FVector(-35.0,-102.5,208.0),FQuat::Identity,FColor(0xA3,0x3A,0x29),false},
{TEXT("Balork"),TEXT("Cone"),FVector(80.0,80.0,75.0),FVector(-35.0,-182.5,208.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,-75.0,0.0).GetSafeNormal()),FColor(0xA3,0x3A,0x29),false},
{TEXT("Balork"),TEXT("Cube"),FVector(55.0,64.0,4.0),FVector(-26.0,-108.0,217.0),FQuat::Identity,FColor(0x29,0x1F,0x22),false},
{TEXT("Balork"),TEXT("Cylinder"),FVector(30.0,30.0,75.16648189186454),FVector(20.0,-59.5,168.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(40.0,-9.0,-63.0).GetSafeNormal()),FColor(0x6B,0x25,0x22),false},
{TEXT("Balork"),TEXT("Cylinder"),FVector(38.0,38.0,95.0),FVector(0.0,35.0,67.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,0.0,95.0).GetSafeNormal()),FColor(0x6B,0x25,0x22),false},
{TEXT("Balork"),TEXT("Sphere"),FVector(70.0,45.0,30.0),FVector(18.0,35.0,15.0),FQuat::Identity,FColor(0x29,0x1F,0x22),false},
{TEXT("Balork"),TEXT("Cone"),FVector(24.0,24.0,60.27437266367855),FVector(7.0,29.5,281.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(-10.0,13.0,58.0).GetSafeNormal()),FColor(0xB7,0xA1,0x81),false},
{TEXT("Balork"),TEXT("Cube"),FVector(100.0,105.0,12.0),FVector(-35.0,102.5,208.0),FQuat::Identity,FColor(0xA3,0x3A,0x29),false},
{TEXT("Balork"),TEXT("Cone"),FVector(80.0,80.0,75.0),FVector(-35.0,182.5,208.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,75.0,0.0).GetSafeNormal()),FColor(0xA3,0x3A,0x29),false},
{TEXT("Balork"),TEXT("Cube"),FVector(55.0,64.0,4.0),FVector(-26.0,108.0,217.0),FQuat::Identity,FColor(0x29,0x1F,0x22),false},
{TEXT("Balork"),TEXT("Cylinder"),FVector(30.0,30.0,75.16648189186454),FVector(20.0,59.5,168.5),FQuat::FindBetweenNormals(FVector::UpVector, FVector(40.0,9.0,-63.0).GetSafeNormal()),FColor(0x6B,0x25,0x22),false},
{TEXT("Balork"),TEXT("Cylinder"),FVector(10.0,10.0,400.0),FVector(40.0,0.0,130.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,400.0,0.0).GetSafeNormal()),FColor(0xB7,0xA1,0x81),true},
{TEXT("Balork"),TEXT("Cone"),FVector(36.0,36.0,45.0),FVector(40.0,187.5,130.0),FQuat::FindBetweenNormals(FVector::UpVector, FVector(0.0,45.0,0.0).GetSafeNormal()),FColor(0xA9,0xA3,0x9B),true},
};
}
bool LHEnemyPresentation::Known(FName Id)
{
    const FString S = Id.ToString();
    for (const auto& P : LHEnemyPresentationPrivate::Pieces) if (S == FString(TEXT("Presentation.Enemy.")) + P.Id) return true;
    return false;
}
void LHEnemyPresentation::Build(AActor* Owner, USceneComponent* Root, FName Id)
{
    auto* Body = NewObject<USceneComponent>(Owner);
    Owner->AddInstanceComponent(Body); Body->SetupAttachment(Root);
    Body->ComponentTags.Add(TEXT("LH.Presentation.BodyGroup")); Body->RegisterComponent();
    for (const auto& P : LHEnemyPresentationPrivate::Pieces)
    {
        if (Id.ToString() != FString(TEXT("Presentation.Enemy.")) + P.Id) continue;
        auto* Mesh = NewObject<UStaticMeshComponent>(Owner);
        Owner->AddInstanceComponent(Mesh);
        Mesh->SetupAttachment(P.Weapon ? Root : Body);
        Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, *(FString(TEXT("/Engine/BasicShapes/")) + P.Mesh + TEXT(".") + P.Mesh)));
        Mesh->SetRelativeLocation(P.Center);
        Mesh->SetRelativeRotation(P.Rotation);
        Mesh->SetRelativeScale3D(P.Size / 100.f);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetGenerateOverlapEvents(false);
        Mesh->SetCanEverAffectNavigation(false);
        if (P.Weapon) Mesh->ComponentTags.Add(TEXT("LH.Presentation.Weapon"));
        Mesh->RegisterComponent();
        if (auto* Material = Mesh->CreateDynamicMaterialInstance(0))
            Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(P.Color));
    }
}
void LHEnemyPresentation::Dead(USceneComponent* Root, FName Id)
{
    const FString S = Id.ToString();
    double Z = S.Contains(TEXT("Bat")) ? .15 : S.EndsWith(TEXT("GiantSpider")) ? .30 :
        S.Contains(TEXT("Goblin")) ? .30 : S.EndsWith(TEXT("Atrocity")) ? .25 : S.EndsWith(TEXT("Balork")) ? .20 : .35;
    // Bats are suspended; lower the complete assembly to floor before flattening.
    double Floor = 0;
    if (S.Contains(TEXT("Bat")))
    {
        Floor = TNumericLimits<double>::Max();
        for (const auto& P : LHEnemyPresentationPrivate::Pieces)
            if (S == FString(TEXT("Presentation.Enemy.")) + P.Id)
                Floor = FMath::Min(Floor, P.Center.Z - P.Size.Z * .5);
    }
    TArray<USceneComponent*> Children; Root->GetChildrenComponents(false, Children);
    for (auto* C : Children)
    {
        FVector Pos = C->GetRelativeLocation(), Scale = C->GetRelativeScale3D();
        if (C->ComponentHasTag(TEXT("LH.Presentation.Weapon")))
        {
            if (S.EndsWith(TEXT("Balork"))) Pos.Z = 18;
            else
            {
                const FQuat Rot(FVector::RightVector, PI / 2);
                const double OriginZ = S.EndsWith(TEXT("GoblinWarrior")) ? 115 : 105;
                Pos = Rot.RotateVector(Pos - FVector(Pos.X, Pos.Y, OriginZ)) + FVector(0,0,S.EndsWith(TEXT("GoblinWarrior")) ? 8 : 7);
                C->SetRelativeRotation(Rot * C->GetRelativeRotation().Quaternion());
            }
        }
        else if (C->ComponentHasTag(TEXT("LH.Presentation.BodyGroup"))) { Pos.Z = -Floor * Z; Scale.Z = Z; }
        C->SetRelativeLocation(Pos); C->SetRelativeScale3D(Scale);
    }
    Root->GetChildrenComponents(true, Children);
    for (auto* C : Children) if (auto* Mesh = Cast<UStaticMeshComponent>(C))
    {
        if (auto* Material = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0)))
        {
            FLinearColor Color;
            if (Material->GetVectorParameterValue(FMaterialParameterInfo(TEXT("Color")),Color))
            {
                Color = FMath::Lerp(Color,FLinearColor(FColor(0x33,0x33,0x33)),.65f);
                const float Grey = Color.GetLuminance();
                Color = FMath::Lerp(FLinearColor(Grey,Grey,Grey),Color,.7f);
                Material->SetVectorParameterValue(TEXT("Color"),Color);
            }
        }
    }
}
