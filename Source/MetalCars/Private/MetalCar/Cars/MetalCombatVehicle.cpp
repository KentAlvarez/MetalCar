// Fill out your copyright notice in the Description page of Project Settings.


#include "MetalCar/Cars/MetalCombatVehicle.h"

#include "MetalCarsGameMode.h"
#include "Actors/Projectiles/MetalProjectile.h"
#include "Components/MetalHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


AMetalCombatVehicle::AMetalCombatVehicle()
{
	HealthComponent = CreateDefaultSubobject<UMetalHealthComponent>(TEXT("HealthComponent"));

	LeftPrimaryFirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeftPrimaryFirePoint"));
	LeftPrimaryFirePoint->SetupAttachment(GetMesh());
	LeftPrimaryFirePoint->SetRelativeLocation(FVector(220.0f, -55.0f, 80.0f));

	RightPrimaryFirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("RightPrimaryFirePoint"));
	RightPrimaryFirePoint->SetupAttachment(GetMesh());
	RightPrimaryFirePoint->SetRelativeLocation(FVector(220.0f, 55.0f, 80.0f));

	MissileFirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MissileFirePoint"));
	MissileFirePoint->SetupAttachment(GetMesh());
	MissileFirePoint->SetRelativeLocation(FVector(180.0f, 0.0f, 130.0f));
}

void AMetalCombatVehicle::BeginPlay()
{
	Super::BeginPlay();

	ApplyVehicleStats();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &AMetalCombatVehicle::HandleHealthChanged);
		HealthComponent->OnDeath.AddDynamic(this, &AMetalCombatVehicle::HandleDeath);
	}

	UpdateDamageStateFromHealth();

	if (HasAuthority())
	{
		AddMissileAmmo(TestMissileClass, 5);
	}
	
}

USceneComponent* AMetalCombatVehicle::GetNextPrimaryFirePoint()
{
	USceneComponent* SelectedPoint = bNextPrimaryShotUsesLeft
		? LeftPrimaryFirePoint
		: RightPrimaryFirePoint;

	bNextPrimaryShotUsesLeft = !bNextPrimaryShotUsesLeft;

	return SelectedPoint;
}

bool AMetalCombatVehicle::CanFirePrimary() const
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!PrimaryProjectileClass)
	{
		return false;
	}

	if (HealthComponent && HealthComponent->IsDead())
	{
		return false;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	return CurrentTime - LastPrimaryFireTime >= PrimaryFireCooldown;
}

void AMetalCombatVehicle::ApplyVehicleStats()
{
	MaxSpeedKmh = VehicleStats.MaxSpeedKmh;
	AccelerationKmhPerSecond = VehicleStats.AccelerationKmhPerSecond;
	BrakeKmhPerSecond = VehicleStats.BrakeKmhPerSecond;
	TurnRateDegreesPerSecond = VehicleStats.TurnRateDegreesPerSecond;
	SteeringVelocityGrip = VehicleStats.SteeringVelocityGrip;

	if (HealthComponent)
	{
		HealthComponent->InitializeHealth(
			VehicleStats.MaxHealth,
			VehicleStats.DamageMultiplier
		);
	}
}

void AMetalCombatVehicle::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	UpdateDamageStateFromHealth();
}

void AMetalCombatVehicle::HandleDeath(AController* KillerController)
{
	SetDamageState(EVehicleDamageState::Destroyed);

	BP_OnVehicleDeath(KillerController);

	if (!HasAuthority())
	{
		return;
	}

	// Cortamos disparo primario si estaba manteniendo click.
	GetWorldTimerManager().ClearTimer(PrimaryFireTimerHandle);

	if (GetMesh())
	{
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}

	if (AMetalCarsGameMode* GM = Cast<AMetalCarsGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->HandleVehicleDeath(this, KillerController);
	}
	else
	{
		// Fallback por si el GameMode no está bien seteado.
		SetLifeSpan(3.0f);
	}
}

void AMetalCombatVehicle::UpdateDamageStateFromHealth()
{
	SetDamageState(CalculateDamageState());
}

