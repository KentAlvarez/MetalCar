// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "MetalCarsGameMode.generated.h"

class AMetalCombatVehicle;
class AMetalCarsPlayerState;

UCLASS(abstract)
class AMetalCarsGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMetalCarsGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Respawn")
	float RespawnDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Score")
	int32 KillScoreValue = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Match")
	float MatchDuration = 300.0f;

public:

	void HandleVehicleDeath(
		AMetalCombatVehicle* DeadVehicle,
		AController* KillerController
	);

protected:

	FTimerHandle MatchTimerHandle;

	virtual void BeginPlay() override;

	void RespawnController(AController* ControllerToRespawn);

	void TickMatchTimer();

	void FinishMatch();

	AMetalCarsPlayerState* FindWinnerPlayerState() const;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawn")
	float SpawnCheckRadius = 600.0f;

	bool IsSpawnPointSafe(AActor* SpawnPoint) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Vehicle Random Spawn")
	TArray<TSubclassOf<APawn>> RandomVehicleClasses;

	UPROPERTY()
	TArray<int32> VehicleBag;

	int32 LastPickedVehicleIndex = INDEX_NONE;

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	void RefillVehicleBag();

	TSubclassOf<APawn> GetRandomVehicleClassFromBag();
	
	TMap<TWeakObjectPtr<AController>, TSubclassOf<APawn>> AssignedVehicleClasses;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	void AssignRandomVehicleToController(AController* Controller);
};


