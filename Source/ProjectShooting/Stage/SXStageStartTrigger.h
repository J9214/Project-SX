// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXStageStartTrigger.generated.h"

class ASXStageFlowManager;
class UBoxComponent;

UCLASS()
class PROJECTSHOOTING_API ASXStageStartTrigger : public AActor
{
	GENERATED_BODY()

public:
	ASXStageStartTrigger();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void FindStageFlowManager();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Stage")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	TObjectPtr<ASXStageFlowManager> StageFlowManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	bool bAutoFindStageFlowManager = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	bool bTriggerOnce = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Stage")
	bool bTriggered = false;
};
