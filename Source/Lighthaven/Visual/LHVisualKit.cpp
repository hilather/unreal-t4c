#include "Visual/LHVisualKit.h"
#include "Visual/LHB1ArtBinding.h"
#include "Visual/LHB1Lighting.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "ProceduralMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Misc/Crc.h"
#include "UObject/ConstructorHelpers.h"

namespace LHVisualPrivate
{
FLinearColor Hex(const TCHAR* H) { return FLinearColor(FColor::FromHex(H)); }
void Box(FLHVisualRecipe& R, FVector C, FVector S, int32 Surface=0, bool Solid=false, FRotator Rot=FRotator::ZeroRotator)
{
    FLHVisualBox B; B.Center=C; B.Size=S; B.Surface=Surface; B.Rotation=Rot;
    R.Geometry.Add(B); if (Solid) R.Collision.Add(B);
}
void Wall(FLHVisualRecipe& R, float Length)
{
    FLHVisualBox Solid; Solid.Center={Length/2,-10,200}; Solid.Size={Length,20,400}; R.Collision.Add(Solid);
    Box(R,{Length/2,-10,200},{Length,18,400});
    // Recessed courses fit INSIDE the collision strip, on both clear faces.
    for (int32 Row=0; Row<8; ++Row)
    {
        const float Offset=(Row%2)*50.f;
        for (float X=-Offset; X<Length; X+=100)
        {
            float A=FMath::Max(0.f,X)+2, B=FMath::Min(Length,X+100)-2;
            if (B>A)
            {
                int32 Surface=(Row+int32(X+100)/100)%3==0?1:0;
                if(R.Style==ELHVisualStyle::Church && Row>=4) Surface=1;
                if(R.Style==ELHVisualStyle::B2Damp) Surface=Row<2?1:0;
                if(R.Style==ELHVisualStyle::B3Crypt) Surface=(Row>=3 && Row<=5 && A<Length*.5f)?1:0;
                Box(R,{(A+B)/2,-10,Row*50.f+25},{B-A,20,46},Surface);
            }
        }
    }

}
void Frame(FLHVisualRecipe& R,float Width,float Height,float Bay,bool Arch)
{
    const float Side=(Bay-Width)/2;
    Box(R,{-(Width+Side)/2,-10,200},{Side,20,400},0,true);
    Box(R,{(Width+Side)/2,-10,200},{Side,20,400},0,true);
    Box(R,{0,-10,(Height+400)/2},{Width,20,400-Height},0,true);
    if(Arch) R.Geometry.Last().Size.Y=18;
    if (Arch)
    {
        // Six stepped voussoirs; underside never enters rectangular clear aperture.
        for (int32 I=0; I<6; ++I)
        {
            const float Z=Height+10+20*(1-FMath::Abs((I+.5f)/3-1));
            Box(R,{(I-2.5f)*Width/6,-10,Z},{Width/6-3,20,18},1);
        }
    }
}
void Stair(FLHVisualRecipe& R,float Run,float Rise)
{
    R.TriangleBudget=6000;
    const float Angle=FMath::RadiansToDegrees(FMath::Atan2(Rise,Run));
    const FRotator Rot(Angle,0,0);
    // Top face follows exact ramp; visual treads are independent, no step collision.
    const FVector N=Rot.RotateVector(FVector::UpVector);
    Box(R,FVector(Run/2,0,Rise/2)-N*10,{FMath::Sqrt(Run*Run+Rise*Rise),300,20},0,true,Rot);
    const int32 Count=FMath::RoundToInt(FMath::Abs(Rise)/10);
    for(int32 I=0; I<Count; ++I)
        Box(R,{(I+.5f)*Run/Count,0,(I+.5f)*Rise/Count},{Run/Count-2,300,4},1);
}
// Outward winding: each face has distinct normals and UVs; one section per surface.
void AppendBox(const FLHVisualBox& B,TArray<FVector>& V,TArray<int32>& Ind,TArray<FVector>& N,TArray<FVector2D>& UV)
{
    const FVector H=B.Size/2;
    const FVector P[8]={{-H.X,-H.Y,-H.Z},{H.X,-H.Y,-H.Z},{H.X,H.Y,-H.Z},{-H.X,H.Y,-H.Z},
        {-H.X,-H.Y,H.Z},{H.X,-H.Y,H.Z},{H.X,H.Y,H.Z},{-H.X,H.Y,H.Z}};
    const int32 Faces[6][4]={{0,3,2,1},{4,5,6,7},{0,1,5,4},{3,7,6,2},{0,4,7,3},{1,2,6,5}};
    for (const auto& F:Faces)
    {
        const int32 Base=V.Num();
        FVector Normal=FVector::CrossProduct(P[F[1]]-P[F[0]],P[F[2]]-P[F[0]]).GetSafeNormal();
        const float U=(P[F[1]]-P[F[0]]).Length()/100, W=(P[F[3]]-P[F[0]]).Length()/100;
        for(int32 J=0;J<4;++J) { V.Add(B.Center+B.Rotation.RotateVector(P[F[J]])); N.Add(B.Rotation.RotateVector(Normal)); }
        UV.Append({{0,0},{U,0},{U,W},{0,W}});
        // Unreal front faces use clockwise winding viewed from outside.
        Ind.Append({Base,Base+2,Base+1,Base,Base+3,Base+2});
    }
}
}

