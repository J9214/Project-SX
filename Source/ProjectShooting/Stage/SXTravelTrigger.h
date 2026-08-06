// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXTravelTrigger.generated.h"

class APawn;
class ASXStageFlowManager;
class UBoxComponent;
class USoundBase;
class USXTravelDatabase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSXOnTravelTriggeredSignature, APawn*, TravelingPawn, FName, SourcePointId, FName, DestinationPointId);

UCLASS()
class PROJECTSHOOTING_API ASXTravelTrigger : public AActor
{
	GENERATED_BODY()

public:
	ASXTravelTrigger();

	UFUNCTION(BlueprintCallable, Category="SX|Travel")
	bool TravelPawn(APawn* TravelingPawn);

	UFUNCTION(BlueprintCallable, Category="SX|Travel")
	bool TravelActor(AActor* TravelingActor);

	UFUNCTION(BlueprintCallable, Category="SX|Travel|Portal")
	void SetPortalActive(bool bNewActive);

	UFUNCTION(BlueprintCallable, Category="SX|Travel|Portal")
	void ActivatePortal();

	UFUNCTION(BlueprintCallable, Category="SX|Travel|Portal")
	void DeactivatePortal();

	UFUNCTION(BlueprintPure, Category="SX|Travel|Portal")
	bool IsPortalActive() const { return bPortalActive; }

	UFUNCTION(BlueprintPure, Category="SX|Travel")
	TArray<FName> GetTravelPointIdOptions() const;

	UPROPERTY(BlueprintAssignable, Category="SX|Travel")
	FSXOnTravelTriggeredSignature OnTravelTriggered;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleStageCleared();

	bool ResolveDestination(FName& OutDestinationPointId, FTransform& OutDestinationTransform) const;
	void FinishTravel(APawn* TravelingPawn, FName ResolvedDestinationPointId);
	void BindStageFlowManager();
	void PlayTeleportSound(AActor* TravelingActor) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Travel")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel|Portal")
	TObjectPtr<ASXStageFlowManager> StageFlowManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel|Portal")
	TObjectPtr<ASXStageFlowManager> DestinationStageFlowManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	TObjectPtr<USXTravelDatabase> TravelDatabase;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel", meta=(GetOptions="GetTravelPointIdOptions"))
	FName SourcePointId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel", meta=(GetOptions="GetTravelPointIdOptions"))
	FName DestinationPointId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	FVector DestinationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	bool bPlayerOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	bool bTriggerOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	bool bSetControllerRotation = true;

	/** Played immediately before an actor travels through this portal. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel|Audio")
	TObjectPtr<USoundBase> TeleportSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel|Audio", meta=(ClampMin="0.0"))
	float TeleportSoundVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel|Portal")
	bool bStartActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel|Portal")
	bool bActivateOnStageCleared = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel|Portal")
	bool bAutoFindStageFlowManager = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel|Portal")
	bool bSetDestinationStageFlowManagerOnTravel = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Travel|Portal")
	bool bPortalActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Travel")
	bool bTriggered = false;
};
