// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/MetalProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "MetalCar/Cars/MetalCombatVehicle.h"


AMetalProjectile::AMetalProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);

	CollisionComponent->InitSphereRadius(18.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetNotifyRigidBodyCollision(true);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 3500.0f;
	ProjectileMovement->MaxSpeed = 3500.0f;
	ProjectileMovement->ProjectileGravityScale = 0.05f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	PrimaryActorTick.bCanEverTick = true;

	TrailVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailVFXComponent"));
	TrailVFXComponent->SetupAttachment(RootComponent);
	TrailVFXComponent->bAutoActivate = false;
}

void AMetalProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (TrailVFXComponent && TrailVFX)
	{
		TrailVFXComponent->SetAsset(TrailVFX);
		TrailVFXComponent->Activate(true);
	}

	if (HasAuthority())
	{
		SetLifeSpan(LifeSeconds);
		CollisionComponent->OnComponentHit.AddDynamic(this, &AMetalProjectile::OnProjectileHit);

		if (bUseHoming)
		{
			GetWorldTimerManager().SetTimer(
				HomingActivationTimerHandle,
				this,
				&AMetalProjectile::ActivateHoming,
				HomingActivationDelay,
				false
			);
		}
	}
}

void AMetalProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	AController* InstigatorController = GetInstigatorController();

	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		InstigatorController,
		this,
		UDamageType::StaticClass()
	);

	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		const FVector ImpulseDirection = GetActorForwardVector();

		OtherComp->AddImpulseAtLocation(
			ImpulseDirection * ImpactImpulse,
			Hit.ImpactPoint
		);
	}

	const FRotator ImpactRotation = Hit.ImpactNormal.Rotation();

	// El server avisa a todos los clientes que spawneen chispas.
	Multicast_ImpactFX(Hit.ImpactPoint, ImpactRotation);

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	if (MeshComponent)
	{
		MeshComponent->SetVisibility(false, true);
	}

	if (TrailVFXComponent)
	{
		TrailVFXComponent->Deactivate();
	}

	// Le damos un pequeño margen para que el multicast llegue y el FX se vea.
	SetLifeSpan(0.35f);

	Destroy();
}

void AMetalProjectile::Multicast_ImpactFX_Implementation(FVector ImpactLocation, FRotator ImpactRotation)
{
	// VFX directo desde C++
	if (ImpactVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactVFX,
			ImpactLocation,
			ImpactRotation
		);
	}

	// Extra opcional en Blueprint: sonido, decal, camera shake, etc.
	BP_OnImpactFX(ImpactLocation, ImpactRotation);
}

void AMetalProjectile::ActivateHoming()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ProjectileMovement)
	{
		return;
	}

	AActor* Target = FindBestHomingTarget();

	if (!Target)
	{
		return;
	}

	USceneComponent* TargetComponent = Target->GetRootComponent();

	if (!TargetComponent)
	{
		return;
	}

	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingTargetComponent = TargetComponent;
	ProjectileMovement->HomingAccelerationMagnitude = HomingAccelerationMagnitude;
}

AActor* AMetalProjectile::FindBestHomingTarget() const
{
	UWorld* World = GetWorld();

	if (!World)
	{
		return nullptr;
	}

	TArray<AActor*> Vehicles;
	UGameplayStatics::GetAllActorsOfClass(World, AMetalCombatVehicle::StaticClass(), Vehicles);

	AActor* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	const FVector MyLocation = GetActorLocation();
	const FVector MyForward = GetActorForwardVector();

	for (AActor* Candidate : Vehicles)
	{
		if (!IsValidHomingTarget(Candidate))
		{
			continue;
		}

		const FVector ToTarget = Candidate->GetActorLocation() - MyLocation;
		const float Distance = ToTarget.Size();

		if (Distance > HomingSearchRadius)
		{
			continue;
		}

		const FVector DirectionToTarget = ToTarget.GetSafeNormal();
		const float Dot = FVector::DotProduct(MyForward, DirectionToTarget);

		const float ClampedDot = FMath::Clamp(Dot, -1.0f, 1.0f);
		const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(ClampedDot));

		if (AngleDegrees > HomingMaxAngleDegrees)
		{
			continue;
		}

		// Menor score = mejor target.
		// Prioriza cercanía y que esté más al frente.
		const float Score = Distance + AngleDegrees * 50.0f;

		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

bool AMetalProjectile::IsValidHomingTarget(AActor* Candidate) const
{
	if (!Candidate)
	{
		return false;
	}

	if (Candidate == this || Candidate == GetOwner())
	{
		return false;
	}

	if (Candidate->IsPendingKillPending())
	{
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const APawn* CandidatePawn = Cast<APawn>(Candidate);

	if (!CandidatePawn)
	{
		return false;
	}

	// Evita seguir al mismo jugador / dueño.
	if (OwnerPawn && OwnerPawn->GetController() == CandidatePawn->GetController())
	{
		return false;
	}

	const AMetalCombatVehicle* Vehicle = Cast<AMetalCombatVehicle>(Candidate);

	if (!Vehicle)
	{
		return false;
	}

	return true;
}