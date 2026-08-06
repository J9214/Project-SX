// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXWorldDistortionAreaActor.generated.h"

class UCharacterMovementComponent;
class UDecalComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMaterialParameterCollection;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXWorldDistortionAreaActor : public AActor
{
	GENERATED_BODY()

public:
	ASXWorldDistortionAreaActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="SX|Skill|World Distortion")
	void InitializeWorldDistortionArea(
		AActor* InSourceActor,
		float InRadius,
		float InSlowMultiplier,
		float InDuration,
		UMaterialParameterCollection* InParameterCollection,
		FName InPositionParameterName
	);

protected:
	UFUNCTION()
	void HandleAreaBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleAreaEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	void ApplyDistortionParameters();
	void ClearDistortionParameters();
	void ApplySlowToActor(AActor* TargetActor);
	void RemoveSlowFromActor(AActor* TargetActor);
	void RemoveAllSlows();
	void InitializeVisuals();
	void UpdateVisuals(float Alpha);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UStaticMeshComponent> DistortionMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UDecalComponent> RangeDecalComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion", meta=(ClampMin="0.0"))
	float Radius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SlowMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion", meta=(ClampMin="0.0"))
	float Duration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion")
	bool bAffectSourceActor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion")
	bool bClearDistortionParameterOnEnd = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion")
	FVector ClearedDistortionPosition = FVector(0.0f, 0.0f, -100000.0f);

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill|World Distortion")
	TObjectPtr<AActor> SourceActor;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill|World Distortion")
	TObjectPtr<UMaterialParameterCollection> ParameterCollection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion")
	FName PositionParameterName = TEXT("Position");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual")
	TObjectPtr<UMaterialInterface> DistortionMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual")
	TObjectPtr<UMaterialInterface> RangeDecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual")
	bool bAutoScaleVisualsToRadius = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual", meta=(ClampMin="1.0"))
	float VisualMeshBaseSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual", meta=(ClampMin="0.001"))
	float VisualMeshZScale = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual", meta=(ClampMin="0.0"))
	float VisualZOffset = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual", meta=(ClampMin="1.0"))
	float DecalDepth = 256.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual")
	FName ProgressParameterName = TEXT("Progress");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual")
	FName OpacityParameterName = TEXT("Opacity");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual")
	FName DistortionStrengthParameterName = TEXT("DistortionStrength");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual")
	FName MaterialRadiusParameterName = TEXT("Radius");

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual")
	TObjectPtr<UMaterialInstanceDynamic> DistortionMaterialInstance;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual")
	TObjectPtr<UMaterialInstanceDynamic> RangeDecalMaterialInstance;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill|World Distortion|Visual")
	float ElapsedTime = 0.0f;

	TMap<UCharacterMovementComponent*, float> OriginalMaxWalkSpeeds;
};
