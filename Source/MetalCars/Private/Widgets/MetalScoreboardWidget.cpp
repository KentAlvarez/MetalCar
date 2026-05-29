// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MetalScoreboardWidget.h"
#include "Components/VerticalBox.h"
#include "MetalCarsPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Widgets/MetalScoreboardEntryWidget.h"

void UMetalScoreboardWidget::RefreshScoreboard()
{
	if (!EntriesBox || !EntryWidgetClass)
	{
		return;
	}

	EntriesBox->ClearChildren();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AGameStateBase* GS = World->GetGameState();
	if (!GS)
	{
		return;
	}

	TArray<AMetalCarsPlayerState*> SortedPlayers;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (AMetalCarsPlayerState* MetalPS = Cast<AMetalCarsPlayerState>(PS))
		{
			SortedPlayers.Add(MetalPS);
		}
	}

	SortedPlayers.Sort([](const AMetalCarsPlayerState& A, const AMetalCarsPlayerState& B)
	{
		return A.GetCombatScore() > B.GetCombatScore();
	});

	for (AMetalCarsPlayerState* MetalPS : SortedPlayers)
	{
		if (!MetalPS)
		{
			continue;
		}

		UMetalScoreboardEntryWidget* Entry = CreateWidget<UMetalScoreboardEntryWidget>(
			GetOwningPlayer(),
			EntryWidgetClass
		);

		if (!Entry)
		{
			continue;
		}

		Entry->SetEntryData(
			MetalPS->GetPlayerName(),
			MetalPS->GetKills(),
			MetalPS->GetDeaths(),
			MetalPS->GetCombatScore()
		);

		EntriesBox->AddChild(Entry);
	}
}
