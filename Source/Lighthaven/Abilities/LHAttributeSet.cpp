#include "Abilities/LHAttributeSet.h"
void ULHAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);
    if (!FMath::IsFinite(NewValue)) NewValue = 0;
    if (Attribute == GetHealthAttribute()) NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    if (Attribute == GetManaAttribute()) NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
    if (Attribute == GetMaxHealthAttribute() || Attribute == GetMaxManaAttribute() ||
        Attribute == GetArmorAttribute() || Attribute == GetResistanceAttribute()) NewValue = FMath::Max(0.f, NewValue);
}
