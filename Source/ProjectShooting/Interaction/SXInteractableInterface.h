// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SXInteractableInterface.generated.h"

class ASXPlayerCharacter;

UINTERFACE(Blueprintable)
class PROJECTSHOOTING_API USXInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTSHOOTING_API ISXInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SX|Interaction")
	bool CanInteract(ASXPlayerCharacter* InteractingCharacter) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SX|Interaction")
	void Interact(ASXPlayerCharacter* InteractingCharacter);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SX|Interaction")
	FText GetInteractionText() const;
};
