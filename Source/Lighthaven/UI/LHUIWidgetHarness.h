#pragma once
#include "CoreMinimal.h"
#include "UI/LHUIPresenter.h"
// Opaque test seam: widget/event construction lives in the runtime UI module,
// keeping the test module free of a new direct Slate dependency.
enum class ELHUITestKey : uint8 { Down, Up, South, East, RightShoulder, Right, Left, Character, Inventory };
class LIGHTHAVEN_API ILHUIWidgetHarness
{
public:
    virtual ~ILHUIWidgetHarness() = default;
    virtual void Open(ELHUIScreen Screen) = 0;
    virtual void Activate(FName Id) = 0;
    virtual void EditName(const FString& Name) = 0;
    virtual FString FieldName() const = 0;
    virtual FString SummaryText() const = 0;
    virtual FString MessageText() const = 0;
    virtual bool HasControl(FName Id) const = 0;
    virtual bool IsModal() const = 0;
    virtual void Key(ELHUITestKey Key, bool bRepeat = false) = 0;
    static TUniquePtr<ILHUIWidgetHarness> Create(FLHUIPresenter& Presenter);
};
