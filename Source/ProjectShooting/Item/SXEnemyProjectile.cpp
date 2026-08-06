// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/SXEnemyProjectile.h"

#include "Character/SXCharacterBase.h"
#include "Character/SXEnemyCharacterBase.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"

ASXEnemyProjectile::ASXEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// Project collision channel 1 is named "Projectile" in DefaultEngine.ini.
	// Keeping enemy projectiles on this dedicated channel lets player barriers
	// stop enemy shots without also stopping the player's projectiles.
	CollisionComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1200.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void ASXEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleBeginOverlap);
	SetLifeSpan(LifeTime);
}

void ASXEnemyProjectile::InitializeProjectile(float InDamage, AActor* InDamageCauser)
{
	Damage = FMath::Max(0.0f, InDamage);
	DamageCauser = InDamageCauser;
}

void ASXEnemyProjectile::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(OtherActor) == false || OtherActor == this || OtherActor == GetOwner() || OtherActor == DamageCauser)
	{
		return;
	}

	if (Cast<ASXEnemyCharacterBase>(OtherActor))
	{
		return;
	}

	ASXCharacterBase* HitCharacter = Cast<ASXCharacterBase>(OtherActor);
	if (IsValid(HitCharacter) && HitCharacter->IsAlive())
	{
		FDamageEvent DamageEvent;
		HitCharacter->TakeDamage(Damage, DamageEvent, GetInstigatorController(), DamageCauser ? DamageCauser : this);
		Destroy();
		return;
	}

	Destroy();
}
