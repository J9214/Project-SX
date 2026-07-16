// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXStageDoor.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class PROJECTSHOOTING_API ASXStageDoor : public AActor
{
	GENERATED_BODY()

public:
	ASXStageDoor();

	UFUNCTION(BlueprintCallable, Category="SX|Stage")
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category="SX|Stage")
	void CloseDoor();

	UFUNCTION(BlueprintCallable, Category="SX|Stage")
	void SetDoorOpened(bool bOpened);

	UFUNCTION(BlueprintPure, Category="SX|Stage")
	bool IsDoorOpen() const { return bIsOpen; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Stage")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Stage")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	FVector OpenLocationOffset = FVector(0.0f, 0.0f, 300.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	bool bDisableCollisionWhenOpened = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	bool bStartOpen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Stage")
	bool bIsOpen = false;

	FVector ClosedRelativeLocation = FVector::ZeroVector;
};
