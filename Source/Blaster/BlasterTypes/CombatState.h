#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "UnOccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_MAX UMETA(DisplayName = "DefaultMax")
};