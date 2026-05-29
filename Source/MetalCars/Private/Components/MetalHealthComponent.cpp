// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MetalHealthComponent.h"

#include "Net/UnrealNetwork.h"


UMetalHealthComponent::UMetalHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMetalHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentHealth = MaxHealth;
		bIsDead = false;

		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UMetalHealthComponent::HandleOwnerTakeAnyDamage);
	}

	BroadcastHealthChanged();
}

void UMetalHealthComponent::InitializeHealth(float NewMaxHealth, float NewDamageMultiplier)
{
	MaxHealth = FMath::Max(1.0f, NewMaxHealth);
	DamageMultiplier = FMath::Max(0.0f, NewDamageMultiplier);

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentHealth = MaxHealth;
		bIsDead = false;
	}

	BroadcastHealthChanged();
}

void UMetalHealthComponent::HandleOwnerTakeAnyDamage(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser
)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (bIsDead || Damage <= 0.0f)
	{
		return;
	}

	const float FinalDamage = Damage * DamageMultiplier;
	CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.0f, MaxHealth);

	BroadcastHealthChanged();

	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		OnDeath.Broadcast(InstigatedBy);
	}
}

void UMetalHealthComponent::Heal(float HealAmount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (bIsDead || HealAmount <= 0.0f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
	BroadcastHealthChanged();
}

float UMetalHealthComponent::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentHealth / MaxHealth;
}

void UMetalHealthComponent::OnRep_Health()
{
	BroadcastHealthChanged();
}

void UMetalHealthComponent::BroadcastHealthChanged()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UMetalHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMetalHealthComponent, MaxHealth);
	DOREPLIFETIME(UMetalHealthComponent, CurrentHealth);
	DOREPLIFETIME(UMetalHealthComponent, bIsDead);
}