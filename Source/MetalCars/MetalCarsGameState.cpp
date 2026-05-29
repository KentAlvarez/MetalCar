// Fill out your copyright notice in the Description page of Project Settings.


#include "MetalCarsGameState.h"

#include "Net/UnrealNetwork.h"


AMetalCarsGameState::AMetalCarsGameState()
{
	bReplicates = true;
}

void AMetalCarsGameState::SetTimeRemaining(float NewTime)
{
	if (!HasAuthority())
	{
		return;
	}

	TimeRemaining = FMath::Max(0.0f, NewTime);
	BP_OnMatchStateChanged();
}

void AMetalCarsGameState::SetMatchFinished(bool bNewFinished)
{
	if (!HasAuthority())
	{
		return;
	}

	bMatchFinished = bNewFinished;
	BP_OnMatchStateChanged();
}

void AMetalCarsGameState::SetWinnerName(const FString& NewWinnerName)
{
	if (!HasAuthority())
	{
		return;
	}

	WinnerName = NewWinnerName;
	BP_OnMatchStateChanged();
}

void AMetalCarsGameState::OnRep_TimeRemaining()
{
	BP_OnMatchStateChanged();
}

void AMetalCarsGameState::OnRep_MatchFinished()
{
	BP_OnMatchStateChanged();
}

void AMetalCarsGameState::OnRep_WinnerName()
{
	BP_OnMatchStateChanged();
}

void AMetalCarsGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMetalCarsGameState, TimeRemaining);
	DOREPLIFETIME(AMetalCarsGameState, bMatchFinished);
	DOREPLIFETIME(AMetalCarsGameState, WinnerName);
}