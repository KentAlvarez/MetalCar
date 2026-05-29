// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MetalHealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMetalHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMetalDeath, AController*, KillerController);

UCLASS(ClassGroup=(MetalCars), meta=(BlueprintSpawnableComponent))
class METALCARS_API UMetalHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMetalHealthComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Health")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Health")
	bool bIsDead = false;

	/**
	 * 1.0 = daño normal
	 * 0.75 = recibe 25% menos daño
	 * 1.25 = recibe 25% más daño
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float DamageMultiplier = 1.0f;

	UFUNCTION()
	void HandleOwnerTakeAnyDamage(
		AActor* DamagedActor,
		float Damage,
		const UDamageType* DamageType,
		AController* InstigatedBy,
		AActor* DamageCauser
	);

	UFUNCTION()
	void OnRep_Health();

	void BroadcastHealthChanged();

public:
	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnMetalHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnMetalDeath OnDeath;

	UFUNCTION(BlueprintCallable, Category="Health")
	void InitializeHealth(float NewMaxHealth, float NewDamageMultiplier);

	UFUNCTION(BlueprintCallable, Category="Health")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category="Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category="Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category="Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, Category="Health")
	bool IsDead() const { return bIsDead; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
