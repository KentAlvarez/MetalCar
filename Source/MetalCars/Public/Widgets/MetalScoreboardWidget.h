// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MetalScoreboardWidget.generated.h"

class UMetalScoreboardEntryWidget;
class UVerticalBox;

UCLASS(Abstract)
class METALCARS_API UMetalScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> EntriesBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scoreboard")
	TSubclassOf<UMetalScoreboardEntryWidget> EntryWidgetClass;

public:

	UFUNCTION(BlueprintCallable, Category="Scoreboard")
	void RefreshScoreboard();
};
