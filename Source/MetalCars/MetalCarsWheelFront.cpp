// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetalCarsWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UMetalCarsWheelFront::UMetalCarsWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}