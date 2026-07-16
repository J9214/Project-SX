// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/SXWaveClearDoor.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Stage/SXWaveSpawner.h"

ASXWaveClearDoor::ASXWaveClearDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(SceneRoot);
}

void ASXWaveClearDoor::BeginPlay()
{
	Super::BeginPlay();

	ClosedRelativeLocation = DoorMesh->GetRelativeLocation();
	BindWaveSpawner();
}

void ASXWaveClearDoor::BindWaveSpawner()
{
	if (IsValid(WaveSpawner) == false && bAutoFindWaveSpawner)
	{
		WaveSpawner = Cast<ASXWaveSpawner>(UGameplayStatics::GetActorOfClass(this, ASXWaveSpawner::StaticClass()));
	}

	if (IsValid(WaveSpawner) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXWaveClearDoor %s has no WaveSpawner."), *GetNameSafe(this));
		return;
	}

	WaveSpawner->OnWaveCleared.AddDynamic(this, &ThisClass::HandleWaveCleared);

	if (WaveSpawner->IsWaveCleared())
	{
		OpenDoor();
	}
}

void ASXWaveClearDoor::HandleWaveCleared(int32 WaveIndex)
{
	UE_LOG(LogTemp, Log, TEXT("SXWaveClearDoor %s received WaveCleared: %d"), *GetNameSafe(this), WaveIndex);
	OpenDoor();
}

void ASXWaveClearDoor::OpenDoor()
{
	if (bIsOpen)
	{
		return;
	}

	bIsOpen = true;
	DoorMesh->SetRelativeLocation(ClosedRelativeLocation + OpenLocationOffset);

	if (bDisableCollisionWhenOpened)
	{
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ASXWaveClearDoor::CloseDoor()
{
	if (!bIsOpen)
	{
		return;
	}

	bIsOpen = false;
	DoorMesh->SetRelativeLocation(ClosedRelativeLocation);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
