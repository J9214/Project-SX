// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/SXStageDoor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ASXStageDoor::ASXStageDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(SceneRoot);
}

void ASXStageDoor::BeginPlay()
{
	Super::BeginPlay();

	ClosedRelativeLocation = DoorMesh->GetRelativeLocation();
	SetDoorOpened(bStartOpen);
}

void ASXStageDoor::OpenDoor()
{
	SetDoorOpened(true);
}

void ASXStageDoor::CloseDoor()
{
	SetDoorOpened(false);
}

void ASXStageDoor::SetDoorOpened(bool bOpened)
{
	bIsOpen = bOpened;

	if (IsValid(DoorMesh) == false)
	{
		return;
	}

	const FVector TargetLocation = bIsOpen ? ClosedRelativeLocation + OpenLocationOffset : ClosedRelativeLocation;
	DoorMesh->SetRelativeLocation(TargetLocation);
	DoorMesh->SetCollisionEnabled(bIsOpen && bDisableCollisionWhenOpened ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
}
