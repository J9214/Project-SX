// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXSkillBarrierActor.generated.h"

class ASXEnemyCharacterBase;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;
class UStaticMeshComponent;
class USphereComponent;

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXSkillBarrierActor : public AActor
{
	GENERATED_BODY()

public:
	ASXSkillBarrierActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="SX|Skill")
	void InitializeBarrier(
		AActor* InSourceActor,
		float InDuration,
		float InBarrierRadius,
		float InExpansionDuration,
		float InPushStrength,
		float InPushTickInterval
	);

protected:
	UFUNCTION()
	void HandleBarrierBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void UpdateBarrier(float DeltaSeconds);
	void ApplyBarrierPush();
	void PushEnemyOut(ASXEnemyCharacterBase* EnemyCharacter, float DeltaSeconds) const;
	void UpdateVisualScale(float CurrentRadius);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Defense|Barrier", meta=(ClampMin="0.0", Units=cm))
	float BarrierRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Defense|Barrier", meta=(ClampMin="0.0", Units=s))
	float ExpansionDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Defense|Barrier", meta=(ClampMin="0.0", Units="cm/s"))
	float PushStrength = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Defense|Barrier", meta=(ClampMin="0.01", Units=s))
	float PushTickInterval = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Defense|Barrier")
	bool bKeepEnemiesOutside = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Defense|Barrier|Visual", meta=(ClampMin="1.0", Units=cm))
	float VisualMeshBaseDiameter = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Defense|Barrier|Visual")
	FName ProgressParameterName = TEXT("Progress");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Defense|Barrier|Visual")
	FName OpacityParameterName = TEXT("Opacity");

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill")
	float Duration = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill")
	float ElapsedTime = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill")
	float CurrentRadius = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill")
	TObjectPtr<AActor> SourceActor;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill|Defense|Barrier|Visual")
	TObjectPtr<UMaterialInstanceDynamic> BarrierMaterialInstance;

	FTimerHandle PushTimerHandle;
};
