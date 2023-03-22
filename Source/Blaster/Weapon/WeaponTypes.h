#pragma once

#define TRACE_LENGTH 80000.f;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SubmachineGun UMETA(DisplayName = "SunmachineGun"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_SniperRifle UMETA(DisplayName = "SniperRifle"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
	EWT_Flag UMETA(DisplayName = "Flag"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};


UENUM(BlueprintType)
enum class EWeaponHighLight : uint8
{
	None = 0,
	EWHL_PURPLE=250 UMETA(DisplayName = "PURPLE"),
	EWHL_BLUE UMETA(DisplayName = "BLUE"),
	EWHL_TAN UMETA(DisplayName = "TAN"),

	EWHL_MAX UMETA(DisplayName = "DefaultMAX")
};