uint32 FLHVisualRecipe::Fingerprint() const
{
    // Explicit fields, no struct padding, actor GUIDs, pointer addresses or global RNG.
    FString S=Id.ToString()+FString::FromInt(int32(Style));
    for(const FLinearColor& C:Colors) S+=C.ToString();
    for(float F:Roughness) S+=FString::Printf(TEXT("%.9g"),F);
    for(const auto* List:{&Geometry,&Collision})
    {
        S+=TEXT("|");
        for(const auto& B:*List) S+=B.Center.ToString()+B.Size.ToString()+B.Rotation.ToString()+FString::FromInt(B.Surface);
    }
    S+=FString::FromInt(TriangleBudget)+TEXT("/")+FString::FromInt(DrawBudget);
    return FCrc::StrCrc32(*S);
}

const TArray<FName>& LHVisual::PieceIds()
{
    static const TArray<FName> Ids={
        TEXT("Presentation.Environment.Shared.Wall100"),TEXT("Presentation.Environment.Shared.Wall200"),TEXT("Presentation.Environment.Shared.Wall400"),
        TEXT("Presentation.Environment.Shared.Floor100"),TEXT("Presentation.Environment.Shared.Floor200"),TEXT("Presentation.Environment.Shared.Floor400"),
        TEXT("Presentation.Environment.Shared.Door240"),TEXT("Presentation.Environment.Shared.Door320"),TEXT("Presentation.Environment.Basement.DoorBoss500"),
        TEXT("Presentation.Environment.Shared.Arch240"),TEXT("Presentation.Environment.Shared.Arch320"),TEXT("Presentation.Environment.Basement.ArchBoss500"),
        TEXT("Presentation.Environment.Shared.Stair400x100"),TEXT("Presentation.Environment.Shared.Stair600x120"),TEXT("Presentation.Environment.Shared.Stair250x80"),TEXT("Presentation.Environment.Shared.Stair300x80"),
        TEXT("Presentation.Environment.Church.Pillar"),TEXT("Presentation.Environment.Church.DoorLeafPreview"),
        TEXT("Presentation.Environment.Shared.Torch"),TEXT("Presentation.Environment.Shared.Sconce"),TEXT("Presentation.Environment.Shared.Barrel"),
        TEXT("Presentation.Environment.Shared.Crate"),TEXT("Presentation.Environment.Shared.Table"),TEXT("Presentation.Environment.Shared.Bench"),
        TEXT("Presentation.Environment.Shared.Altar"),TEXT("Presentation.Environment.Shared.Debris")};
    return Ids;
}
bool LHVisual::MakeRecipe(FName Id,ELHVisualStyle Style,FLHVisualRecipe& Out,bool bDescending)
{
    if (!PieceIds().Contains(Id) || uint8(Style)>uint8(ELHVisualStyle::B4Ritual)) return false;
    using namespace LHVisualPrivate;
    FLHVisualRecipe R; R.Id=Id; R.Style=Style;
    R.Colors[0]=Hex(Style==ELHVisualStyle::B4Ritual?TEXT("A69C89"):TEXT("78634B"));
    R.Colors[1]=Hex(Style==ELHVisualStyle::B2Damp?TEXT("433E32"):Style==ELHVisualStyle::B4Ritual?TEXT("B78370"):TEXT("9B9484"));
    if(Style==ELHVisualStyle::Church) R.Colors[1]=Hex(TEXT("B2A58A"));
    const FString S=Id.ToString();
    if(bDescending && !S.Contains(TEXT(".Stair"))) return false;
    const float Sign=bDescending?-1.f:1.f;
    if(S.Contains(TEXT(".Wall"))) Wall(R,S.EndsWith(TEXT("100"))?100:S.EndsWith(TEXT("200"))?200:400);
    else if(S.Contains(TEXT(".Floor")))
    {
        float L=S.EndsWith(TEXT("100"))?100:S.EndsWith(TEXT("200"))?200:400;
        R.Colors[0]=Hex(Style==ELHVisualStyle::Church?TEXT("77766D"):Style==ELHVisualStyle::B4Ritual?TEXT("A69C89"):TEXT("695640"));
        FLHVisualBox Solid; Solid.Center={L/2,L/2,-10}; Solid.Size={L,L,20}; R.Collision.Add(Solid);
        Box(R,{L/2,L/2,-11},{L,L,18});
        for(float X=0;X<L;X+=100) for(float Y=0;Y<L;Y+=100)
            Box(R,{X+50,Y+50,-1},{96,96,2},((int32(X+Y)/100)%4==0)?1:0);
    }
    else if(S.Contains(TEXT(".Stair")))
    {
        if(S.EndsWith(TEXT("400x100"))) Stair(R,400,Sign*100);
        else if(S.EndsWith(TEXT("600x120"))) Stair(R,600,Sign*120);
        else if(S.EndsWith(TEXT("250x80"))) Stair(R,250,Sign*80);
        else Stair(R,300,Sign*80);
    }
    else if(S.EndsWith(TEXT("DoorLeafPreview")))
    {
        R.Colors[0]=Hex(TEXT("43352A")); R.Colors[1]=Hex(TEXT("535451")); R.Roughness[0]=.8f; R.Roughness[1]=.65f;
        Box(R,{80,0,150},{160,8,300},0,true); R.Geometry.Last().Size.Y=6;
        for(int32 I=0;I<5;++I) Box(R,{I*32.f+16,0,150},{29,8,295});
        Box(R,{80,0,60},{158,10,10},1); Box(R,{80,0,240},{158,10,10},1);
    }
    else if(S.Contains(TEXT(".Door")) || S.Contains(TEXT(".Arch")))
    {
        R.TriangleBudget=4000;
        bool Boss=S.Contains(TEXT("Boss")); float W=Boss?500:S.EndsWith(TEXT("240"))?240:320;
        Frame(R,W,Boss?360:300,Boss?600:400,S.Contains(TEXT(".Arch")));
    }
    else if(S.EndsWith(TEXT("Pillar")))
    {
        R.TriangleBudget=4000; Box(R,{0,0,200},{60,60,400},0,true); R.Geometry.Last().Size={54,54,400};
        Box(R,{0,0,15},{60,60,30},1); Box(R,{0,0,385},{60,60,30},1);
    }
    else if(S.EndsWith(TEXT("Torch")) || S.EndsWith(TEXT("Sconce")))
    {
        R.Colors[0]=Hex(TEXT("535451")); R.Colors[1]=Hex(TEXT("E8AA53")); R.Roughness[0]=.65f;
        Box(R,{0,-3,0},{25,6,40}); Box(R,{0,12,-8},{8,30,8});
        Box(R,{0,25,12},{18,18,28},1); // broad warm flame proxy, no light actor
    }
    else if(S.EndsWith(TEXT("Debris")))
    {
        for(int32 I=0;I<5;++I) Box(R,{I*18.f-36,float((I%2)*20-10),6},{16,24,12},I%2,false,FRotator(0,I*27,0));
    }
    else if(S.EndsWith(TEXT("Altar")))
    {
        R.TriangleBudget=4000; Box(R,{0,0,20},{200,160,40},0,true);
        Box(R,{0,0,75},{160,120,70},0,true); Box(R,{0,0,115},{180,140,10},1,true);
    }
    else
    {
        R.Colors[0]=Hex(TEXT("43352A")); R.Colors[1]=Hex(TEXT("535451")); R.Roughness[0]=.8f; R.Roughness[1]=.65f;
        if(S.EndsWith(TEXT("Crate")))
        {
            Box(R,{0,0,40},{80,80,80},0,true);
            Box(R,{0,0,12},{82,82,10},1); Box(R,{0,0,68},{82,82,10},1);
        }
        else if(S.EndsWith(TEXT("Barrel")))
        {
            // Octagonal silhouette from intersecting boxes, one conservative footprint blocker.
            for(int32 I=0;I<4;++I) Box(R,{0,0,50},{60,25,100},0,false,FRotator(0,I*45,0));
            FLHVisualBox B; B.Center={0,0,50}; B.Size={60,60,100}; R.Collision.Add(B);
            Box(R,{0,0,20},{62,62,8},1); Box(R,{0,0,80},{62,62,8},1);
        }
        else
        {
            const bool Bench=S.EndsWith(TEXT("Bench")); float W=Bench?45:90,H=Bench?45:80;
            R.TriangleBudget=4000;
            Box(R,{0,0,H-5},{160,W,10},0,true);
            for(float X:{-60.f,60.f}) for(float Y:{-W/2+8,W/2-8}) Box(R,{X,Y,(H-10)/2},{12,12,H-10},0,true);
        }
    }
    Out=MoveTemp(R); return true;
}

