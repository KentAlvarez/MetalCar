// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MetalWeaponPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AMetalProjectile;
class AMetalCombatVehicle;

UCLASS()
class METALCARS_API AMetalWeaponPickup : public AActor
{
	GENERATED_BODY()

public:
	AMetalWeaponPickup();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
	TSubclassOf<AMetalProjectile> MissileClassToGive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
	int32 AmmoToGive = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
	float RespawnTime = 10.0f;

	UPROPERTY(ReplicatedUsing=OnRep_IsAvailable, BlueprintReadOnly, Category="Pickup")
	bool bIsAvailable = true;

	FTimerHandle RespawnTimerHandle;

	UFUNCTION()
	void OnPickupOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void GivePickupToVehicle(AMetalCombatVehicle* Vehicle);
	void SetPickupAvailable(bool bNewAvailable);
	void RespawnPickup();

	UFUNCTION()
	void OnRep_IsAvailable();

	UFUNCTION(BlueprintImplementableEvent, Category="Pickup")
	void BP_OnPickupStateChanged(bool bNewAvailable);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
