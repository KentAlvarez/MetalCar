// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MetalScoreboardEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class METALCARS_API UMetalScoreboardEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Scoreboard")
	void SetEntryData(const FString& PlayerName, int32 Kills, int32 Deaths, int32 Score);

protected:

	UFUNCTION(BlueprintImplementableEvent, Category="Scoreboard")
	void BP_OnEntryDataUpdated(const FString& PlayerName, int32 Kills, int32 Deaths, int32 Score);
};
