// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXSkillAreaActor.generated.h"

class USphereComponent;

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXSkillAreaActor : public AActor
{
	GENERATED_BODY()

public:
	ASXSkillAreaActor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="SX|Skill")
	void InitializeSkillArea(AActor* InSourceActor, float InDamagePerSecond, float InRadius, float InTickInterval, float InDuration);

protected:
	void ApplyTickDamage();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill", meta=(ClampMin="0.0"))
	float DamagePerSecond = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill", meta=(ClampMin="0.0"))
	float Radius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill", meta=(ClampMin="0.01"))
	float TickInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill", meta=(ClampMin="0.0"))
	float Duration = 3.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill")
	TObjectPtr<AActor> SourceActor;

	FTimerHandle DamageTimerHandle;
};
