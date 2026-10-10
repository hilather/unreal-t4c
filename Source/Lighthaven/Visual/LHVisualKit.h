#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LHVisualKit.generated.h"

class UProceduralMeshComponent;
class UBoxComponent;
class UMaterialInterface;

// Prototype presentation only. Never carries gameplay or persistent identity.
UENUM()
enum class ELHVisualStyle : uint8 { Church, B1Cellar, B2Damp, B3Crypt, B4Ritual };

USTRUCT()
struct LIGHTHAVEN_API FLHVisualBox
{
    GENERATED_BODY()
    UPROPERTY() FVector Center = FVector::ZeroVector;
    UPROPERTY() FVector Size = FVector::OneVector;
    UPROPERTY() FRotator Rotation = FRotator::ZeroRotator;
    UPROPERTY() int32 Surface = 0;
};
USTRUCT()
struct LIGHTHAVEN_API FLHVisualRecipe
{
    GENERATED_BODY()
    UPROPERTY() FName Id;
    UPROPERTY() ELHVisualStyle Style = ELHVisualStyle::Church;
    UPROPERTY() TArray<FLHVisualBox> Geometry;
    UPROPERTY() TArray<FLHVisualBox> Collision;
    UPROPERTY() FLinearColor Colors[2] = {FLinearColor::White, FLinearColor::White};
    UPROPERTY() float Roughness[2] = {0.85f, 0.9f};
    UPROPERTY() int32 TriangleBudget = 2000;
    UPROPERTY() int32 DrawBudget = 2;
    uint32 Fingerprint() const;
};
struct LIGHTHAVEN_API FLHVisualTotals
{
    int32 Pieces = 0, Triangles = 0, DrawCalls = 0, CollisionBoxes = 0;
    TArray<FString> Errors;
};

UCLASS()
class LIGHTHAVEN_API ALHVisualPiece : public AActor
{
    GENERATED_BODY()
public:
    ALHVisualPiece();
    bool Build(const FLHVisualRecipe& Recipe);
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void PostLoad() override;
    const FLHVisualRecipe& GetRecipe() const { return BuiltRecipe; }
    UProceduralMeshComponent* GetMesh() const { return Mesh; }
    const TArray<TObjectPtr<UBoxComponent>>& GetBlockers() const { return Blockers; }
private:
    UPROPERTY(VisibleAnywhere, Transient) TObjectPtr<UProceduralMeshComponent> Mesh;
    UPROPERTY() TArray<TObjectPtr<UBoxComponent>> Blockers;
    UPROPERTY() FLHVisualRecipe BuiltRecipe;
    UPROPERTY() TObjectPtr<UMaterialInterface> MaterialParent;
};

namespace LHVisual
{
    // IDs are Presentation.Environment.<suffix>. Exact catalog includes all supported IDs.
    LIGHTHAVEN_API const TArray<FName>& PieceIds();
    LIGHTHAVEN_API bool MakeRecipe(FName Id, ELHVisualStyle Style, FLHVisualRecipe& Out, bool bDescending = false);
    LIGHTHAVEN_API ALHVisualPiece* SpawnProp(UWorld* World, FName Id, const FTransform& Transform,
        ELHVisualStyle Style = ELHVisualStyle::Church, bool bDescending = false);
    // Start/End are bottom corners on the clear wall face; horizontal runs only.
    // Segments <=400 cm; wall extends to local Y=-20, Z=400. Atomic on invalid input.
    LIGHTHAVEN_API TArray<ALHVisualPiece*> SpawnWallRun(UWorld* World, FVector Start, FVector End, ELHVisualStyle Style);
    // Measured CPU geometry / section totals, conservative base-pass draw estimate (not GPU profiling).
    LIGHTHAVEN_API FLHVisualTotals ValidatePlacedSet(const TArray<ALHVisualPiece*>& Pieces);
}
