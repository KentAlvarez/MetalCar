// Copyright Epic Games, Inc. All Rights Reserved.


#include "MetalCarsPlayerController.h"
#include "MetalCarsPawn.h"
#include "MetalCarsUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "MetalCars.h"
#include "MetalCarsGameState.h"
#include "MetalCarsPlayerState.h"
#include "Actors/Projectiles/MetalProjectile.h"
#include "Components/MetalHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "MetalCar/Cars/MetalCombatVehicle.h"
#include "Widgets/MetalScoreboardWidget.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AMetalCarsPlayerController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);

	VehiclePawn = Cast<AMetalCarsPawn>(InPawn);

	UE_LOG(LogTemp, Warning, TEXT("CLIENT AcknowledgePossession VehiclePawn: %s"),
		*GetNameSafe(VehiclePawn));
}

void AMetalCarsPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// ensure we're attached to the vehicle pawn so that World Partition streaming works correctly
	bAttachToPawn = true;

	// only spawn UI on local player controllers
	if (IsLocalPlayerController())
	{
		if (ShouldUseTouchControls())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogMetalCars, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}
		

		// spawn the UI widget and add it to the viewport
		VehicleUI = CreateWidget<UMetalCarsUI>(this, VehicleUIClass);

		if (VehicleUI)
		{
			VehicleUI->AddToViewport();

		} else {

			UE_LOG(LogMetalCars, Error, TEXT("Could not spawn vehicle UI widget."));

		}
	}
}

void AMetalCarsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ScoreboardAction)
		{
			EnhancedInputComponent->BindAction(
				ScoreboardAction,
				ETriggerEvent::Started,
				this,
				&AMetalCarsPlayerController::StartShowScoreboard
			);

			EnhancedInputComponent->BindAction(
				ScoreboardAction,
				ETriggerEvent::Completed,
				this,
				&AMetalCarsPlayerController::StopShowScoreboard
			);

			EnhancedInputComponent->BindAction(
				ScoreboardAction,
				ETriggerEvent::Canceled,
				this,
				&AMetalCarsPlayerController::StopShowScoreboard
			);
		}
	}
}

void AMetalCarsPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (!IsValid(VehicleUI))
	{
		return;
	}

	if (!IsValid(VehiclePawn))
	{
		VehiclePawn = Cast<AMetalCarsPawn>(GetPawn());
	}

	if (!IsValid(VehiclePawn))
	{
		return;
	}

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
		//-----------:)--------------------------
		if (AMetalCombatVehicle* CombatVehicle = Cast<AMetalCombatVehicle>(VehiclePawn))
		{
			if (UMetalHealthComponent* HealthComp = CombatVehicle->GetHealthComponent())
			{
				VehicleUI->UpdateHealth(
					HealthComp->GetCurrentHealth(),
					HealthComp->GetMaxHealth()
				);
			}

			const int32 MissileAmmo = CombatVehicle->GetCurrentMissileAmmo();
			TSubclassOf<AMetalProjectile> MissileClass = CombatVehicle->GetCurrentMissileClass();

			FString MissileName = TEXT("None");

			if (MissileClass)
			{
				MissileName = MissileClass->GetName();
			}

			VehicleUI->UpdateMissileInfo(MissileName, MissileAmmo);
		}
		if (AMetalCarsPlayerState* MetalPS = GetPlayerState<AMetalCarsPlayerState>())
		{
			VehicleUI->UpdateScore(
				MetalPS->GetKills(),
				MetalPS->GetDeaths(),
				MetalPS->GetCombatScore()
			);
		}
		if (AMetalCarsGameState* MetalGS = GetWorld() ? GetWorld()->GetGameState<AMetalCarsGameState>() : nullptr)
		{
			VehicleUI->UpdateMatchTime(MetalGS->GetTimeRemaining());

			VehicleUI->UpdateMatchResult(
				MetalGS->IsMatchFinished(),
				MetalGS->GetWinnerName()
			);
		}
	}
}

void AMetalCarsPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	VehiclePawn = Cast<AMetalCarsPawn>(InPawn);

	if (VehiclePawn)
	{
		//VehiclePawn->OnDestroyed.AddDynamic(this, &AMetalCarsPlayerController::OnPawnDestroyed);
	}
}

void AMetalCarsPlayerController::OnPawnDestroyed(AActor* DestroyedPawn)
{
	// find the player start
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		// spawn a vehicle at the player start
		const FTransform SpawnTransform = ActorList[0]->GetActorTransform();

		if (AMetalCarsPawn* RespawnedVehicle = GetWorld()->SpawnActor<AMetalCarsPawn>(VehiclePawnClass, SpawnTransform))
		{
			// possess the vehicle
			Possess(RespawnedVehicle);
		}
	}
}

bool AMetalCarsPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}


void AMetalCarsPlayerController::StartShowScoreboard(const FInputActionValue& Value)
{
	ShowScoreboard();
}

void AMetalCarsPlayerController::StopShowScoreboard(const FInputActionValue& Value)
{
	HideScoreboard();
}

void AMetalCarsPlayerController::ShowScoreboard()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (!ScoreboardWidget && ScoreboardWidgetClass)
	{
		ScoreboardWidget = CreateWidget<UMetalScoreboardWidget>(this, ScoreboardWidgetClass);
	}

	if (!ScoreboardWidget)
	{
		return;
	}

	ScoreboardWidget->RefreshScoreboard();

	if (!ScoreboardWidget->IsInViewport())
	{
		ScoreboardWidget->AddToViewport(50);
	}
}

void AMetalCarsPlayerController::HideScoreboard()
{
	if (ScoreboardWidget && ScoreboardWidget->IsInViewport())
	{
		ScoreboardWidget->RemoveFromParent();
	}
}