#pragma once
#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "LHAttributeSet.generated.h"

UCLASS()
class LIGHTHAVEN_API ULHAttributeSet : public UAttributeSet
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Health;
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ULHAttributeSet, Health)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(Health)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Health)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Health)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData MaxHealth;
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ULHAttributeSet, MaxHealth)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(MaxHealth)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(MaxHealth)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(MaxHealth)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Mana;
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ULHAttributeSet, Mana)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(Mana)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Mana)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Mana)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData MaxMana;
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ULHAttributeSet, MaxMana)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(MaxMana)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(MaxMana)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(MaxMana)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Accuracy;
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ULHAttributeSet, Accuracy)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(Accuracy)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Accuracy)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Accuracy)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Avoidance;
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ULHAttributeSet, Avoidance)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(Avoidance)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Avoidance)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Avoidance)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData DamageBonus;
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ULHAttributeSet, DamageBonus)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(DamageBonus)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(DamageBonus)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(DamageBonus)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Armor;
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ULHAttributeSet, Armor)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(Armor)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Armor)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Armor)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Resistance;
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ULHAttributeSet, Resistance)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(Resistance)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Resistance)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Resistance)
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
};
