// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetalCarsGameMode.h"
#include "MetalCarsPlayerController.h"
#include "MetalCarsPlayerState.h"
#include "MetalCar/Cars/MetalCombatVehicle.h"
#include "TimerManager.h"

AMetalCarsGameMode::AMetalCarsGameMode()
{
	PlayerControllerClass = AMetalCarsPlayerController::StaticClass();
	PlayerStateClass = AMetalCarsPlayerState::StaticClass();
}

void AMetalCarsGameMode::HandleVehicleDeath(
	AMetalCombatVehicle* DeadVehicle,
	AController* KillerController
)
{
	if (!HasAuthority() || !DeadVehicle)
	{
		return;
	}

	AController* DeadController = DeadVehicle->GetController();

	if (AMetalCarsPlayerState* DeadPS = DeadController ? DeadController->GetPlayerState<AMetalCarsPlayerState>() : nullptr)
	{
		DeadPS->AddDeath(1);
	}

	// Si no fue suicidio, sumamos kill al atacante.
	if (KillerController && KillerController != DeadController)
	{
		if (AMetalCarsPlayerState* KillerPS = KillerController->GetPlayerState<AMetalCarsPlayerState>())
		{
			KillerPS->AddKill(1);
			KillerPS->AddCombatScore(KillScoreValue);
		}
	}

	// Sacamos el pawn del controller antes de destruirlo.
	if (DeadController)
	{
		DeadController->UnPossess();
	}

	// Dejá que se vea destruido unos segundos.
	DeadVehicle->SetLifeSpan(RespawnDelay);

	if (DeadController)
	{
		FTimerHandle RespawnTimerHandle;

		FTimerDelegate RespawnDelegate;
		RespawnDelegate.BindUObject(this, &AMetalCarsGameMode::RespawnController, DeadController);

		GetWorldTimerManager().SetTimer(
			RespawnTimerHandle,
			RespawnDelegate,
			RespawnDelay,
			false
		);
	}
}

void AMetalCarsGameMode::RespawnController(AController* ControllerToRespawn)
{
	if (!HasAuthority() || !ControllerToRespawn)
	{
		return;
	}

	RestartPlayer(ControllerToRespawn);
}