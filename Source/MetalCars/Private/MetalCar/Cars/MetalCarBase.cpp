// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/MetalCar/Cars/MetalCarBase.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "MetalCarsSportsWheelFront.h"
#include "MetalCarsSportsWheelRear.h"


AMetalCarBase::AMetalCarBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GetChaosVehicleMovement()->ChassisHeight = 144.0f;
	GetChaosVehicleMovement()->DragCoefficient = 0.31f;

	// Configura las ruedas
	GetChaosVehicleMovement()->bLegacyWheelFrictionPosition = true;
	GetChaosVehicleMovement()->WheelSetups.SetNum(4);

	GetChaosVehicleMovement()->WheelSetups[0].WheelClass = UMetalCarsSportsWheelFront::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[0].BoneName = FName("Phys_Wheel_FL");
	GetChaosVehicleMovement()->WheelSetups[0].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	GetChaosVehicleMovement()->WheelSetups[1].WheelClass = UMetalCarsSportsWheelFront::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[1].BoneName = FName("Phys_Wheel_FR");
	GetChaosVehicleMovement()->WheelSetups[1].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	GetChaosVehicleMovement()->WheelSetups[2].WheelClass = UMetalCarsSportsWheelRear::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[2].BoneName = FName("Phys_Wheel_BL");
	GetChaosVehicleMovement()->WheelSetups[2].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	GetChaosVehicleMovement()->WheelSetups[3].WheelClass = UMetalCarsSportsWheelRear::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[3].BoneName = FName("Phys_Wheel_BR");
	GetChaosVehicleMovement()->WheelSetups[3].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	// Configura el motor
	ApplyArcadeVehicleSetup();
	
	// Configura la direccion
	// NOTA: revisar el asset Blueprint para la curva de direccion
	GetChaosVehicleMovement()->SteeringSetup.SteeringType = ESteeringType::Ackermann;
	GetChaosVehicleMovement()->SteeringSetup.AngleRatio = 0.7f;
}

void AMetalCarBase::Server_SetArcadeInput_Implementation(float NewSteering, float NewThrottle, float NewBrake)
{
	CurrentSteeringInput = FMath::Clamp(NewSteering, -1.0f, 1.0f);
	CurrentThrottleInput = FMath::Clamp(NewThrottle, -1.0f, 1.0f);
	CurrentBrakeInput = FMath::Clamp(NewBrake, 0.0f, 1.0f);
}


void AMetalCarBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyArcadeVehicleSetup();
}

void AMetalCarBase::BeginPlay()
{
	Super::BeginPlay();

	ApplyArcadeVehicleSetup();
}

void AMetalCarBase::Tick(float Delta)
{
	Super::Tick(Delta);

	if (HasAuthority())
	{
		ApplyArcadeSteering(Delta);
		ApplyArcadeDrive(Delta);
		ClampToMaxSpeed();
	}

	UpdateWheelVisuals(Delta);
}

void AMetalCarBase::DoSteering(float SteeringValue)
{
	CurrentSteeringInput = FMath::Clamp(SteeringValue, -1.0f, 1.0f);

	if (!HasAuthority())
	{
		Server_SetArcadeInput(CurrentSteeringInput, CurrentThrottleInput, CurrentBrakeInput);
	}
}

void AMetalCarBase::DoThrottle(float ThrottleValue)
{
	CurrentThrottleInput = FMath::Clamp(ThrottleValue, -1.0f, 1.0f);

	if (FMath::Abs(CurrentThrottleInput) > 0.01f)
	{
		CurrentBrakeInput = 0.0f;
	}

	if (!HasAuthority())
	{
		Server_SetArcadeInput(CurrentSteeringInput, CurrentThrottleInput, CurrentBrakeInput);
	}
}

