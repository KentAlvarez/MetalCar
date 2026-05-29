// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MetalCarsPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class METALCARS_API AMetalCarsPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMetalCarsPlayerState();

protected:

	UPROPERTY(ReplicatedUsing=OnRep_Kills, BlueprintReadOnly, Category="Score")
	int32 Kills = 0;

	UPROPERTY(ReplicatedUsing=OnRep_Deaths, BlueprintReadOnly, Category="Score")
	int32 Deaths = 0;

	UPROPERTY(ReplicatedUsing=OnRep_CombatScore, BlueprintReadOnly, Category="Score")
	int32 CombatScore = 0;

	UFUNCTION()
	void OnRep_Kills();

	UFUNCTION()
	void OnRep_Deaths();

	UFUNCTION()
	void OnRep_CombatScore();

	UFUNCTION(BlueprintImplementableEvent, Category="Score")
	void BP_OnScoreChanged();

public:

	UFUNCTION(BlueprintCallable, Category="Score")
	int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintCallable, Category="Score")
	int32 GetDeaths() const { return Deaths; }

	UFUNCTION(BlueprintCallable, Category="Score")
	int32 GetCombatScore() const { return CombatScore; }

	void AddKill(int32 Amount = 1);
	void AddDeath(int32 Amount = 1);
	void AddCombatScore(int32 Amount);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
