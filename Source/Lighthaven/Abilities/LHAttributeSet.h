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
    static FGameplayAttribute GetHealthAttribute();
    float GetHealth() const;
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Health)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Health)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData MaxHealth;
    static FGameplayAttribute GetMaxHealthAttribute();
    float GetMaxHealth() const;
    GAMEPLAYATTRIBUTE_VALUE_SETTER(MaxHealth)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(MaxHealth)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Mana;
    static FGameplayAttribute GetManaAttribute();
    float GetMana() const;
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Mana)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Mana)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData MaxMana;
    static FGameplayAttribute GetMaxManaAttribute();
    float GetMaxMana() const;
    GAMEPLAYATTRIBUTE_VALUE_SETTER(MaxMana)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(MaxMana)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Accuracy;
    static FGameplayAttribute GetAccuracyAttribute();
    float GetAccuracy() const;
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Accuracy)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Accuracy)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Avoidance;
    static FGameplayAttribute GetAvoidanceAttribute();
    float GetAvoidance() const;
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Avoidance)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Avoidance)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData DamageBonus;
    static FGameplayAttribute GetDamageBonusAttribute();
    float GetDamageBonus() const;
    GAMEPLAYATTRIBUTE_VALUE_SETTER(DamageBonus)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(DamageBonus)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Armor;
    static FGameplayAttribute GetArmorAttribute();
    float GetArmor() const;
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Armor)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Armor)
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Resistance;
    static FGameplayAttribute GetResistanceAttribute();
    float GetResistance() const;
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Resistance)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Resistance)
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
};
