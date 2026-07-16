// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXWaveClearDoor.generated.h"

class ASXWaveSpawner;
class UStaticMeshComponent;

UCLASS()
class PROJECTSHOOTING_API ASXWaveClearDoor : public AActor
{
	GENERATED_BODY()

public:
	ASXWaveClearDoor();

	UFUNCTION(BlueprintCallable, Category="SX|Door")
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category="SX|Door")
	void CloseDoor();

	UFUNCTION(BlueprintPure, Category="SX|Door")
	bool IsDoorOpen() const { return bIsOpen; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleWaveCleared(int32 WaveIndex);

	void BindWaveSpawner();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Door")
	TObjectPtr<ASXWaveSpawner> WaveSpawner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Door")
	bool bAutoFindWaveSpawner = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Door")
	FVector OpenLocationOffset = FVector(0.0f, 0.0f, 300.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Door")
	bool bDisableCollisionWhenOpened = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Door")
	bool bIsOpen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Door")
	FVector ClosedRelativeLocation = FVector::ZeroVector;
};
