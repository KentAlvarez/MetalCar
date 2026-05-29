// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MetalCarsGameMode.generated.h"

class AMetalCombatVehicle;

UCLASS(abstract)
class AMetalCarsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMetalCarsGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Respawn")
	float RespawnDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Score")
	int32 KillScoreValue = 100;

public:

	void HandleVehicleDeath(
		AMetalCombatVehicle* DeadVehicle,
		AController* KillerController
	);

protected:

	void RespawnController(AController* ControllerToRespawn);
};



