// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MetalCarsUI.generated.h"

/**
 *  Simple Vehicle HUD class
 *  Displays the current speed and gear.
 *  Widget setup is handled in a Blueprint subclass.
 */
UCLASS(abstract)
class UMetalCarsUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	/** Controls the display of speed in Km/h or MPH */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle")
	bool bIsMPH = false;

public:

	/** Called to update the speed display */
	void UpdateSpeed(float NewSpeed);

	/** Called to update the gear display */
	void UpdateGear(int32 NewGear);

	void UpdateHealth(float CurrentHealth, float MaxHealth);

	void UpdateMissileInfo(const FString& MissileName, int32 Ammo);

	void UpdateScore(int32 Kills, int32 Deaths, int32 Score);

	void UpdateMatchTime(float TimeRemaining);

protected:

	/** Implemented in Blueprint to display the new speed */
	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void OnSpeedUpdate(float NewSpeed);

	/** Implemented in Blueprint to display the new gear */
	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void OnGearUpdate(int32 NewGear);

	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void OnHealthUpdate(float CurrentHealth, float MaxHealth, float HealthPercent);

	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void OnMissileInfoUpdate(const FString& MissileName, int32 Ammo);

	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void OnScoreUpdate(int32 Kills, int32 Deaths, int32 Score);

	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void OnMatchTimeUpdate(float TimeRemaining);
};
