// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/SXStageStartTrigger.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Stage/SXStageFlowManager.h"

ASXStageStartTrigger::ASXStageStartTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(120.0f, 120.0f, 120.0f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

void ASXStageStartTrigger::BeginPlay()
{
	Super::BeginPlay();

	FindStageFlowManager();
	TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleBeginOverlap);
}

void ASXStageStartTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bTriggerOnce && bTriggered)
	{
		return;
	}

	APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (IsValid(OverlappingPawn) == false || OverlappingPawn->IsPlayerControlled() == false)
	{
		return;
	}

	FindStageFlowManager();
	if (IsValid(StageFlowManager) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXStageStartTrigger %s has no StageFlowManager."), *GetName());
		return;
	}

	bTriggered = true;
	StageFlowManager->StartStage();

	if (bTriggerOnce)
	{
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ASXStageStartTrigger::FindStageFlowManager()
{
	if (bAutoFindStageFlowManager && IsValid(StageFlowManager) == false)
	{
		TArray<AActor*> FoundManagers;
		UGameplayStatics::GetAllActorsOfClass(this, ASXStageFlowManager::StaticClass(), FoundManagers);

		if (FoundManagers.Num() == 1)
		{
			StageFlowManager = Cast<ASXStageFlowManager>(FoundManagers[0]);
			return;
		}

		if (FoundManagers.Num() > 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("SXStageStartTrigger %s found %d StageFlowManagers. Set StageFlowManager explicitly and disable AutoFind."),
				*GetName(),
				FoundManagers.Num());
		}
	}
}
