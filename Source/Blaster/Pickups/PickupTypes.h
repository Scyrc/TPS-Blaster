#pragma once

UENUM(BlueprintType)
enum class EPickupHighLight : uint8
{
	None = 0,
	EPHL_PURPLE=250 UMETA(DisplayName = "PURPLE"),
	EPHL_BLUE UMETA(DisplayName = "BLUE"),
	EPHL_TAN UMETA(DisplayName = "TAN"),

	EPHL_MAX UMETA(DisplayName = "DefaultMax"),

};
