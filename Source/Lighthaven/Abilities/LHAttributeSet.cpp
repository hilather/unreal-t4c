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

FGameplayAttribute ULHAttributeSet::GetHealthAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ULHAttributeSet, Health));
    return FGameplayAttribute(Property);
}
float ULHAttributeSet::GetHealth() const
{
    return Health.GetCurrentValue();
}

FGameplayAttribute ULHAttributeSet::GetMaxHealthAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ULHAttributeSet, MaxHealth));
    return FGameplayAttribute(Property);
}
float ULHAttributeSet::GetMaxHealth() const
{
    return MaxHealth.GetCurrentValue();
}

FGameplayAttribute ULHAttributeSet::GetManaAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ULHAttributeSet, Mana));
    return FGameplayAttribute(Property);
}
float ULHAttributeSet::GetMana() const
{
    return Mana.GetCurrentValue();
}

FGameplayAttribute ULHAttributeSet::GetMaxManaAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ULHAttributeSet, MaxMana));
    return FGameplayAttribute(Property);
}
float ULHAttributeSet::GetMaxMana() const
{
    return MaxMana.GetCurrentValue();
}

FGameplayAttribute ULHAttributeSet::GetAccuracyAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ULHAttributeSet, Accuracy));
    return FGameplayAttribute(Property);
}
float ULHAttributeSet::GetAccuracy() const
{
    return Accuracy.GetCurrentValue();
}

FGameplayAttribute ULHAttributeSet::GetAvoidanceAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ULHAttributeSet, Avoidance));
    return FGameplayAttribute(Property);
}
float ULHAttributeSet::GetAvoidance() const
{
    return Avoidance.GetCurrentValue();
}

FGameplayAttribute ULHAttributeSet::GetDamageBonusAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ULHAttributeSet, DamageBonus));
    return FGameplayAttribute(Property);
}
float ULHAttributeSet::GetDamageBonus() const
{
    return DamageBonus.GetCurrentValue();
}

FGameplayAttribute ULHAttributeSet::GetArmorAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ULHAttributeSet, Armor));
    return FGameplayAttribute(Property);
}
float ULHAttributeSet::GetArmor() const
{
    return Armor.GetCurrentValue();
}

FGameplayAttribute ULHAttributeSet::GetResistanceAttribute()
{
    static FProperty* Property = FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ULHAttributeSet, Resistance));
    return FGameplayAttribute(Property);
}
float ULHAttributeSet::GetResistance() const
{
    return Resistance.GetCurrentValue();
}