ALHVisualPiece::ALHVisualPiece()
{
    PrimaryActorTick.bCanEverTick=false;
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Parent(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    MaterialParent=Parent.Object;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Placement")));
    Mesh=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Visual"), true);
    Mesh->SetFlags(RF_Transient);
    Mesh->SetupAttachment(GetRootComponent());
    Mesh->SetCollisionProfileName(TEXT("NoCollision")); Mesh->SetGenerateOverlapEvents(false); Mesh->SetCanEverAffectNavigation(false);
    Mesh->bUseComplexAsSimpleCollision=false;
    ImportedMesh=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("B1Art"),true);
    ImportedMesh->SetFlags(RF_Transient);
    ImportedMesh->SetupAttachment(GetRootComponent());
    ImportedMesh->SetCollisionProfileName(TEXT("NoCollision"));
    ImportedMesh->SetGenerateOverlapEvents(false); ImportedMesh->SetCanEverAffectNavigation(false);
    ArtSupport=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("B1SolidSupport"),true);
    ArtSupport->SetFlags(RF_Transient); ArtSupport->SetupAttachment(GetRootComponent());
    ArtSupport->SetCollisionProfileName(TEXT("NoCollision")); ArtSupport->SetGenerateOverlapEvents(false);
    ArtSupport->SetCanEverAffectNavigation(false); ArtSupport->bUseComplexAsSimpleCollision=false;
    for(const auto& Name:LHB1Art::AssetNames())
    {
        const FString Path=TEXT("/Game/Lighthaven/Art/Env/B1/SM_")+Name+TEXT(".SM_")+Name;
        B1Assets.Add(LoadObject<UStaticMesh>(nullptr,*Path,nullptr,LOAD_NoWarn|LOAD_Quiet));
    }
}
void ALHVisualPiece::PostLoad()
{
    Super::PostLoad();
    if(!BuiltRecipe.Geometry.IsEmpty()) { const FLHVisualRecipe Copy=BuiltRecipe; Build(Copy); }
}
void ALHVisualPiece::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if(!BuiltRecipe.Geometry.IsEmpty()) { const FLHVisualRecipe Copy=BuiltRecipe; Build(Copy); }
}
void ALHVisualPiece::BeginPlay()
{
    Super::BeginPlay();
    // Regenerate dynamic materials and simple collision from serialized source at runtime.
    if(!BuiltRecipe.Geometry.IsEmpty()) { const FLHVisualRecipe Copy=BuiltRecipe; Build(Copy); }
}
bool ALHVisualPiece::Build(const FLHVisualRecipe& R)
{
    if(R.Geometry.IsEmpty() || R.Geometry.Num()*12>R.TriangleBudget) return false;
    for(const auto* List:{&R.Geometry,&R.Collision}) for(const auto& B:*List)
        if(B.Center.ContainsNaN() || B.Size.ContainsNaN() || B.Rotation.ContainsNaN() || B.Size.GetMin()<=0 || B.Surface<0 || B.Surface>1) return false;
    auto* Parent=MaterialParent.Get();
    if(!Parent) return false;
    int32 Sections=0; for(int32 S=0;S<2;++S) if(R.Geometry.ContainsByPredicate([S](const FLHVisualBox& B){ return B.Surface==S; })) ++Sections;
    if(Sections>R.DrawBudget) return false;
    for(int32 S=0;S<2;++S) if(!FMath::IsFinite(R.Roughness[S]) || R.Roughness[S]<0 || R.Roughness[S]>1 || (!FMath::IsFinite(R.Colors[S].R) || !FMath::IsFinite(R.Colors[S].G) || !FMath::IsFinite(R.Colors[S].B) || !FMath::IsFinite(R.Colors[S].A))) return false;
    for(const auto& B:Blockers) B->DestroyComponent(); Blockers.Reset(); Mesh->ClearAllMeshSections();
    for(int32 Surface=0;Surface<2;++Surface)
    {
        TArray<FVector> V,N; TArray<int32> Ind; TArray<FVector2D> UV;
        for(const auto& B:R.Geometry) if(B.Surface==Surface) LHVisualPrivate::AppendBox(B,V,Ind,N,UV);
        if(V.IsEmpty()) continue;
        Mesh->CreateMeshSection(Surface,V,Ind,N,UV,TArray<FColor>(),TArray<FProcMeshTangent>(),false);
        auto* M=UMaterialInstanceDynamic::Create(Parent,this);
        M->SetVectorParameterValue(TEXT("Color"),R.Colors[Surface]);
        M->SetScalarParameterValue(TEXT("Roughness"),R.Roughness[Surface]); Mesh->SetMaterial(Surface,M);
    }
    for(const auto& B:R.Collision)
    {
        auto* C=NewObject<UBoxComponent>(this); AddInstanceComponent(C); C->SetupAttachment(Mesh);
        C->SetBoxExtent(B.Size/2); C->SetRelativeLocationAndRotation(B.Center,B.Rotation);
        C->SetCollisionProfileName(TEXT("BlockAll")); C->SetGenerateOverlapEvents(false); C->SetCanEverAffectNavigation(true); C->RegisterComponent(); Blockers.Add(C);
    }
    ImportedMesh->ClearInstances();
    ImportedMesh->SetStaticMesh(nullptr); ImportedMesh->SetVisibility(false);
    Mesh->SetVisibility(true);
    LHB1Art::FFit Fit;
    if(LHB1Art::Resolve(R,Fit))
    {
        const int32 Index=LHB1Art::AssetNames().IndexOfByKey(Fit.AssetName);
        if(B1Assets.IsValidIndex(Index) && B1Assets[Index])
        {
            ImportedMesh->SetStaticMesh(B1Assets[Index]);
            ImportedMesh->SetRelativeTransform(FTransform::Identity);
            for(const auto& Instance:Fit.Instances) ImportedMesh->AddInstance(Instance);
            for(int32 Slot=0;Slot<ImportedMesh->GetStaticMesh()->GetStaticMaterials().Num();++Slot)
                ImportedMesh->SetMaterial(Slot,UMaterialInstanceDynamic::Create(ImportedMesh->GetStaticMesh()->GetMaterial(Slot),this));
            UpdateArtClip();
            ImportedMesh->SetVisibility(true); Mesh->SetVisibility(false);
        }
    }
    BuiltRecipe=R; UpdateArtClip(); UpdateArtSupport(R); ULHB1TorchLightComponent::Configure(this); return true;
}
ALHVisualPiece* LHVisual::SpawnProp(UWorld* World,FName Id,const FTransform& T,ELHVisualStyle Style,bool bDescending)
{
    FLHVisualRecipe R;
    if(!World || !T.IsValid() || !T.GetScale3D().Equals(FVector::OneVector) || !MakeRecipe(Id,Style,R,bDescending)) return nullptr;
    auto* A=World->SpawnActor<ALHVisualPiece>(ALHVisualPiece::StaticClass(),T);
    if(A && !A->Build(R)) { A->Destroy(); return nullptr; } return A;
}
TArray<ALHVisualPiece*> LHVisual::SpawnWallRun(UWorld* World,FVector Start,FVector End,ELHVisualStyle Style)
{
    TArray<ALHVisualPiece*> Out;
    if(!World || Start.ContainsNaN() || End.ContainsNaN() || !FMath::IsNearlyEqual(Start.Z,End.Z)) return Out;
    FVector D=End-Start; float Length=D.Size();
    if(Length<5 || Length>100000 || uint8(Style)>4) return Out;
    FRotator Rot(0,FMath::RadiansToDegrees(FMath::Atan2(D.Y,D.X)),0); D/=Length;
    for(float X=0;X<Length;X+=400)
    {
        FLHVisualRecipe R; MakeRecipe(TEXT("Presentation.Environment.Shared.Wall400"),Style,R);
        R.Geometry.Reset(); R.Collision.Reset(); LHVisualPrivate::Wall(R,FMath::Min(400.f,Length-X));
        auto* A=World->SpawnActor<ALHVisualPiece>(ALHVisualPiece::StaticClass(),FTransform(Rot,Start+D*X));
        if(!A || !A->Build(R)) { if(A) A->Destroy(); for(auto* P:Out) P->Destroy(); Out.Reset(); return Out; }
        Out.Add(A);
    }
    return Out;
}
FLHVisualTotals LHVisual::ValidatePlacedSet(const TArray<ALHVisualPiece*>& Pieces)
{
    FLHVisualTotals T; TSet<ALHVisualPiece*> Seen;
    for(auto* P:Pieces)
    {
        if(!IsValid(P)) { T.Errors.Add(TEXT("Null/destroyed piece")); continue; }
        if(Seen.Contains(P)) { T.Errors.Add(TEXT("Duplicate piece")); continue; } Seen.Add(P);
        ++T.Pieces; auto* M=P->GetMesh(); int32 Tri=0,Draw=0;
        for(int32 I=0;I<M->GetNumSections();++I) if(const auto* S=M->GetProcMeshSection(I))
            if(!S->ProcIndexBuffer.IsEmpty()) { Tri+=S->ProcIndexBuffer.Num()/3; ++Draw; }
        auto* Imported=P->GetImportedMesh();
        int32 VisibleTri=Tri, VisibleDraw=Draw;
        if(Imported->IsVisible() && Imported->GetStaticMesh())
        {
            const auto* Data=Imported->GetStaticMesh()->GetRenderData();
            if(Data && !Data->LODResources.IsEmpty())
            {
                VisibleTri=Data->LODResources[0].GetNumTriangles()*Imported->GetInstanceCount();
                VisibleDraw=Data->LODResources[0].Sections.Num();
                if(Data->LODResources[0].GetNumTriangles()>6000 || VisibleDraw>3) T.Errors.Add(P->GetRecipe().Id.ToString()+TEXT(": imported budget"));
            }
            else T.Errors.Add(TEXT("Imported mesh has no render data"));
            if(Imported->GetCollisionEnabled()!=ECollisionEnabled::NoCollision || Imported->CanEverAffectNavigation())
                T.Errors.Add(TEXT("Imported dressing collision/nav"));
        }
        if(const auto* Support=P->GetArtSupport()->GetProcMeshSection(0))
        {
            VisibleTri+=Support->ProcIndexBuffer.Num()/3;
            if(!Support->ProcIndexBuffer.IsEmpty()) ++VisibleDraw;
        }
        if(P->GetArtSupport()->GetCollisionEnabled()!=ECollisionEnabled::NoCollision || P->GetArtSupport()->CanEverAffectNavigation())
            T.Errors.Add(TEXT("B1 support collision/nav"));
        T.Triangles+=VisibleTri; T.DrawCalls+=VisibleDraw; T.CollisionBoxes+=P->GetBlockers().Num();
        const auto& R=P->GetRecipe();
        if(R.Geometry.IsEmpty() || !P->GetActorScale3D().Equals(FVector::OneVector)) T.Errors.Add(R.Id.ToString()+TEXT(": empty recipe/nonunit scale"));
        if(Tri>R.TriangleBudget || Draw>R.DrawBudget || Tri!=R.Geometry.Num()*12)
            T.Errors.Add(R.Id.ToString()+TEXT(": geometry/budget mismatch"));
        if(M->GetCollisionEnabled()!=ECollisionEnabled::NoCollision || M->CanEverAffectNavigation()) T.Errors.Add(R.Id.ToString()+TEXT(": render collision"));
        if(P->GetBlockers().Num()!=R.Collision.Num()) T.Errors.Add(R.Id.ToString()+TEXT(": blocker count"));
        for(int32 I=0;I<P->GetBlockers().Num();++I)
        {
            const auto* B=P->GetBlockers()[I].Get();
            if(!IsValid(B) || B->GetCollisionProfileName()!=TEXT("BlockAll") || B->GetCollisionResponseToChannel(ECC_Pawn)!=ECR_Block ||
                B->GetCollisionResponseToChannel(ECC_Visibility)!=ECR_Block || !B->CanEverAffectNavigation())
                T.Errors.Add(R.Id.ToString()+TEXT(": blocker profile"));
            if(IsValid(B) && R.Collision.IsValidIndex(I) &&
                (!B->GetUnscaledBoxExtent().Equals(R.Collision[I].Size/2) || !B->GetRelativeLocation().Equals(R.Collision[I].Center) || !B->GetRelativeRotation().Equals(R.Collision[I].Rotation)))
                T.Errors.Add(R.Id.ToString()+TEXT(": blocker dimensions"));
        }
    }
    return T;
}

