// Fill out your copyright notice in the Description page of Project Settings.


#include "MetalCarsPlayerState.h"
#include "Net/UnrealNetwork.h"

AMetalCarsPlayerState::AMetalCarsPlayerState()
{
	bReplicates = true;
}

void AMetalCarsPlayerState::AddKill(int32 Amount)
{
	if (!HasAuthority())
	{
		return;
	}

	Kills += Amount;
	BP_OnScoreChanged();
}

void AMetalCarsPlayerState::AddDeath(int32 Amount)
{
	if (!HasAuthority())
	{
		return;
	}

	Deaths += Amount;
	BP_OnScoreChanged();
}

void AMetalCarsPlayerState::AddCombatScore(int32 Amount)
{
	if (!HasAuthority())
	{
		return;
	}

	CombatScore += Amount;
	BP_OnScoreChanged();
}

void AMetalCarsPlayerState::OnRep_Kills()
{
	BP_OnScoreChanged();
}

void AMetalCarsPlayerState::OnRep_Deaths()
{
	BP_OnScoreChanged();
}

void AMetalCarsPlayerState::OnRep_CombatScore()
{
	BP_OnScoreChanged();
}

void AMetalCarsPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMetalCarsPlayerState, Kills);
	DOREPLIFETIME(AMetalCarsPlayerState, Deaths);
	DOREPLIFETIME(AMetalCarsPlayerState, CombatScore);
}