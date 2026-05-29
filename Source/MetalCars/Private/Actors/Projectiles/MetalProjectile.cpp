// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/MetalProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


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
}

void AMetalProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetLifeSpan(LifeSeconds);
		CollisionComponent->OnComponentHit.AddDynamic(this, &AMetalProjectile::OnProjectileHit);
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
		OtherComp->AddImpulseAtLocation(ImpulseDirection * ImpactImpulse, Hit.ImpactPoint);
	}

	Multicast_ImpactFX(Hit.ImpactPoint);
	Destroy();
}

void AMetalProjectile::Multicast_ImpactFX_Implementation(FVector ImpactLocation)
{
	BP_OnImpactFX(ImpactLocation);
}

