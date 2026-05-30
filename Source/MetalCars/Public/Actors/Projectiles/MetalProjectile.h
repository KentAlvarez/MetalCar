// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "MetalProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class METALCARS_API AMetalProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMetalProjectile();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float Damage = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float LifeSeconds = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float ImpactImpulse = 70000.0f;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ImpactFX(FVector ImpactLocation, FRotator ImpactRotation);

	UFUNCTION(BlueprintImplementableEvent, Category="VFX")
	void BP_OnImpactFX(FVector ImpactLocation, FRotator ImpactRotation);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> TrailVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> ImpactVFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX")
	TObjectPtr<UNiagaraComponent> TrailVFXComponent;

	//------------------
	//Homing
	//------------Teledirigido

	// -------------------------
	// Homing
	// -------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Homing")
	bool bUseHoming = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Homing", meta=(EditCondition="bUseHoming"))
	float HomingSearchRadius = 6000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Homing", meta=(EditCondition="bUseHoming"))
	float HomingAccelerationMagnitude = 12000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Homing", meta=(EditCondition="bUseHoming"))
	float HomingActivationDelay = 0.20f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Homing", meta=(EditCondition="bUseHoming"))
	float HomingMaxAngleDegrees = 60.0f;

	FTimerHandle HomingActivationTimerHandle;

	void ActivateHoming();

	AActor* FindBestHomingTarget() const;

	bool IsValidHomingTarget(AActor* Candidate) const;
	
};