EVehicleDamageState AMetalCombatVehicle::CalculateDamageState() const
{
	if (!HealthComponent)
	{
		return EVehicleDamageState::Healthy;
	}

	const float HealthPercent = HealthComponent->GetHealthPercent();

	if (HealthPercent <= 0.0f)
	{
		return EVehicleDamageState::Destroyed;
	}

	if (HealthPercent <= 0.10f)
	{
		return EVehicleDamageState::CriticalDamage;
	}

	if (HealthPercent <= 0.40f)
	{
		return EVehicleDamageState::HeavyDamage;
	}

	if (HealthPercent <= 0.70f)
	{
		return EVehicleDamageState::LightDamage;
	}

	return EVehicleDamageState::Healthy;
}

void AMetalCombatVehicle::SetDamageState(EVehicleDamageState NewDamageState)
{
	if (DamageState == NewDamageState)
	{
		return;
	}

	DamageState = NewDamageState;

	// En el server OnRep no se ejecuta solo, así que llamamos manual.
	BP_OnDamageStateChanged(DamageState);
}

void AMetalCombatVehicle::OnRep_DamageState()
{
	BP_OnDamageStateChanged(DamageState);
}

void AMetalCombatVehicle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMetalCombatVehicle, DamageState);
	DOREPLIFETIME(AMetalCombatVehicle, MissileInventory);
	DOREPLIFETIME(AMetalCombatVehicle, CurrentMissileIndex);
}


void AMetalCombatVehicle::DoStartFirePrimary()
{
	bIsHoldingPrimaryFire = true;

	Server_StartPrimaryFire();
}

void AMetalCombatVehicle::DoStopFirePrimary()
{
	bIsHoldingPrimaryFire = false;

	Server_StopPrimaryFire();
}

void AMetalCombatVehicle::FirePrimaryInternal()
{
	Server_FirePrimaryShot();
}


void AMetalCombatVehicle::Server_StartPrimaryFire_Implementation()
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}

	bIsHoldingPrimaryFire = true;

	// Disparo inmediato en server.
	Server_FirePrimaryShot();

	GetWorldTimerManager().ClearTimer(PrimaryFireTimerHandle);

	GetWorldTimerManager().SetTimer(
		PrimaryFireTimerHandle,
		this,
		&AMetalCombatVehicle::Server_FirePrimaryShot,
		PrimaryFireCooldown,
		true
	);
}

void AMetalCombatVehicle::Server_StopPrimaryFire_Implementation()
{
	bIsHoldingPrimaryFire = false;

	GetWorldTimerManager().ClearTimer(PrimaryFireTimerHandle);
}


void AMetalCombatVehicle::Server_FirePrimaryShot_Implementation()
{
	if (!CanFirePrimary())
	{
		return;
	}

	LastPrimaryFireTime = GetWorld()->GetTimeSeconds();

	USceneComponent* FirePoint = GetNextPrimaryFirePoint();

	const FVector SpawnLocation = FirePoint
		? FirePoint->GetComponentLocation()
		: GetActorLocation() + GetActorForwardVector() * 220.0f + FVector(0.0f, 0.0f, 80.0f);

	const FRotator SpawnRotation = FirePoint
		? FirePoint->GetComponentRotation()
		: GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<AMetalProjectile>(
		PrimaryProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);
}

void AMetalCombatVehicle::OnRep_MissileInventory()
{
	BP_OnMissileInventoryChanged();
}

void AMetalCombatVehicle::OnRep_CurrentMissileIndex()
{
	BP_OnMissileInventoryChanged();
}

void AMetalCombatVehicle::AddMissileAmmo(TSubclassOf<AMetalProjectile> MissileClass, int32 AmmoAmount)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!MissileClass || AmmoAmount <= 0)
	{
		return;
	}

	for (FMetalMissileInventoryEntry& Entry : MissileInventory)
	{
		if (Entry.MissileClass == MissileClass)
		{
			Entry.Ammo += AmmoAmount;
			BP_OnMissileInventoryChanged();
			return;
		}
	}

	FMetalMissileInventoryEntry NewEntry;
	NewEntry.MissileClass = MissileClass;
	NewEntry.Ammo = AmmoAmount;

	MissileInventory.Add(NewEntry);

	if (CurrentMissileIndex == INDEX_NONE)
	{
		CurrentMissileIndex = MissileInventory.Num() - 1;
	}

	BP_OnMissileInventoryChanged();
}

