// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetalCarsWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UMetalCarsWheelRear::UMetalCarsWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}