// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/SXAmmoProjectile.h"

#include "Character/SXCharacterBase.h"
#include "Character/SXPlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Controller/SXPlayerController.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"

ASXAmmoProjectile::ASXAmmoProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetSphereRadius(8.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void ASXAmmoProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleBeginOverlap);
	SetLifeSpan(LifeTime);
}

void ASXAmmoProjectile::InitializeProjectile(ESXAmmoType InAmmoType, float InDamage, float InExplosiveRadius, int32 InMaxPiercingHitCount, float InLifeTime, AActor* InDamageCauser, AActor* InSourceActor)
{
	AmmoType = InAmmoType;
	Damage = FMath::Max(0.0f, InDamage);
	ExplosiveRadius = FMath::Max(0.0f, InExplosiveRadius);
	MaxPiercingHitCount = FMath::Max(1, InMaxPiercingHitCount);
	LifeTime = FMath::Max(0.1f, InLifeTime);
	DamageCauser = InDamageCauser;
	SourceActor = InSourceActor;

	SetLifeSpan(LifeTime);
}

void ASXAmmoProjectile::SetProjectileSpeed(float InProjectileSpeed)
{
	const float ProjectileSpeed = FMath::Max(0.0f, InProjectileSpeed);
	ProjectileMovement->InitialSpeed = ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileSpeed;
	ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileSpeed;
}

void ASXAmmoProjectile::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(OtherActor) == false || OtherActor == this || OtherActor == DamageCauser || OtherActor == SourceActor)
	{
		return;
	}

	if (AmmoType == ESXAmmoType::Explosive)
	{
		ApplyExplosionDamage(GetActorLocation());
		Destroy();
		return;
	}

	ASXCharacterBase* HittedCharacter = Cast<ASXCharacterBase>(OtherActor);
	if (IsValid(HittedCharacter) == true && HittedCharacter->IsAlive() == true)
	{
		FDamageEvent DamageEvent;
		const float AppliedDamage = HittedCharacter->TakeDamage(Damage, DamageEvent, GetInstigatorController(), DamageCauser ? DamageCauser : this);
		if (AppliedDamage > 0.0f)
		{
			if (ASXPlayerCharacter* SourcePlayer = Cast<ASXPlayerCharacter>(SourceActor))
			{
				if (ASXPlayerController* PlayerController = SourcePlayer->GetController<ASXPlayerController>())
				{
					PlayerController->ShowHitMarker(HittedCharacter->IsAlive() == false);
				}
			}
		}

		if (AmmoType == ESXAmmoType::Piercing)
		{
			++CurrentPiercingHitCount;
			if (CurrentPiercingHitCount < MaxPiercingHitCount)
			{
				return;
			}
		}
	}

	Destroy();
}

void ASXAmmoProjectile::ApplyExplosionDamage(const FVector& ExplosionLocation)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	FCollisionShape ExplosionShape = FCollisionShape::MakeSphere(ExplosiveRadius);
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams OverlapParams(NAME_None, false, this);
	OverlapParams.AddIgnoredActor(this);
	OverlapParams.AddIgnoredActor(DamageCauser);
	OverlapParams.AddIgnoredActor(SourceActor);

	const bool bHasOverlaps = World->OverlapMultiByChannel(
		OverlapResults,
		ExplosionLocation,
		FQuat::Identity,
		ECC_Pawn,
		ExplosionShape,
		OverlapParams
	);

	if (bHasOverlaps == false)
	{
		return;
	}

	TSet<AActor*> DamagedActors;
	bool bAnyDamageApplied = false;
	bool bAnyKilled = false;
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		ASXCharacterBase* HittedCharacter = Cast<ASXCharacterBase>(OverlapResult.GetActor());
		if (IsValid(HittedCharacter) == false || DamagedActors.Contains(HittedCharacter))
		{
			continue;
		}

		FDamageEvent DamageEvent;
		const float AppliedDamage = HittedCharacter->TakeDamage(Damage, DamageEvent, GetInstigatorController(), DamageCauser ? DamageCauser : this);
		if (AppliedDamage > 0.0f)
		{
			bAnyDamageApplied = true;
			bAnyKilled |= HittedCharacter->IsAlive() == false;
		}
		DamagedActors.Add(HittedCharacter);
	}

	if (bAnyDamageApplied)
	{
		if (ASXPlayerCharacter* SourcePlayer = Cast<ASXPlayerCharacter>(SourceActor))
		{
			if (ASXPlayerController* PlayerController = SourcePlayer->GetController<ASXPlayerController>())
			{
				PlayerController->ShowHitMarker(bAnyKilled);
			}
		}
	}
}