TSubclassOf<AMetalProjectile> AMetalCombatVehicle::GetCurrentMissileClass() const
{
	if (!MissileInventory.IsValidIndex(CurrentMissileIndex))
	{
		return nullptr;
	}

	return MissileInventory[CurrentMissileIndex].MissileClass;
}

int32 AMetalCombatVehicle::GetCurrentMissileAmmo() const
{
	if (!MissileInventory.IsValidIndex(CurrentMissileIndex))
	{
		return 0;
	}

	return MissileInventory[CurrentMissileIndex].Ammo;
}

void AMetalCombatVehicle::DoFireMissile()
{
	Server_FireMissile();
}

bool AMetalCombatVehicle::CanFireMissile() const
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!GetCurrentMissileClass())
	{
		return false;
	}

	if (GetCurrentMissileAmmo() <= 0)
	{
		return false;
	}

	if (HealthComponent && HealthComponent->IsDead())
	{
		return false;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	return CurrentTime - LastMissileFireTime >= MissileFireCooldown;
}

void AMetalCombatVehicle::Server_FireMissile_Implementation()
{
	if (!CanFireMissile())
	{
		return;
	}

	LastMissileFireTime = GetWorld()->GetTimeSeconds();

	FMetalMissileInventoryEntry& CurrentEntry = MissileInventory[CurrentMissileIndex];

	const FVector SpawnLocation = MissileFirePoint
		? MissileFirePoint->GetComponentLocation()
		: GetActorLocation() + GetActorForwardVector() * 180.0f + FVector(0.0f, 0.0f, 130.0f);

	const FRotator SpawnRotation = MissileFirePoint
		? MissileFirePoint->GetComponentRotation()
		: GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<AMetalProjectile>(
		CurrentEntry.MissileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	CurrentEntry.Ammo--;

	if (CurrentEntry.Ammo <= 0)
	{
		MissileInventory.RemoveAt(CurrentMissileIndex);

		if (MissileInventory.Num() <= 0)
		{
			CurrentMissileIndex = INDEX_NONE;
		}
		else
		{
			CurrentMissileIndex = FMath::Clamp(CurrentMissileIndex, 0, MissileInventory.Num() - 1);
		}
	}

	BP_OnMissileInventoryChanged();
}

void AMetalCombatVehicle::SelectNextMissile()
{
	Server_SelectNextMissile();
}

void AMetalCombatVehicle::SelectPreviousMissile()
{
	Server_SelectPreviousMissile();
}

void AMetalCombatVehicle::Server_SelectNextMissile_Implementation()
{
	if (MissileInventory.Num() <= 0)
	{
		CurrentMissileIndex = INDEX_NONE;
		BP_OnMissileInventoryChanged();
		return;
	}

	if (CurrentMissileIndex == INDEX_NONE)
	{
		CurrentMissileIndex = 0;
	}
	else
	{
		CurrentMissileIndex = (CurrentMissileIndex + 1) % MissileInventory.Num();
	}

	BP_OnMissileInventoryChanged();
}

void AMetalCombatVehicle::Server_SelectPreviousMissile_Implementation()
{
	if (MissileInventory.Num() <= 0)
	{
		CurrentMissileIndex = INDEX_NONE;
		BP_OnMissileInventoryChanged();
		return;
	}

	if (CurrentMissileIndex == INDEX_NONE)
	{
		CurrentMissileIndex = 0;
	}
	else
	{
		CurrentMissileIndex--;

		if (CurrentMissileIndex < 0)
		{
			CurrentMissileIndex = MissileInventory.Num() - 1;
		}
	}

	BP_OnMissileInventoryChanged();
}


void AMetalCarsPawn::DoFireMissile()
{
}

void AMetalCarsPawn::DoNextMissile()
{
}

void AMetalCarsPawn::DoPreviousMissile()
{
}

void AMetalCombatVehicle::DoNextMissile()
{
	SelectNextMissile();
}

void AMetalCombatVehicle::DoPreviousMissile()
{
	SelectPreviousMissile();
}