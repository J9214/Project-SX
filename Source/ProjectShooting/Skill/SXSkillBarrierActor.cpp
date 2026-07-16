// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXSkillBarrierActor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

ASXSkillBarrierActor::ASXSkillBarrierActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetBoxExtent(FVector(25.0f, 200.0f, 140.0f));
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASXSkillBarrierActor::InitializeBarrier(AActor* InSourceActor, float InDuration)
{
	SourceActor = InSourceActor;

	if (InDuration > 0.0f)
	{
		SetLifeSpan(InDuration);
	}
}
