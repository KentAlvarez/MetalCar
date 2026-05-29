// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MetalCarsPawn.h"
#include "MetalCarsSportsCar.generated.h"

/**
 *  Implementacion del vehiculo con ruedas de auto deportivo
 */
UCLASS(abstract)
class AMetalCarsSportsCar : public AMetalCarsPawn
{
	GENERATED_BODY()
	
public:

	AMetalCarsSportsCar();
};
