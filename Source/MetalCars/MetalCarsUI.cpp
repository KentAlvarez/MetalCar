// Copyright Epic Games, Inc. All Rights Reserved.


#include "MetalCarsUI.h"


void UMetalCarsUI::UpdateSpeed(float NewSpeed)
{
	// format the speed to KPH or MPH
	float FormattedSpeed = FMath::Abs(NewSpeed) * (bIsMPH ? 0.022f : 0.036f);

	// call the Blueprint handler
	OnSpeedUpdate(FormattedSpeed);
}

void UMetalCarsUI::UpdateGear(int32 NewGear)
{
	// call the Blueprint handler
	OnGearUpdate(NewGear);
}

void UMetalCarsUI::UpdateHealth(float CurrentHealth, float MaxHealth)
{
	const float HealthPercent = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
	OnHealthUpdate(CurrentHealth, MaxHealth, HealthPercent);
}

void UMetalCarsUI::UpdateMissileInfo(const FString& MissileName, int32 Ammo)
{
	OnMissileInfoUpdate(MissileName, Ammo);
}

void UMetalCarsUI::UpdateScore(int32 Kills, int32 Deaths, int32 Score)
{
	OnScoreUpdate(Kills, Deaths, Score);
}

void UMetalCarsUI::UpdateMatchTime(float TimeRemaining)
{
	OnMatchTimeUpdate(TimeRemaining);
}

void UMetalCarsUI::UpdateMatchResult(bool bMatchFinished, const FString& WinnerName)
{
	OnMatchResultUpdate(bMatchFinished, WinnerName);
}