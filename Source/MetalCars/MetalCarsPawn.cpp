// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetalCarsPawn.h"
#include "MetalCarsWheelFront.h"
#include "MetalCarsWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "MetalCars.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

AMetalCarsPawn::AMetalCarsPawn()
{
	// Construye el brazo de la camara frontal
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// Construye el brazo de la camara trasera
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configura la malla del auto
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// Obtiene el componente de movimiento Chaos Wheeled
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

}

void AMetalCarsPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Direccion
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AMetalCarsPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AMetalCarsPawn::Steering);

		// Aceleracion
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AMetalCarsPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AMetalCarsPawn::Throttle);

		// Freno
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AMetalCarsPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &AMetalCarsPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AMetalCarsPawn::StopBrake);

		// Freno de mano
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AMetalCarsPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AMetalCarsPawn::StopHandbrake);

		// Mirar alrededor
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &AMetalCarsPawn::LookAround);

		// Cambiar camara
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &AMetalCarsPawn::ToggleCamera);

		// Reiniciar el vehiculo
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &AMetalCarsPawn::ResetVehicle);

		//Disparar
		EnhancedInputComponent->BindAction(FirePrimaryAction, ETriggerEvent::Started, this, &AMetalCarsPawn::StartFirePrimary);
		EnhancedInputComponent->BindAction(FirePrimaryAction, ETriggerEvent::Completed, this, &AMetalCarsPawn::StopFirePrimary);
		EnhancedInputComponent->BindAction(FirePrimaryAction,ETriggerEvent::Canceled,this,&AMetalCarsPawn::StopFirePrimary);

		// disparar misiles
		EnhancedInputComponent->BindAction(FireMissileAction,ETriggerEvent::Started,this,&AMetalCarsPawn::FireMissile);
		EnhancedInputComponent->BindAction(NextMissileAction,ETriggerEvent::Started,this,&AMetalCarsPawn::NextMissile);
		EnhancedInputComponent->BindAction(PreviousMissileAction,ETriggerEvent::Started,this,&AMetalCarsPawn::PreviousMissile);
		
	}
	else
	{
		UE_LOG(LogMetalCars, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMetalCarsPawn::BeginPlay()
{
	Super::BeginPlay();

	// Configura el temporizador de comprobacion de vuelco
	GetWorld()->GetTimerManager().SetTimer(FlipCheckTimer, this, &AMetalCarsPawn::FlippedCheck, FlipCheckTime, true);
}

void AMetalCarsPawn::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	// Limpia el temporizador de comprobacion de vuelco
	GetWorld()->GetTimerManager().ClearTimer(FlipCheckTimer);

	Super::EndPlay(EndPlayReason);
}

void AMetalCarsPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// Agrega amortiguacion angular si el vehiculo esta en el aire
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// Realinea el yaw de la camara para mirar hacia adelante
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

void AMetalCarsPawn::Steering(const FInputActionValue& Value)
{
	// Redirige la entrada
	DoSteering(Value.Get<float>());
}

void AMetalCarsPawn::Throttle(const FInputActionValue& Value)
{
	// Redirige la entrada
	DoThrottle(Value.Get<float>());
}

void AMetalCarsPawn::Brake(const FInputActionValue& Value)
{
	// Redirige la entrada
	DoBrake(Value.Get<float>());
}

void AMetalCarsPawn::StartBrake(const FInputActionValue& Value)
{
	// Redirige la entrada
	DoBrakeStart();
}

void AMetalCarsPawn::StopBrake(const FInputActionValue& Value)
{
	// Redirige la entrada
	DoBrakeStop();
}

void AMetalCarsPawn::StartHandbrake(const FInputActionValue& Value)
{
	// Redirige la entrada
	DoHandbrakeStart();
}

void AMetalCarsPawn::StopHandbrake(const FInputActionValue& Value)
{
	// Redirige la entrada
	DoHandbrakeStop();
}

void AMetalCarsPawn::LookAround(const FInputActionValue& Value)
{
	// Redirige la entrada
	DoLookAround(Value.Get<float>());
}

void AMetalCarsPawn::ToggleCamera(const FInputActionValue& Value)
{
	// Redirige la entrada
	DoToggleCamera();
}

void AMetalCarsPawn::ResetVehicle(const FInputActionValue& Value)
{
	// Redirige la entrada
	DoResetVehicle();
}

void AMetalCarsPawn::DoSteering(float SteeringValue)
{
	// Agrega la entrada
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void AMetalCarsPawn::DoThrottle(float ThrottleValue)
{
	// Agrega la entrada
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);

	// Reinicia la entrada de freno
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void AMetalCarsPawn::DoBrake(float BrakeValue)
{
	// Agrega la entrada
	ChaosVehicleMovement->SetBrakeInput(BrakeValue);

	// Reinicia la entrada de aceleracion
	ChaosVehicleMovement->SetThrottleInput(0.0f);
}

void AMetalCarsPawn::DoBrakeStart()
{
	// Llama al evento de Blueprint para las luces de freno
	BrakeLights(true);
}

void AMetalCarsPawn::DoBrakeStop()
{
	// Llama al evento de Blueprint para las luces de freno
	BrakeLights(false);

	// Reinicia la entrada de freno a cero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void AMetalCarsPawn::DoHandbrakeStart()
{
	// Agrega la entrada
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// Llama al evento de Blueprint para las luces de freno
	BrakeLights(true);
}

void AMetalCarsPawn::DoHandbrakeStop()
{
	// Agrega la entrada
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// Llama al evento de Blueprint para las luces de freno
	BrakeLights(false);
}

void AMetalCarsPawn::DoLookAround(float YawDelta)
{
	// Rota el brazo de resorte
	BackSpringArm->AddLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}

void AMetalCarsPawn::DoToggleCamera()
{
	// Alterna la marca de camara activa
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void AMetalCarsPawn::DoResetVehicle()
{
	// Reinicia a una ubicacion ligeramente por encima de la actual
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// Reinicia usando nuestro yaw. Ignora pitch y roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;

	// Teletransporta el actor al punto de reinicio y reinicia la fisica
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

void AMetalCarsPawn::FlippedCheck()
{
	// Comprueba la diferencia de angulo entre el vector superior de la malla y el superior del mundo
	const float UpDot = FVector::DotProduct(FVector::UpVector, GetMesh()->GetUpVector());

	if (UpDot < FlipCheckMinDot)
	{
		// Es la segunda vez que comprobamos que el vehiculo sigue volcado?
		if (bPreviousFlipCheck)
		{
			// Reinicia el vehiculo para dejarlo derecho
			DoResetVehicle();
		}
		
		// Activa la marca de comprobacion de vuelco para que la siguiente comprobacion reinicie el auto
		bPreviousFlipCheck = true;

	} else {

		// Estamos derechos. Reinicia la marca de comprobacion de vuelco
		bPreviousFlipCheck = false;
	}
}


void AMetalCarsPawn::StartFirePrimary(const FInputActionValue& Value)
{
	DoStartFirePrimary();
}

void AMetalCarsPawn::StopFirePrimary(const FInputActionValue& Value)
{
	DoStopFirePrimary();
}

void AMetalCarsPawn::DoStartFirePrimary()
{
	// Vacío en base. Lo implementa el vehículo de combate.
}

void AMetalCarsPawn::DoStopFirePrimary()
{
	// Vacío en base. Lo implementa el vehículo de combate.
}
void AMetalCarsPawn::FireMissile(const FInputActionValue& Value)
{
	DoFireMissile();
}

void AMetalCarsPawn::NextMissile(const FInputActionValue& Value)
{
	DoNextMissile();
}

void AMetalCarsPawn::PreviousMissile(const FInputActionValue& Value)
{
	DoPreviousMissile();
}
#undef LOCTEXT_NAMESPACE
