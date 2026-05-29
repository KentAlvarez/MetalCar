// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MetalScoreboardEntryWidget.h"


void UMetalScoreboardEntryWidget::SetEntryData(const FString& PlayerName, int32 Kills, int32 Deaths, int32 Score)
{
	BP_OnEntryDataUpdated(PlayerName, Kills, Deaths, Score);
}