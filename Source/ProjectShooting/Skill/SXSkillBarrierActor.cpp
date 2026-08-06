// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXSkillBarrierActor.h"

#include "Character/SXEnemyCharacterBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/SXEnemyProjectile.h"
#include "Materials/MaterialInstanceDynamic.h"

ASXSkillBarrierActor::ASXSkillBarrierActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetSphereRadius(1.0f);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
}

void ASXSkillBarrierActor::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleBarrierBeginOverlap);

	CurrentRadius = 1.0f;
	CollisionComponent->SetSphereRadius(CurrentRadius);
	UpdateVisualScale(CurrentRadius);

	if (IsValid(MeshComponent.Get()) == true)
	{
		BarrierMaterialInstance = MeshComponent->CreateDynamicMaterialInstance(0);
	}

	if (Duration > 0.0f)
	{
		SetLifeSpan(Duration);
	}

	if (PushTickInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(PushTimerHandle, this, &ThisClass::ApplyBarrierPush, PushTickInterval, true, 0.0f);
	}
}

void ASXSkillBarrierActor::HandleBarrierBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (ASXEnemyProjectile* EnemyProjectile = Cast<ASXEnemyProjectile>(OtherActor))
	{
		EnemyProjectile->Destroy();
	}
}

void ASXSkillBarrierActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateBarrier(DeltaSeconds);
}

void ASXSkillBarrierActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(PushTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void ASXSkillBarrierActor::InitializeBarrier(
	AActor* InSourceActor,
	float InDuration,
	float InBarrierRadius,
	float InExpansionDuration,
	float InPushStrength,
	float InPushTickInterval
)
{
	SourceActor = InSourceActor;
	Duration = InDuration;
	BarrierRadius = FMath::Max(1.0f, InBarrierRadius);
	ExpansionDuration = FMath::Max(0.0f, InExpansionDuration);
	PushStrength = FMath::Max(0.0f, InPushStrength);
	PushTickInterval = FMath::Max(0.01f, InPushTickInterval);

	CurrentRadius = ExpansionDuration <= 0.0f ? BarrierRadius : 1.0f;

	if (IsValid(CollisionComponent.Get()) == true)
	{
		CollisionComponent->SetSphereRadius(CurrentRadius);
	}

	UpdateVisualScale(CurrentRadius);
}

void ASXSkillBarrierActor::UpdateBarrier(float DeltaSeconds)
{
	ElapsedTime += DeltaSeconds;

	const float ExpansionAlpha = ExpansionDuration <= 0.0f
		? 1.0f
		: FMath::Clamp(ElapsedTime / ExpansionDuration, 0.0f, 1.0f);

	CurrentRadius = FMath::Lerp(1.0f, BarrierRadius, ExpansionAlpha);
	CollisionComponent->SetSphereRadius(CurrentRadius);
	UpdateVisualScale(CurrentRadius);

	if (IsValid(BarrierMaterialInstance.Get()) == true)
	{
		if (ProgressParameterName != NAME_None)
		{
			BarrierMaterialInstance->SetScalarParameterValue(ProgressParameterName, ExpansionAlpha);
		}

		if (OpacityParameterName != NAME_None && Duration > 0.0f)
		{
			const float LifetimeAlpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
			BarrierMaterialInstance->SetScalarParameterValue(OpacityParameterName, 1.0f - LifetimeAlpha);
		}
	}
}

void ASXSkillBarrierActor::ApplyBarrierPush()
{
	TArray<AActor*> OverlappingActors;
	CollisionComponent->GetOverlappingActors(OverlappingActors, ASXEnemyCharacterBase::StaticClass());

	const float DeltaSeconds = FMath::Max(0.01f, PushTickInterval);
	for (AActor* OverlappingActor : OverlappingActors)
	{
		ASXEnemyCharacterBase* EnemyCharacter = Cast<ASXEnemyCharacterBase>(OverlappingActor);
		if (IsValid(EnemyCharacter) == true)
		{
			PushEnemyOut(EnemyCharacter, DeltaSeconds);
		}
	}
}

void ASXSkillBarrierActor::PushEnemyOut(ASXEnemyCharacterBase* EnemyCharacter, float DeltaSeconds) const
{
	if (IsValid(EnemyCharacter) == false || EnemyCharacter->IsAlive() == false)
	{
		return;
	}

	FVector FromCenter = EnemyCharacter->GetActorLocation() - GetActorLocation();
	FromCenter.Z = 0.0f;

	if (FromCenter.IsNearlyZero())
	{
		FromCenter = FVector::ForwardVector;
	}

	const FVector PushDirection = FromCenter.GetSafeNormal();
	const float Distance2D = FVector::Dist2D(EnemyCharacter->GetActorLocation(), GetActorLocation());
	const float TargetDistance = CurrentRadius + 10.0f;

	if (bKeepEnemiesOutside && Distance2D < TargetDistance)
	{
		FVector NewLocation = GetActorLocation() + PushDirection * TargetDistance;
		NewLocation.Z = EnemyCharacter->GetActorLocation().Z;
		EnemyCharacter->SetActorLocation(NewLocation, true);
	}

	if (UCharacterMovementComponent* MovementComponent = EnemyCharacter->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	const FVector LaunchVelocity = PushDirection * PushStrength * DeltaSeconds;
	EnemyCharacter->LaunchCharacter(LaunchVelocity, true, false);
}

void ASXSkillBarrierActor::UpdateVisualScale(float InCurrentRadius)
{
	if (IsValid(MeshComponent.Get()) == false)
	{
		return;
	}

	const float SafeBaseDiameter = FMath::Max(1.0f, VisualMeshBaseDiameter);
	const float Diameter = InCurrentRadius * 2.0f;
	const float VisualScale = Diameter / SafeBaseDiameter;
	MeshComponent->SetRelativeScale3D(FVector(VisualScale));
}
