// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Pickup/MetalWeaponPickup.h"

#include "Actors/Projectiles/MetalProjectile.h"
#include "Components/SphereComponent.h"
#include "MetalCar/Cars/MetalCombatVehicle.h"
#include "Net/UnrealNetwork.h"


AMetalWeaponPickup::AMetalWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);

	CollisionComponent->InitSphereRadius(90.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMetalWeaponPickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(
			this,
			&AMetalWeaponPickup::OnPickupOverlap
		);
	}

	OnRep_IsAvailable();
}

void AMetalWeaponPickup::OnPickupOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bIsAvailable)
	{
		return;
	}

	AMetalCombatVehicle* Vehicle = Cast<AMetalCombatVehicle>(OtherActor);

	if (!Vehicle)
	{
		return;
	}

	GivePickupToVehicle(Vehicle);
}

void AMetalWeaponPickup::GivePickupToVehicle(AMetalCombatVehicle* Vehicle)
{
	if (!Vehicle || !MissileClassToGive || AmmoToGive <= 0)
	{
		return;
	}

	Vehicle->AddMissileAmmo(MissileClassToGive, AmmoToGive);

	SetPickupAvailable(false);

	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&AMetalWeaponPickup::RespawnPickup,
		RespawnTime,
		false
	);
}

void AMetalWeaponPickup::SetPickupAvailable(bool bNewAvailable)
{
	bIsAvailable = bNewAvailable;

	CollisionComponent->SetCollisionEnabled(
		bIsAvailable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision
	);

	MeshComponent->SetVisibility(bIsAvailable, true);

	BP_OnPickupStateChanged(bIsAvailable);
}

void AMetalWeaponPickup::RespawnPickup()
{
	SetPickupAvailable(true);
}

void AMetalWeaponPickup::OnRep_IsAvailable()
{
	CollisionComponent->SetCollisionEnabled(
		bIsAvailable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision
	);

	MeshComponent->SetVisibility(bIsAvailable, true);

	BP_OnPickupStateChanged(bIsAvailable);
}

void AMetalWeaponPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMetalWeaponPickup, bIsAvailable);
}

