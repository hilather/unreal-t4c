#include "Misc/AutomationTest.h"
#include "Visual/LHVisualKit.h"
#include "Visual/LHEnvArtBinding.h"
#include "Visual/LHFloorLighting.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "World/LHAreaRegistry.h"
#include "World/LHWorldMarkers.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/Package.h"

namespace LHChurchSurroundingsTestsPrivate
{
bool OverlapXY(const FBox& A,const FBox& B)
{
    return A.Min.X<B.Max.X-.01 && A.Max.X>B.Min.X+.01 && A.Min.Y<B.Max.Y-.01 && A.Max.Y>B.Min.Y+.01;
}
// The same continuous camera shadow contract as authoring, independently
// evaluated over every original floor/ramp envelope; no center-ray shortcuts.
bool ClearSight(const FBox& Visual,const TArray<FBox>& Floors)
{
    const FVector Boom=-FRotator(-55,45,0).Vector()*1200;
    for(const FBox& Floor:Floors)
    {
        const double Far=FMath::Max(0.,Visual.Max.Z-Floor.Min.Z)*(-Boom.X/(Boom.Z-90.));
        const FBox Shadow(FVector(Visual.Min.X,Visual.Min.Y,Floor.Min.Z),
            FVector(Visual.Max.X+Far,Visual.Max.Y+Far,Floor.Max.Z));
        if(Visual.Max.Z>Floor.Min.Z && OverlapXY(Shadow,Floor)) return false;
    }
    return true;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHChurchSurroundingsTest,"Lighthaven.World.Dressing.ChurchSurroundings",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHChurchSurroundingsTest::RunTest(const FString& Parameters)
{
    using namespace LHChurchSurroundingsTestsPrivate;
    const auto& Area=LHWorld::Registry()[0];
    auto* Package=LoadPackage(nullptr,*Area.Map.GetLongPackageName(),LOAD_None);
    auto* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
    if(!TestNotNull(TEXT("Regenerated Church surroundings map"),World)) return false;
    TArray<FBox> Floors;
    TArray<ALHVisualPiece*> Added;
    int32 Ramps=0,Aprons=0,Outlines=0,MainAprons=0,MageAprons=0,Entrances=0,NPCs=0,Portals=0,ExteriorMoons=0;
    for(TActorIterator<AActor> It(World);It;++It)
    {
        if(auto* Moon=Cast<ADirectionalLight>(*It)) if(Moon->Tags.Contains(TEXT("LH.Church.ExteriorMoon")))
        {
            ++ExteriorMoons;
            const auto* Light=CastChecked<UDirectionalLightComponent>(Moon->GetLightComponent());
            TestEqual(TEXT("Exterior moon is movable"),Light->Mobility,EComponentMobility::Movable);
            TestEqual(TEXT("Exterior moon remains four lux"),Light->Intensity,4.f);
            TestTrue(TEXT("Exterior moon keeps its cool linear palette"),Light->GetLightColor().Equals(FLinearColor(.35f,.52f,1.f),.01f));
            TestTrue(TEXT("Exterior moon affects only exterior receivers"),
                !Light->LightingChannels.bChannel0 && Light->LightingChannels.bChannel1 && !Light->LightingChannels.bChannel2);
            TestFalse(TEXT("Exterior moon is shadowless"),Light->CastShadows);
            TestFalse(TEXT("Exterior moon cannot affect navigation"),Light->CanEverAffectNavigation());
            TestFalse(TEXT("Exterior moon is not a room fill"),Moon->Tags.Contains(TEXT("LH.Lighting.RoomFill")));
            TestEqual(TEXT("Exterior moon has no indirect contribution"),Light->IndirectLightingIntensity,0.f);
            TestEqual(TEXT("Exterior moon has no volumetric contribution"),Light->VolumetricScatteringIntensity,0.f);
            TestFalse(TEXT("Exterior moon cannot replace the atmospheric sun"),bool(Light->bAtmosphereSunLight));
        }
        if(auto* A=Cast<AStaticMeshActor>(*It))
        {
            const auto* C=A->GetStaticMeshComponent();
            const FString Label=A->GetActorLabel();
            const FVector Size=A->GetActorScale3D().GetAbs()*100;
            if(C->GetStaticMesh() && C->GetStaticMesh()->GetName()==TEXT("Cube") && Size.Z<=20 && Size.X>=5 && Size.Y>=5
                && !A->IsHidden() && (C->IsVisible() || A->Tags.Contains(TEXT("LH.Dressing.RetainedCollider")))
                && !Label.StartsWith(TEXT("Scenic.")) && !Label.StartsWith(TEXT("NPC.")) && !Label.Contains(TEXT("Arrow"))
                && !Label.Contains(TEXT("Tread")) && !Label.Contains(TEXT("RedAisle")))
            {
                Floors.Add(C->GetStaticMesh()->GetBoundingBox().TransformBy(C->GetComponentTransform()));
                Ramps+=Label==TEXT("Temple.Descent.Ramp")?1:0;
                TestTrue(TEXT("Original floor collider still blocks gameplay"),C->GetCollisionEnabled()!=ECollisionEnabled::NoCollision);
            }
        }
        if(auto* E=Cast<ALHEntranceMarker>(*It))
        {
            ++Entrances;
            const auto* Definition=Area.Entrances.FindByPredicate([&](const FLHEntranceDefinition& D){return LHWorld::SameEntrance(D.Id,E->EntranceId);});
            if(TestNotNull(TEXT("Church entrance remains in registry"),Definition))
                TestTrue(TEXT("Surroundings preserve exact reviewed arrival transform"),E->SafeArrivalTransform.Equals(Definition->SafeTransform,.001));
        }
        if(auto* N=Cast<ALHInteractableMarker>(*It)) NPCs+=N->DefinitionId.Value.ToString().StartsWith(TEXT("NPC."))?1:0;
        if(It->IsA<ALHPortal>()) ++Portals;
        if(auto* Piece=Cast<ALHVisualPiece>(*It)) if(Piece->Tags.Contains(TEXT("LH.Church.Surroundings"))) Added.Add(Piece);
    }
    TestTrue(TEXT("All authored Church walkable envelopes retained"),Floors.Num()>20);
    TestEqual(TEXT("Descending wing ramp is included in exclusion audit"),Ramps,1);
    TestEqual(TEXT("Church entrance count preserved"),Entrances,2);
    TestEqual(TEXT("Church NPC count preserved"),NPCs,13);
    TestEqual(TEXT("Church portal count preserved"),Portals,1);
    TestEqual(TEXT("One dedicated exterior moon is present"),ExteriorMoons,1);
    for(auto* Piece:Added)
    {
        const bool Apron=Piece->Tags.Contains(TEXT("LH.Church.Apron"));
        const bool Outline=Piece->Tags.Contains(TEXT("LH.Church.Outline"));
        TestTrue(TEXT("Surroundings have exactly one purpose tag"),Apron!=Outline);
        const auto& Recipe=Piece->GetRecipe();
        TestEqual(TEXT("Surroundings use existing Church kit"),Recipe.Style,ELHVisualStyle::Church);
        TestEqual(TEXT("Only existing floor and wall assets are used"),Recipe.Id,
            FName(Apron?TEXT("Presentation.Environment.Shared.Floor400"):TEXT("Presentation.Environment.Shared.Wall400")));
        TestTrue(TEXT("Saved exterior palette marker survives map reload"),Recipe.Colors[0].Equals(FLinearColor(.061f,.081f,.11f,1.f),.00001f));
        TestEqual(TEXT("No new gameplay collision recipe"),Recipe.Collision.Num(),0);
        TestEqual(TEXT("No new blocking components"),Piece->GetBlockers().Num(),0);
        TArray<UPrimitiveComponent*> Components; Piece->GetComponents(Components);
        for(const auto* C:Components)
        {
            TestEqual(TEXT("Every surroundings surface has collision disabled"),C->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
            TestFalse(TEXT("Every surroundings surface cannot affect navigation"),C->CanEverAffectNavigation());
            TestTrue(TEXT("Exterior receivers exclude warm channel-zero interior lights"),
                !C->LightingChannels.bChannel0 && C->LightingChannels.bChannel1 && !C->LightingChannels.bChannel2);
        }
        LHEnvArt::FFit Fit;
        if(!TestTrue(TEXT("Surroundings fit an existing imported kit mesh"),LHEnvArt::Resolve(Recipe,Fit))) continue;
        TestTrue(TEXT("Every assembly stays within imported instance fit cap"),Fit.Instances.Num()>0 && Fit.Instances.Num()<=256);
        const auto* Imported=Piece->GetImportedMesh();
        TestTrue(TEXT("Imported Church mesh visible instead of procedural fallback"),Imported && Imported->IsVisible() && Imported->GetStaticMesh());
        if(Imported)
        {
            TestEqual(TEXT("Imported assembly count matches fit"),Imported->GetInstanceCount(),Fit.Instances.Num());
            for(int32 Slot=0;Slot<Imported->GetNumMaterials();++Slot)
            {
                const auto* Material=Cast<UMaterialInstanceDynamic>(Imported->GetMaterial(Slot));
                if(!TestNotNull(TEXT("Exterior retains clipped imported dynamic materials"),Material)) continue;
                float Blend=0; FLinearColor Color;
                TestTrue(TEXT("Existing imported shader receives exterior albedo blend"),Material->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Flame")),Blend));
                TestTrue(TEXT("Exterior blend keeps faint relief and softens perimeter"),Blend>=.819f && Blend<=.961f);
                TestTrue(TEXT("Existing imported shader receives blue exterior tint"),Material->GetVectorParameterValue(FMaterialParameterInfo(TEXT("FlameColor")),Color));
                TestTrue(TEXT("Exterior tint is dark desaturated blue"),Color.B>Color.G && Color.G>Color.R && Color.B<=.281f && Color.B>=.094f);
                const float EdgeMix=FMath::Clamp((Blend-.82f)/.14f,0.f,1.f);
                TestTrue(TEXT("Exterior tint follows exact inner-to-haze palette"),
                    Color.Equals(FMath::Lerp(FLinearColor(.035f,.055f,.095f,1.f),FLinearColor(.11f,.17f,.28f,1.f),EdgeMix),.0001f));
            }
        }
        const FBox Envelope=Fit.ClipBounds.TransformBy(Piece->GetActorTransform());
        TestTrue(TEXT("All apron and outline footprints avoid every original floor/ramp plus 8cm"),
            !Floors.ContainsByPredicate([&](const FBox& Floor){return OverlapXY(Envelope,Floor.ExpandBy(FVector(8,8,0)));}));
        TestTrue(TEXT("No surroundings silhouette covers any walkable camera shadow envelope"),ClearSight(Envelope,Floors));
        if(Apron)
        {
            for(const auto& Instance:Fit.Instances)
                TestTrue(TEXT("Apron paving uses 800cm units without changing vertical thickness"),
                    Instance.GetScale3D().Equals(FVector(2,2,1),.001));
            auto InteriorRecipe=Recipe; InteriorRecipe.Colors[0]=FLinearColor::White;
            LHEnvArt::FFit InteriorFit;
            if(TestTrue(TEXT("Unmarked interior still fits original Church floor"),LHEnvArt::Resolve(InteriorRecipe,InteriorFit)))
            {
                TestTrue(TEXT("Larger apron units reduce imported instances"),Fit.Instances.Num()<=InteriorFit.Instances.Num());
                TestTrue(TEXT("Exterior tiling preserves original clip envelope"),Fit.ClipBounds.Equals(InteriorFit.ClipBounds,.001));
                for(const auto& Instance:InteriorFit.Instances)
                    TestTrue(TEXT("Interior paving retains 400cm units"),Instance.GetScale3D().Equals(FVector::OneVector,.001));
            }
            ++Aprons;
            MainAprons+=Piece->GetActorLocation().Y<4000?1:0;
            MageAprons+=Piece->GetActorLocation().Y>4000?1:0;
            TestTrue(TEXT("Apron is above scenic water and below playable flat floor"),FMath::IsNearlyEqual(Envelope.Max.Z,-24.,.01));
            TestTrue(TEXT("Apron assemblies are tiled at most 1200cm in XY"),Envelope.GetSize().X<=1200.01 && Envelope.GetSize().Y<=1200.01);
        }
        if(Outline)
        {
            ++Outlines;
            TestTrue(TEXT("Outline remains an 80cm low wall"),FMath::IsNearlyEqual(Envelope.GetSize().Z,80.,.01));
        }
    }
    TestTrue(TEXT("Church chapel surroundings fill camera void"),MainAprons>0);
    TestTrue(TEXT("Mage compound surroundings fill camera void"),MageAprons>0);
    TestTrue(TEXT("Low perimeter outline is present"),Outlines>0);
    const auto Totals=LHVisual::ValidatePlacedSet(Added);
    for(const auto& Error:Totals.Errors) AddError(Error);
    TestTrue(TEXT("Added imported surroundings stay below 240000 triangles"),Totals.Triangles>0 && Totals.Triangles<=240000);
    TestEqual(TEXT("Surroundings add no gameplay blockers"),Totals.CollisionBoxes,0);
    AddInfo(FString::Printf(TEXT("Church surroundings: aprons=%d (chapel=%d mage=%d) outline=%d original floors=%d ramp=%d triangles=%d/240000"),
        Aprons,MainAprons,MageAprons,Outlines,Floors.Num(),Ramps,Totals.Triangles));
    return true;
}
