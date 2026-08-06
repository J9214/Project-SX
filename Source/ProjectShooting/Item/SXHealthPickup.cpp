// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/SXHealthPickup.h"

#include "Character/SXPlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SXStatusComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

ASXHealthPickup::ASXHealthPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetSphereRadius(80.0f);
	CollisionComponent->SetCanEverAffectNavigation(false);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
}

void ASXHealthPickup::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandlePickupBeginOverlap);
}

bool ASXHealthPickup::TryConsume(ASXPlayerCharacter* PickUpCharacter)
{
	if (bConsumed || IsValid(PickUpCharacter) == false || HealAmount <= 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("HealthPickup %s consume rejected. Consumed=%d, Character=%s, HealAmount=%.1f"),
			*GetNameSafe(this),
			bConsumed,
			*GetNameSafe(PickUpCharacter),
			HealAmount);
		return false;
	}

	USXStatusComponent* StatusComponent = PickUpCharacter->GetStatusComponent();
	if (IsValid(StatusComponent) == false || StatusComponent->IsAlive() == false)
	{
		UE_LOG(LogTemp, Log, TEXT("HealthPickup %s consume rejected. Status invalid or character dead."), *GetNameSafe(this));
		return false;
	}

	if (StatusComponent->GetHealth() >= StatusComponent->GetMaxHealth())
	{
		UE_LOG(LogTemp, Log, TEXT("HealthPickup %s overlapped by %s, but health is already full. Health: %.1f / %.1f"),
			*GetNameSafe(this),
			*GetNameSafe(PickUpCharacter),
			StatusComponent->GetHealth(),
			StatusComponent->GetMaxHealth());
		return false;
	}

	const float AppliedHealAmount = StatusComponent->Heal(HealAmount, this);
	if (AppliedHealAmount <= 0.0f)
	{
		return false;
	}

	bConsumed = true;
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetVisibility(false, true);

	if (IsValid(PickupVFX.Get()) == true)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupVFX.Get(), GetActorTransform());
	}

	if (IsValid(PickupSFX.Get()) == true)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSFX, GetActorLocation());
	}

	OnHealthPickupConsumed.Broadcast(PickUpCharacter, HealAmount, AppliedHealAmount);
	BP_OnHealthPickupConsumed(PickUpCharacter, HealAmount, AppliedHealAmount);

	UE_LOG(LogTemp, Log, TEXT("%s consumed health pickup %s. Heal: %.1f / %.1f"),
		*GetNameSafe(PickUpCharacter),
		*GetNameSafe(this),
		AppliedHealAmount,
		HealAmount);

	if (bDestroyOnConsumed)
	{
		Destroy();
	}

	return true;
}

void ASXHealthPickup::HandlePickupBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	ASXPlayerCharacter* PlayerCharacter = Cast<ASXPlayerCharacter>(OtherActor);
	if (IsValid(PlayerCharacter) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("HealthPickup %s overlapped non-player actor %s."), *GetNameSafe(this), *GetNameSafe(OtherActor));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HealthPickup %s overlapped player %s."), *GetNameSafe(this), *GetNameSafe(PlayerCharacter));
	TryConsume(PlayerCharacter);
}
