// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetalCarsGameMode.h"

#include "MetalCarsGameState.h"
#include "MetalCarsPlayerController.h"
#include "MetalCarsPlayerState.h"
#include "MetalCar/Cars/MetalCombatVehicle.h"
#include "TimerManager.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

AMetalCarsGameMode::AMetalCarsGameMode()
{
	PlayerControllerClass = AMetalCarsPlayerController::StaticClass();
	PlayerStateClass = AMetalCarsPlayerState::StaticClass();
	GameStateClass = AMetalCarsGameState::StaticClass();
}

void AMetalCarsGameMode::HandleVehicleDeath(
	AMetalCombatVehicle* DeadVehicle,
	AController* KillerController
)
{
	if (!HasAuthority() || !DeadVehicle)
	{
		return;
	}

	AMetalCarsGameState* MetalGS = GetGameState<AMetalCarsGameState>();

	if (MetalGS && MetalGS->IsMatchFinished())
	{
		return;
	}

	AController* DeadController = DeadVehicle->GetController();

	if (AMetalCarsPlayerState* DeadPS = DeadController ? DeadController->GetPlayerState<AMetalCarsPlayerState>() : nullptr)
	{
		DeadPS->AddDeath(1);
	}

	if (KillerController && KillerController != DeadController)
	{
		if (AMetalCarsPlayerState* KillerPS = KillerController->GetPlayerState<AMetalCarsPlayerState>())
		{
			KillerPS->AddKill(1);
			KillerPS->AddCombatScore(KillScoreValue);
		}
	}

	if (DeadController)
	{
		DeadController->UnPossess();
	}

	DeadVehicle->SetLifeSpan(RespawnDelay);

	if (DeadController)
	{
		FTimerHandle RespawnTimerHandle;

		FTimerDelegate RespawnDelegate;
		RespawnDelegate.BindUObject(this, &AMetalCarsGameMode::RespawnController, DeadController);

		GetWorldTimerManager().SetTimer(
			RespawnTimerHandle,
			RespawnDelegate,
			RespawnDelay,
			false
		);
	}
}

void AMetalCarsGameMode::RespawnController(AController* ControllerToRespawn)
{
	if (!HasAuthority() || !ControllerToRespawn)
	{
		return;
	}

	if (AMetalCarsGameState* MetalGS = GetGameState<AMetalCarsGameState>())
	{
		if (MetalGS->IsMatchFinished())
		{
			return;
		}
	}

	RestartPlayer(ControllerToRespawn);
}

void AMetalCarsGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (AMetalCarsGameState* MetalGS = GetGameState<AMetalCarsGameState>())
	{
		MetalGS->SetTimeRemaining(MatchDuration);
		MetalGS->SetMatchFinished(false);
		MetalGS->SetWinnerName(TEXT(""));
	}

	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&AMetalCarsGameMode::TickMatchTimer,
		1.0f,
		true
	);
}

void AMetalCarsGameMode::TickMatchTimer()
{
	AMetalCarsGameState* MetalGS = GetGameState<AMetalCarsGameState>();

	if (!MetalGS || MetalGS->IsMatchFinished())
	{
		return;
	}

	const float NewTime = MetalGS->GetTimeRemaining() - 1.0f;
	MetalGS->SetTimeRemaining(NewTime);

	if (NewTime <= 0.0f)
	{
		FinishMatch();
	}
}

AMetalCarsPlayerState* AMetalCarsGameMode::FindWinnerPlayerState() const
{
	AMetalCarsPlayerState* WinnerPS = nullptr;
	int32 BestScore = TNumericLimits<int32>::Min();

	for (APlayerState* PS : GameState->PlayerArray)
	{
		AMetalCarsPlayerState* MetalPS = Cast<AMetalCarsPlayerState>(PS);

		if (!MetalPS)
		{
			continue;
		}

		if (MetalPS->GetCombatScore() > BestScore)
		{
			BestScore = MetalPS->GetCombatScore();
			WinnerPS = MetalPS;
		}
	}

	return WinnerPS;
}


void AMetalCarsGameMode::FinishMatch()
{
	AMetalCarsGameState* MetalGS = GetGameState<AMetalCarsGameState>();

	if (!MetalGS || MetalGS->IsMatchFinished())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	AMetalCarsPlayerState* WinnerPS = FindWinnerPlayerState();

	FString WinnerName = TEXT("No winner");

	if (WinnerPS)
	{
		WinnerName = WinnerPS->GetPlayerName();
	}

	MetalGS->SetWinnerName(WinnerName);
	MetalGS->SetMatchFinished(true);

	// Bloquear input de todos los jugadores
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();

		if (!PC)
		{
			continue;
		}

		if (APawn* ControlledPawn = PC->GetPawn())
		{
			ControlledPawn->DisableInput(PC);
		}
	}
}

AActor* AMetalCarsGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> PlayerStarts;

	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		APlayerStart::StaticClass(),
		PlayerStarts
	);

	if (PlayerStarts.Num() <= 0)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	TArray<AActor*> SafeStarts;

	for (AActor* Start : PlayerStarts)
	{
		if (IsSpawnPointSafe(Start))
		{
			SafeStarts.Add(Start);
		}
	}

	if (SafeStarts.Num() > 0)
	{
		const int32 RandomIndex = FMath::RandRange(0, SafeStarts.Num() - 1);
		return SafeStarts[RandomIndex];
	}

	// Si todos están ocupados, usa cualquiera aleatorio.
	const int32 RandomIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
	return PlayerStarts[RandomIndex];
}

bool AMetalCarsGameMode::IsSpawnPointSafe(AActor* SpawnPoint) const
{
	if (!SpawnPoint || !GetWorld())
	{
		return false;
	}

	const FVector SpawnLocation = SpawnPoint->GetActorLocation();

	TArray<FOverlapResult> Overlaps;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(SpawnCheckRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(SpawnPoint);

	const bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		SpawnLocation,
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		QueryParams
	);

	return !bHasOverlap;
}