void ALHVisualPiece::UpdateArtClip()
{
    LHB1Art::FFit Fit;
    if(!LHB1Art::Resolve(BuiltRecipe,Fit) || !ImportedMesh->GetStaticMesh()) return;
    const FTransform Actor=GetActorTransform();
    const FVector Center=Actor.TransformPosition(Fit.ClipBounds.GetCenter());
    const FVector Extent=Fit.ClipBounds.GetExtent()*Actor.GetScale3D().GetAbs();
    for(int32 Slot=0;Slot<ImportedMesh->GetNumMaterials();++Slot)
    {
        auto* M=Cast<UMaterialInstanceDynamic>(ImportedMesh->GetMaterial(Slot));
        if(!M) continue;
        M->SetVectorParameterValue(TEXT("ClipOrigin"),FLinearColor(Center.X,Center.Y,Center.Z,0));
        M->SetVectorParameterValue(TEXT("ClipExtent"),FLinearColor(Extent.X,Extent.Y,Extent.Z,0));
        for(const auto& Pair:TArray<TPair<FName,FVector>>{{TEXT("ClipX"),FVector::ForwardVector},{TEXT("ClipY"),FVector::RightVector},{TEXT("ClipZ"),FVector::UpVector}})
        {
            const FVector Axis=Actor.TransformVectorNoScale(Pair.Value);
            M->SetVectorParameterValue(Pair.Key,FLinearColor(Axis.X,Axis.Y,Axis.Z,0));
        }
    }
}

