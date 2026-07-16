// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXSkillAreaActor.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

ASXSkillAreaActor::ASXSkillAreaActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->SetSphereRadius(Radius);
}

void ASXSkillAreaActor::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->SetSphereRadius(Radius);

	if (Duration > 0.0f)
	{
		SetLifeSpan(Duration);
	}

	GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &ThisClass::ApplyTickDamage, TickInterval, true, 0.0f);
}

void ASXSkillAreaActor::InitializeSkillArea(AActor* InSourceActor, float InDamagePerSecond, float InRadius, float InTickInterval, float InDuration)
{
	SourceActor = InSourceActor;
	DamagePerSecond = InDamagePerSecond;
	Radius = InRadius;
	TickInterval = FMath::Max(0.01f, InTickInterval);
	Duration = InDuration;

	if (IsValid(CollisionComponent.Get()) == true)
	{
		CollisionComponent->SetSphereRadius(Radius);
	}
}

void ASXSkillAreaActor::ApplyTickDamage()
{
	TArray<AActor*> OverlappingActors;
	CollisionComponent->GetOverlappingActors(OverlappingActors);

	AController* SourceController = IsValid(SourceActor.Get()) == true ? SourceActor->GetInstigatorController() : nullptr;
	const float DamageAmount = DamagePerSecond * TickInterval;

	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (IsValid(OverlappingActor) == false || OverlappingActor == SourceActor)
		{
			continue;
		}

		UGameplayStatics::ApplyDamage(OverlappingActor, DamageAmount, SourceController, SourceActor, nullptr);
	}
}
