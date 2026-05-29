// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetalCarsPawn.h"
#include "MetalCarBase.generated.h"

UCLASS(Abstract)
class METALCARS_API AMetalCarBase : public AMetalCarsPawn
{
	GENERATED_BODY()

public:

	AMetalCarBase();

	UFUNCTION(Server, Unreliable)
	void Server_SetArcadeInput(float NewSteering, float NewThrottle, float NewBrake);

	float CurrentSteeringInput = 0.0f;
	float CurrentThrottleInput = 0.0f;
	float CurrentBrakeInput = 0.0f;

protected:
	

	UFUNCTION(BlueprintPure, Category = "Ground Check")
	bool CanUseArcadeGroundControl() const;
	
	/** Aceleracion arcade en kilometros por hora por segundo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade Vehicle|Motor", meta = (ClampMin = "0.0", DisplayName = "Aceleracion Km/h por segundo"))
	float AccelerationKmhPerSecond = 80.0f;

	/** Frenado arcade en kilometros por hora por segundo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade Vehicle|Motor", meta = (ClampMin = "0.0", DisplayName = "Frenado Km/h por segundo"))
	float BrakeKmhPerSecond = 140.0f;

	/** Velocidad maxima horizontal en kilometros por hora */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade Vehicle|Motor", meta = (ClampMin = "0.0", DisplayName = "Velocidad Maxima Km/h"))
	float MaxSpeedKmh = 180.0f;

	/** Velocidad de giro arcade en grados por segundo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade Vehicle|Steering", meta = (ClampMin = "0.0", DisplayName = "Giro Grados por segundo"))
	float TurnRateDegreesPerSecond = 140.0f;

	/** Velocidad minima usada para que el auto pueda doblar aun moviendose lento */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade Vehicle|Steering", meta = (ClampMin = "0.0", DisplayName = "Velocidad Minima para Girar Km/h"))
	float MinSteeringSpeedKmh = 25.0f;

	/** Que tanto la velocidad acompana la direccion del auto al doblar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade Vehicle|Steering", meta = (ClampMin = "0.0", ClampMax = "20.0", DisplayName = "Agarre de Giro"))
	float SteeringVelocityGrip = 6.0f;

	/** Distancia para comprobar si hay suelo debajo del auto */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade Vehicle|Ground Check", meta = (ClampMin = "0.0", DisplayName = "Distancia de Suelo"))
	float GroundCheckDistance = 100.0f;

	/** Minimo producto punto con el eje superior del mundo para permitir control */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade Vehicle|Ground Check", meta = (ClampMin = "-1.0", ClampMax = "1.0", DisplayName = "Minimo Derecho"))
	float MinUprightDotForControl = 0.25f;

	/** Radio visual de la rueda, usado para calcular cuanto gira al avanzar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade Vehicle|Wheel Visuals", meta = (ClampMin = "1.0", DisplayName = "Radio de Rueda cm"))
	float VisualWheelRadiusCm = 40.0f;

	/** Angulo maximo visual de giro para las ruedas delanteras */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade Vehicle|Wheel Visuals", meta = (ClampMin = "0.0", DisplayName = "Angulo Visual Maximo de Rueda"))
	float MaxVisualSteeringAngle = 35.0f;

	/** Giro acumulado visual de las ruedas en grados */
	UPROPERTY(BlueprintReadOnly, Category = "Arcade Vehicle|Wheel Visuals")
	float VisualWheelSpinDegrees = 0.0f;

	/** Angulo visual de las ruedas delanteras en grados */
	UPROPERTY(BlueprintReadOnly, Category = "Arcade Vehicle|Wheel Visuals")
	float VisualSteeringAngle = 0.0f;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float Delta) override;

	virtual void DoSteering(float SteeringValue) override;
	virtual void DoThrottle(float ThrottleValue) override;
	virtual void DoBrake(float BrakeValue) override;
	virtual void DoBrakeStop() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Arcade Vehicle|Wheel Visuals")
	void OnWheelVisualsUpdated(float WheelSpinDegrees, float SteeringAngle);

private:

	void ApplyArcadeVehicleSetup();
	void ApplyArcadeSteering(float Delta);
	void ApplyArcadeDrive(float Delta);
	void UpdateWheelVisuals(float Delta);
	void ClampToMaxSpeed() const;

	
};
