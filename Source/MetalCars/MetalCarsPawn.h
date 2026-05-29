// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "MetalCarsPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UChaosWheeledVehicleMovementComponent;
struct FInputActionValue;

/**
 *  Clase Pawn del vehiculo.
 *  Maneja la funcionalidad comun para todos los tipos de vehiculo,
 *  incluyendo la gestion de entradas y camaras.
 *  
 *  Las configuraciones especificas de cada vehiculo se manejan en subclases.
 */
UCLASS(abstract)
class AMetalCarsPawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	/** Brazo de resorte para la camara frontal */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FrontSpringArm;

	/** Componente de camara frontal */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FrontCamera;

	/** Brazo de resorte para la camara trasera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* BackSpringArm;

	/** Componente de camara trasera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* BackCamera;

	/** Puntero convertido al componente de movimiento Chaos Vehicle */
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

protected:

	/** Accion de direccion */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SteeringAction;

	/** Accion de aceleracion */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ThrottleAction;

	/** Accion de freno */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* BrakeAction;

	/** Accion de freno de mano */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* HandbrakeAction;

	/** Accion de mirar alrededor */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAroundAction;

	/** Accion de cambiar camara */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ToggleCameraAction;

	/** Accion de reiniciar vehiculo */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ResetVehicleAction;

	/** Registra que camara esta activa */
	bool bFrontCameraActive = false;

	/** Registra si el auto esta volcado. Si se mantiene verdadero durante dos comprobaciones, reinicia el vehiculo automaticamente */
	bool bPreviousFlipCheck = false;

	/** Tiempo entre comprobaciones automaticas de vuelco */
	UPROPERTY(EditAnywhere, Category="Flip Check", meta = (Units = "s"))
	float FlipCheckTime = 3.0f;

	/** Valor minimo del producto punto de la direccion superior del vehiculo para considerarlo todavia derecho */
	UPROPERTY(EditAnywhere, Category="Flip Check")
	float FlipCheckMinDot = -0.2f;

	/** Temporizador de comprobacion de vuelco */
	FTimerHandle FlipCheckTimer;

public:
	AMetalCarsPawn();

	// Inicio de la interfaz Pawn

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	// Fin de la interfaz Pawn

	// Inicio de la interfaz Actor

	/** Inicializacion */
	virtual void BeginPlay() override;

	/** Limpieza */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Actualizacion */
	virtual void Tick(float Delta) override;

	// Fin de la interfaz Actor

protected:

	/** Maneja la entrada de direccion */
	void Steering(const FInputActionValue& Value);

	/** Maneja la entrada de aceleracion */
	void Throttle(const FInputActionValue& Value);

	/** Maneja la entrada de freno */
	void Brake(const FInputActionValue& Value);

	/** Maneja las entradas de inicio/fin de freno */
	void StartBrake(const FInputActionValue& Value);
	void StopBrake(const FInputActionValue& Value);

	/** Maneja las entradas de inicio/fin del freno de mano */
	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);

	/** Maneja la entrada de mirar alrededor */
	void LookAround(const FInputActionValue& Value);

	/** Maneja la entrada de cambiar camara */
	void ToggleCamera(const FInputActionValue& Value);

	/** Maneja la entrada de reiniciar vehiculo */
	void ResetVehicle(const FInputActionValue& Value);

public:

	/** Maneja la entrada de direccion mediante acciones de entrada o interfaz movil */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSteering(float SteeringValue);

	/** Maneja la entrada de aceleracion mediante acciones de entrada o interfaz movil */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoThrottle(float ThrottleValue);

	/** Maneja la entrada de freno mediante acciones de entrada o interfaz movil */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoBrake(float BrakeValue);

	/** Maneja la entrada de inicio de freno mediante acciones de entrada o interfaz movil */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBrakeStart();

	/** Maneja la entrada de fin de freno mediante acciones de entrada o interfaz movil */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoBrakeStop();

	/** Maneja la entrada de inicio del freno de mano mediante acciones de entrada o interfaz movil */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoHandbrakeStart();

	/** Maneja la entrada de fin del freno de mano mediante acciones de entrada o interfaz movil */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoHandbrakeStop();

	/** Maneja la entrada de mirar mediante acciones de entrada o interfaz movil */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoLookAround(float YawDelta);

	/** Maneja la entrada de cambiar camara mediante acciones de entrada o interfaz movil */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoToggleCamera();

	/** Maneja la entrada de reiniciar vehiculo mediante acciones de entrada o interfaz movil */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoResetVehicle();

	//disparos infinitos
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* FirePrimaryAction;

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStartFirePrimary();
	
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStopFirePrimary();
	
	void StartFirePrimary(const FInputActionValue& Value);
	void StopFirePrimary(const FInputActionValue& Value);

	// disparos misiles
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* FireMissileAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* NextMissileAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* PreviousMissileAction;

	void FireMissile(const FInputActionValue& Value);
	void NextMissile(const FInputActionValue& Value);
	void PreviousMissile(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoFireMissile();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoNextMissile();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoPreviousMissile();


protected:

	/** Se llama cuando las luces de freno se encienden o apagan */
	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void BrakeLights(bool bBraking);

	/** Comprueba si el auto esta volcado y lo reinicia automaticamente */
	UFUNCTION()
	void FlippedCheck();

	

public:
	/** Devuelve el subobjeto del brazo de resorte frontal */
	FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
	/** Devuelve el subobjeto de la camara frontal */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FrontCamera; }
	/** Devuelve el subobjeto del brazo de resorte trasero */
	FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
	/** Devuelve el subobjeto de la camara trasera */
	FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
	/** Devuelve el subobjeto convertido de movimiento Chaos Vehicle */
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }
};
