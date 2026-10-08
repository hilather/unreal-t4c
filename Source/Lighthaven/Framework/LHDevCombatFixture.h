#pragma once
class AActor;
class ULHCombatComponent;
namespace LHDevCombat
{
    // Explicit synthetic tuning, only for generated dev worlds. Not canonical restore.
    void InitializeForMap(ULHCombatComponent* Combat, AActor* Avatar);
}
