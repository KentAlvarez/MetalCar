// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetalCarBase.h"
#include "Utils/MetalDamageTypes.h"
#include "MetalCombatVehicle.generated.h"


class UMetalHealthComponent;
class AMetalProjectile;
class USceneComponent;

USTRUCT(BlueprintType)
struct FMetalVehicleStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats|Health")
	float MaxHealth = 100.0f;

	/**
	 * 1.0 = normal
	 * 0.75 = tanque, recibe menos daño
	 * 1.25 = liviano, recibe más daño
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats|Health")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats|Movement")
	float MaxSpeedKmh = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats|Movement")
	float AccelerationKmhPerSecond = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats|Movement")
	float BrakeKmhPerSecond = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats|Movement")
	float TurnRateDegreesPerSecond = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats|Movement")
	float SteeringVelocityGrip = 6.0f;
};

USTRUCT(BlueprintType)
struct FMetalMissileInventoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Missile")
	TSubclassOf<AMetalProjectile> MissileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Missile")
	int32 Ammo = 0;
};

UCLASS()
class METALCARS_API AMetalCombatVehicle : public AMetalCarBase
{
	GENERATED_BODY()

public:
	AMetalCombatVehicle();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UMetalHealthComponent> HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Stats")
	FMetalVehicleStats VehicleStats;

	UPROPERTY(ReplicatedUsing=OnRep_DamageState, BlueprintReadOnly, Category="Vehicle|Damage")
	EVehicleDamageState DamageState = EVehicleDamageState::Healthy;

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleDeath(AController* KillerController);

	UFUNCTION()
	void OnRep_DamageState();

	void ApplyVehicleStats();
	void UpdateDamageStateFromHealth();
	EVehicleDamageState CalculateDamageState() const;
	void SetDamageState(EVehicleDamageState NewDamageState);

	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle|Damage")
	void BP_OnDamageStateChanged(EVehicleDamageState NewDamageState);

	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle|Damage")
	void BP_OnVehicleDeath(AController* KillerController);

	
	//virtual void DoFireMissile() override;
	virtual void DoNextMissile() override;
	virtual void DoPreviousMissile() override;
public:
	UFUNCTION(BlueprintCallable, Category="Vehicle|Health")
	UMetalHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintCallable, Category="Vehicle|Damage")
	EVehicleDamageState GetDamageState() const { return DamageState; }

	UFUNCTION(BlueprintCallable, Category="Combat|Missile")
	void AddMissileAmmo(TSubclassOf<AMetalProjectile> MissileClass, int32 AmmoAmount);

	UFUNCTION(BlueprintCallable, Category="Combat|Missile")
	TSubclassOf<AMetalProjectile> GetCurrentMissileClass() const;

	UFUNCTION(BlueprintCallable, Category="Combat|Missile")
	int32 GetCurrentMissileAmmo() const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TSubclassOf<AMetalProjectile> PrimaryProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> LeftPrimaryFirePoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> RightPrimaryFirePoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	float FireCooldown = 0.45f;

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	float LastFireTime = -999.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Primary")
	float PrimaryFireCooldown = 0.18f;

	UPROPERTY(BlueprintReadOnly, Category="Combat|Primary")
	bool bIsHoldingPrimaryFire = false;

	UPROPERTY(BlueprintReadOnly, Category="Combat|Primary")
	float LastPrimaryFireTime = -999.0f;

	UPROPERTY(BlueprintReadOnly, Category="Combat|Primary")
	bool bNextPrimaryShotUsesLeft = true;

	FTimerHandle PrimaryFireTimerHandle;

	virtual void DoStartFirePrimary() override;
	virtual void DoStopFirePrimary() override;

	void FirePrimaryInternal();

	UFUNCTION(Server, Reliable)
	void Server_StartPrimaryFire();

	UFUNCTION(Server, Reliable)
	void Server_StopPrimaryFire();

	UFUNCTION(Server, Reliable)
	void Server_FirePrimaryShot();

	bool CanFirePrimary() const;

	USceneComponent* GetNextPrimaryFirePoint();

	//Misiles

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> MissileFirePoint;

	UPROPERTY(ReplicatedUsing=OnRep_MissileInventory, BlueprintReadOnly, Category="Combat|Missile")
	TArray<FMetalMissileInventoryEntry> MissileInventory;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentMissileIndex, BlueprintReadOnly, Category="Combat|Missile")
	int32 CurrentMissileIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Missile")
	float MissileFireCooldown = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category="Combat|Missile")
	float LastMissileFireTime = -999.0f;

	UFUNCTION()
	void OnRep_MissileInventory();

	UFUNCTION()
	void OnRep_CurrentMissileIndex();

	UFUNCTION(BlueprintImplementableEvent, Category="Combat|Missile")
	void BP_OnMissileInventoryChanged();

	virtual void DoFireMissile() override;

	UFUNCTION(Server, Reliable)
	void Server_FireMissile();

	

	UFUNCTION(BlueprintCallable, Category="Combat|Missile")
	void SelectNextMissile();

	UFUNCTION(BlueprintCallable, Category="Combat|Missile")
	void SelectPreviousMissile();

	UFUNCTION(Server, Reliable)
	void Server_SelectNextMissile();

	UFUNCTION(Server, Reliable)
	void Server_SelectPreviousMissile();

	bool CanFireMissile() const;



	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Missile|Debug")
	TSubclassOf<AMetalProjectile> TestMissileClass;
};
