#pragma once

#include "CoreMinimal.h"
#include "MetalDamageTypes.generated.h"

UENUM(BlueprintType)
enum class EVehicleDamageState : uint8
{
	Healthy			UMETA(DisplayName = "Healthy"),
	LightDamage		UMETA(DisplayName = "Light Damage"),
	HeavyDamage		UMETA(DisplayName = "Heavy Damage"),
	CriticalDamage	UMETA(DisplayName = "Critical Damage"),
	Destroyed		UMETA(DisplayName = "Destroyed")
};