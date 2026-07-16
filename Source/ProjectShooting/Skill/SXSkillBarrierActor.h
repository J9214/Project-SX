// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXSkillBarrierActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXSkillBarrierActor : public AActor
{
	GENERATED_BODY()

public:
	ASXSkillBarrierActor();

	UFUNCTION(BlueprintCallable, Category="SX|Skill")
	void InitializeBarrier(AActor* InSourceActor, float InDuration);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UBoxComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill")
	TObjectPtr<AActor> SourceActor;
};