void AMetalCarBase::DoBrake(float BrakeValue)
{
	CurrentBrakeInput = FMath::Clamp(BrakeValue, 0.0f, 1.0f);

	if (CurrentBrakeInput > 0.01f)
	{
		CurrentThrottleInput = 0.0f;
	}

	if (!HasAuthority())
	{
		Server_SetArcadeInput(CurrentSteeringInput, CurrentThrottleInput, CurrentBrakeInput);
	}
}

void AMetalCarBase::DoBrakeStop()
{
	CurrentBrakeInput = 0.0f;

	if (!HasAuthority())
	{
		Server_SetArcadeInput(CurrentSteeringInput, CurrentThrottleInput, CurrentBrakeInput);
	}
}

void AMetalCarBase::ApplyArcadeVehicleSetup()
{
	UChaosWheeledVehicleMovementComponent* VehicleMovement = GetChaosVehicleMovement().Get();
	if (!VehicleMovement)
	{
		return;
	}

	// Motor arcade: el avance lo controla ApplyArcadeDrive, sin cambios de marcha.
	VehicleMovement->EngineSetup.MaxTorque = 0.0f;
	VehicleMovement->EngineSetup.MaxRPM = 1.0f;
	VehicleMovement->EngineSetup.EngineIdleRPM = 900.0f;
	VehicleMovement->EngineSetup.EngineBrakeEffect = 0.2f;
	VehicleMovement->EngineSetup.EngineRevUpMOI = 5.0f;
	VehicleMovement->EngineSetup.EngineRevDownRate = 600.0f;

	VehicleMovement->TransmissionSetup.bUseAutomaticGears = false;
	VehicleMovement->TransmissionSetup.bUseAutoReverse = true;
	VehicleMovement->TransmissionSetup.FinalRatio = 1.0f;
	VehicleMovement->TransmissionSetup.ChangeUpRPM = 1.0f;
	VehicleMovement->TransmissionSetup.ChangeDownRPM = 0.0f;
	VehicleMovement->TransmissionSetup.GearChangeTime = 0.0f;
	VehicleMovement->TransmissionSetup.TransmissionEfficiency = 1.0f;

	VehicleMovement->TransmissionSetup.ForwardGearRatios.SetNum(1);
	VehicleMovement->TransmissionSetup.ForwardGearRatios[0] = 1.0f;

	VehicleMovement->TransmissionSetup.ReverseGearRatios.SetNum(1);
	VehicleMovement->TransmissionSetup.ReverseGearRatios[0] = 1.0f;
}

bool AMetalCarBase::CanUseArcadeGroundControl() const
{
	if (!GetWorld() || !GetMesh())
	{
		return false;
	}

	const float UprightDot = FVector::DotProduct(FVector::UpVector, GetMesh()->GetUpVector());
	if (UprightDot < MinUprightDotForControl)
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MetalCarGroundCheck), false, this);
	const FVector TraceStart = GetActorLocation();
	const FVector TraceEnd = TraceStart - FVector::UpVector * GroundCheckDistance;

	return GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
}

void AMetalCarBase::ApplyArcadeSteering(float Delta)
{
	if (!GetMesh() || !CanUseArcadeGroundControl() || FMath::IsNearlyZero(CurrentSteeringInput))
	{
		return;
	}

	const FVector CurrentVelocity = GetMesh()->GetPhysicsLinearVelocity();
	FVector HorizontalVelocity(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);
	const float SpeedCmS = HorizontalVelocity.Size();
	const float MinSteeringSpeedCmS = MinSteeringSpeedKmh * 100000.0f / 3600.0f;
	const float SteeringSpeedFactor = FMath::Clamp(FMath::Max(SpeedCmS, MinSteeringSpeedCmS) / MinSteeringSpeedCmS, 0.0f, 1.0f);

	const float YawDelta = CurrentSteeringInput * TurnRateDegreesPerSecond * SteeringSpeedFactor * Delta;
	const FRotator NewRotation = GetActorRotation() + FRotator(0.0f, YawDelta, 0.0f);
	SetActorRotation(NewRotation, ETeleportType::TeleportPhysics);

	if (SpeedCmS > KINDA_SMALL_NUMBER && SteeringVelocityGrip > 0.0f)
	{
		const FVector CurrentDirection = HorizontalVelocity.GetSafeNormal();
		const FVector TargetDirection = GetActorForwardVector().GetSafeNormal2D();
		const float BlendAlpha = FMath::Clamp(SteeringVelocityGrip * Delta, 0.0f, 1.0f);
		const FVector NewDirection = FMath::Lerp(CurrentDirection, TargetDirection, BlendAlpha).GetSafeNormal();

		HorizontalVelocity = NewDirection * SpeedCmS;
		GetMesh()->SetPhysicsLinearVelocity(FVector(HorizontalVelocity.X, HorizontalVelocity.Y, CurrentVelocity.Z));
	}
}

