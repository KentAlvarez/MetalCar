// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MetalCarsGameState.generated.h"

/**
 * 
 */
UCLASS()
class METALCARS_API AMetalCarsGameState : public AGameState
{
	GENERATED_BODY()

public:
	AMetalCarsGameState();

protected:

	UPROPERTY(ReplicatedUsing=OnRep_TimeRemaining, BlueprintReadOnly, Category="Match")
	float TimeRemaining = 300.0f;

	UPROPERTY(ReplicatedUsing=OnRep_MatchFinished, BlueprintReadOnly, Category="Match")
	bool bMatchFinished = false;

	UPROPERTY(ReplicatedUsing=OnRep_WinnerName, BlueprintReadOnly, Category="Match")
	FString WinnerName;

	UFUNCTION()
	void OnRep_TimeRemaining();

	UFUNCTION()
	void OnRep_MatchFinished();

	UFUNCTION()
	void OnRep_WinnerName();

	UFUNCTION(BlueprintImplementableEvent, Category="Match")
	void BP_OnMatchStateChanged();

public:

	UFUNCTION(BlueprintCallable, Category="Match")
	float GetTimeRemaining() const { return TimeRemaining; }

	UFUNCTION(BlueprintCallable, Category="Match")
	bool IsMatchFinished() const { return bMatchFinished; }

	UFUNCTION(BlueprintCallable, Category="Match")
	FString GetWinnerName() const { return WinnerName; }

	void SetTimeRemaining(float NewTime);
	void SetMatchFinished(bool bNewFinished);
	void SetWinnerName(const FString& NewWinnerName);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