void ALHVisualPiece::UpdateArtSupport(const FLHVisualRecipe& R)
{
    ArtSupport->ClearAllMeshSections();
    if(R.Style!=ELHVisualStyle::B1Cellar) return;
    TArray<FLHVisualBox> Boxes;
    UMaterialInterface* Parent=MaterialParent;
    LHB1Art::FFit Fit;
    if(ImportedMesh->IsVisible() && LHB1Art::Resolve(R,Fit))
    {
        Boxes=LHB1Art::SolidBacking(R,Fit);
        // Separate MID: crop shader bounds default to effectively infinite on backing.
        if(!Boxes.IsEmpty()) Parent=ImportedMesh->GetStaticMesh()->GetMaterial(0);
    }
    const FString Id=R.Id.ToString();
    if(Id.EndsWith(TEXT("Sconce")) || Id.EndsWith(TEXT("Torch")))
    {
        const FTransform T=GetActorTransform();
        FCollisionQueryParams Query(SCENE_QUERY_STAT(LHB1FixtureSupport),false,this);
        FHitResult FloorHit;
        // All B1 fixtures receive a stand: retained full-height colliders cannot
        // prove the cropped presentation wall still exists behind a plate.
        bool FoundFloor=false;
        // Prototype bounded retry: a hidden wall overlapping the origin is not ground.
        for(int32 Attempt=0;GetWorld() && Attempt<8;++Attempt)
        {
            // Pawns/enemies and dropped items must never become the stand's ground.
            if(!GetWorld()->LineTraceSingleByObjectType(FloorHit,T.TransformPosition(FVector(0,25,-30)),
                T.TransformPosition(FVector(0,25,-1030)),FCollisionObjectQueryParams(ECC_WorldStatic),Query)) break;
            if(!FloorHit.bStartPenetrating && FloorHit.ImpactNormal.Z>=.5f) { FoundFloor=true; break; }
            if(!FloorHit.GetActor()) break;
            Query.AddIgnoredActor(FloorHit.GetActor());
        }
        if(FoundFloor)
        {
            const double Base=T.InverseTransformPosition(FloorHit.ImpactPoint).Z;
            if(Base<-20)
            {
                FLHVisualBox Stem; Stem.Center={0,25,(Base-8)/2}; Stem.Size={9,9,-8-Base}; Boxes.Add(Stem);
                FLHVisualBox Foot; Foot.Center={0,25,Base+3}; Foot.Size={38,38,6}; Boxes.Add(Foot);
            }
        }
    }
    if(Boxes.IsEmpty() || !Parent) return;
    TArray<FVector> V,N; TArray<int32> Ind; TArray<FVector2D> UV;
    for(const auto& B:Boxes) LHVisualPrivate::AppendBox(B,V,Ind,N,UV);
    TArray<FProcMeshTangent> Tangents;
    for(int32 Base=0;Base<V.Num();Base+=4)
    {
        const FVector U=(V[Base+1]-V[Base]).GetSafeNormal();
        const FVector W=(V[Base+3]-V[Base]).GetSafeNormal();
        const bool Flip=FVector::DotProduct(FVector::CrossProduct(N[Base],U),W)<0;
        for(int32 Corner=0;Corner<4;++Corner) Tangents.Add(FProcMeshTangent(U,Flip));
    }
    ArtSupport->CreateMeshSection(0,V,Ind,N,UV,TArray<FColor>(),Tangents,false);
    auto* M=UMaterialInstanceDynamic::Create(Parent,this);
    M->SetVectorParameterValue(TEXT("Color"),R.Colors[0]); M->SetScalarParameterValue(TEXT("Roughness"),.9f);
    M->SetVectorParameterValue(TEXT("ClipOrigin"),FLinearColor(0,0,0,0));
    M->SetVectorParameterValue(TEXT("ClipExtent"),FLinearColor(1.e8,1.e8,1.e8,0));
    ArtSupport->SetMaterial(0,M);
}