void AMetalCarBase::ApplyArcadeDrive(float Delta)
{
	if (!GetMesh() || !CanUseArcadeGroundControl())
	{
		return;
	}

	FVector CurrentVelocity = GetMesh()->GetPhysicsLinearVelocity();
	FVector HorizontalVelocity(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);

	const FVector ForwardDirection = GetActorForwardVector().GetSafeNormal2D();
	const float AccelerationCmS2 = AccelerationKmhPerSecond * 100000.0f / 3600.0f;
	HorizontalVelocity += ForwardDirection * CurrentThrottleInput * AccelerationCmS2 * Delta;

	if (CurrentBrakeInput > 0.0f)
	{
		const float BrakeCmS2 = BrakeKmhPerSecond * 100000.0f / 3600.0f;
		const float NewSpeed = FMath::Max(0.0f, HorizontalVelocity.Size() - BrakeCmS2 * CurrentBrakeInput * Delta);
		HorizontalVelocity = HorizontalVelocity.GetSafeNormal() * NewSpeed;
	}

	GetMesh()->SetPhysicsLinearVelocity(FVector(HorizontalVelocity.X, HorizontalVelocity.Y, CurrentVelocity.Z));
}

void AMetalCarBase::UpdateWheelVisuals(float Delta)
{
	if (!GetMesh() || VisualWheelRadiusCm <= 0.0f)
	{
		return;
	}

	const FVector CurrentVelocity = GetMesh()->GetPhysicsLinearVelocity();
	const FVector HorizontalVelocity(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);
	const float ForwardSpeedCmS = FVector::DotProduct(HorizontalVelocity, GetActorForwardVector());
	const float WheelCircumferenceCm = 2.0f * UE_PI * VisualWheelRadiusCm;
	const float SpinDeltaDegrees = (ForwardSpeedCmS * Delta / WheelCircumferenceCm) * 360.0f;

	VisualWheelSpinDegrees = FMath::Fmod(VisualWheelSpinDegrees + SpinDeltaDegrees, 360.0f);
	VisualSteeringAngle = CurrentSteeringInput * MaxVisualSteeringAngle;

	OnWheelVisualsUpdated(VisualWheelSpinDegrees, VisualSteeringAngle);
}

void AMetalCarBase::ClampToMaxSpeed() const
{
	if (MaxSpeedKmh <= 0.0f || !GetMesh())
	{
		return;
	}

	const float MaxSpeedCmS = MaxSpeedKmh * 100000.0f / 3600.0f;
	const FVector CurrentVelocity = GetMesh()->GetPhysicsLinearVelocity();
	FVector HorizontalVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);

	if (HorizontalVelocity.SizeSquared() <= FMath::Square(MaxSpeedCmS))
	{
		return;
	}

	HorizontalVelocity = HorizontalVelocity.GetSafeNormal() * MaxSpeedCmS;
	GetMesh()->SetPhysicsLinearVelocity(FVector(HorizontalVelocity.X, HorizontalVelocity.Y, CurrentVelocity.Z));
}

