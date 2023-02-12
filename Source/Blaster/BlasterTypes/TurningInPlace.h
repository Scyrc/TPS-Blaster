#pragma once
#include "HAL/Platform.h"
#include "UObject/Class.h"
UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	ETIP_Left UMETA(DisplayName = "Truing Left"),
	ETIP_Right UMETA(DisplayName = "Truing Right"),
	ETIP_NotTuring UMETA(DisplayName = "Not Turing"),

	ETIP_MAX UMETA(DisplayName = "DefaultMAX"),

};