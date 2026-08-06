// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/SXTravelTrigger.h"

#include "Components/BoxComponent.h"
#include "Controller/SXPlayerController.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Stage/SXStageFlowManager.h"
#include "Stage/SXTravelDatabase.h"

ASXTravelTrigger::ASXTravelTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(120.0f, 120.0f, 120.0f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

void ASXTravelTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleBeginOverlap);
	BindStageFlowManager();
	SetPortalActive(bStartActive);

	if (bActivateOnStageCleared && IsValid(StageFlowManager) && StageFlowManager->IsStageCleared())
	{
		ActivatePortal();
	}
}

void ASXTravelTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bTriggerOnce && bTriggered)
	{
		return;
	}

	if (bPortalActive == false)
	{
		return;
	}

	APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (IsValid(OverlappingPawn) == false)
	{
		return;
	}

	if (bPlayerOnly && OverlappingPawn->IsPlayerControlled() == false)
	{
		return;
	}

	TravelPawn(OverlappingPawn);
}

bool ASXTravelTrigger::TravelPawn(APawn* TravelingPawn)
{
	if (IsValid(TravelingPawn) == false)
	{
		return false;
	}

	if (bPortalActive == false)
	{
		return false;
	}

	if (bTriggerOnce && bTriggered)
	{
		return false;
	}

	FName ResolvedDestinationPointId = NAME_None;
	FTransform DestinationTransform = FTransform::Identity;
	if (ResolveDestination(ResolvedDestinationPointId, DestinationTransform) == false)
	{
		return false;
	}

	DestinationTransform.AddToTranslation(DestinationOffset);

	const FVector DestinationLocation = DestinationTransform.GetLocation();
	const FRotator DestinationRotation = DestinationTransform.GetRotation().Rotator();
	PlayTeleportSound(TravelingPawn);
	const bool bTeleported = TravelingPawn->TeleportTo(DestinationLocation, DestinationRotation, false, true);
	if (bTeleported == false)
	{
		TravelingPawn->SetActorLocationAndRotation(DestinationLocation, DestinationRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (bSetControllerRotation && IsValid(TravelingPawn->GetController()))
	{
		TravelingPawn->GetController()->SetControlRotation(DestinationRotation);
	}

	FinishTravel(TravelingPawn, ResolvedDestinationPointId);
	return true;
}

bool ASXTravelTrigger::TravelActor(AActor* TravelingActor)
{
	if (bPortalActive == false)
	{
		return false;
	}

	if (APawn* TravelingPawn = Cast<APawn>(TravelingActor))
	{
		return TravelPawn(TravelingPawn);
	}

	if (IsValid(TravelingActor) == false)
	{
		return false;
	}

	FName ResolvedDestinationPointId = NAME_None;
	FTransform DestinationTransform = FTransform::Identity;
	if (ResolveDestination(ResolvedDestinationPointId, DestinationTransform) == false)
	{
		return false;
	}

	DestinationTransform.AddToTranslation(DestinationOffset);
	PlayTeleportSound(TravelingActor);
	TravelingActor->SetActorTransform(DestinationTransform, false, nullptr, ETeleportType::TeleportPhysics);

	OnTravelTriggered.Broadcast(nullptr, SourcePointId, ResolvedDestinationPointId);
	return true;
}

void ASXTravelTrigger::SetPortalActive(bool bNewActive)
{
	bPortalActive = bNewActive;

	const bool bEnableCollision = bPortalActive && (bTriggerOnce == false || bTriggered == false);
	TriggerBox->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	SetActorHiddenInGame(!bPortalActive);
}

void ASXTravelTrigger::ActivatePortal()
{
	SetPortalActive(true);
}

void ASXTravelTrigger::DeactivatePortal()
{
	SetPortalActive(false);
}

void ASXTravelTrigger::HandleStageCleared()
{
	if (bActivateOnStageCleared)
	{
		ActivatePortal();
	}
}

TArray<FName> ASXTravelTrigger::GetTravelPointIdOptions() const
{
	if (IsValid(TravelDatabase))
	{
		return TravelDatabase->GetTravelPointIds();
	}

	return TArray<FName>();
}

bool ASXTravelTrigger::ResolveDestination(FName& OutDestinationPointId, FTransform& OutDestinationTransform) const
{
	if (IsValid(TravelDatabase) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXTravelTrigger %s has no TravelDatabase."), *GetName());
		return false;
	}

	if (TravelDatabase->ResolveDestinationPointId(SourcePointId, DestinationPointId, OutDestinationPointId) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXTravelTrigger %s cannot resolve destination. Source=%s Destination=%s"),
			*GetName(),
			*SourcePointId.ToString(),
			*DestinationPointId.ToString());
		return false;
	}

	if (TravelDatabase->GetTravelPointTransform(OutDestinationPointId, OutDestinationTransform) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXTravelTrigger %s cannot find destination point %s in %s."),
			*GetName(),
			*OutDestinationPointId.ToString(),
			*GetNameSafe(TravelDatabase.Get()));
		return false;
	}

	return true;
}

void ASXTravelTrigger::BindStageFlowManager()
{
	if (bActivateOnStageCleared == false)
	{
		return;
	}

	if (IsValid(StageFlowManager) == false && bAutoFindStageFlowManager)
	{
		StageFlowManager = Cast<ASXStageFlowManager>(UGameplayStatics::GetActorOfClass(this, ASXStageFlowManager::StaticClass()));
	}

	if (IsValid(StageFlowManager) == false)
	{
		return;
	}

	StageFlowManager->OnStageCleared.RemoveDynamic(this, &ThisClass::HandleStageCleared);
	StageFlowManager->OnStageCleared.AddDynamic(this, &ThisClass::HandleStageCleared);
}

void ASXTravelTrigger::PlayTeleportSound(AActor* TravelingActor) const
{
	if (IsValid(TeleportSound) == false || IsValid(TravelingActor) == false)
	{
		return;
	}

	const float Volume = FMath::Max(0.0f, TeleportSoundVolume);
	if (const APawn* TravelingPawn = Cast<APawn>(TravelingActor); IsValid(TravelingPawn) && TravelingPawn->IsPlayerControlled())
	{
		UGameplayStatics::PlaySound2D(this, TeleportSound, Volume);
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, TeleportSound, TravelingActor->GetActorLocation(), Volume);
}

void ASXTravelTrigger::FinishTravel(APawn* TravelingPawn, FName ResolvedDestinationPointId)
{
	bTriggered = true;
	OnTravelTriggered.Broadcast(TravelingPawn, SourcePointId, ResolvedDestinationPointId);

	if (bSetDestinationStageFlowManagerOnTravel
		&& IsValid(DestinationStageFlowManager)
		&& IsValid(TravelingPawn)
		&& TravelingPawn->IsPlayerControlled())
	{
		if (ASXPlayerController* SXPlayerController = Cast<ASXPlayerController>(TravelingPawn->GetController()))
		{
			SXPlayerController->SetActiveStageFlowManager(DestinationStageFlowManager.Get());
		}
	}

	if (bTriggerOnce)
	{
		SetPortalActive(false);
	}
}
