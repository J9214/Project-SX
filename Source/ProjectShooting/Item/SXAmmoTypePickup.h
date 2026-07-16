// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/SXWeapon.h"
#include "SXAmmoTypePickup.generated.h"

class ASXPlayerCharacter;
class USXPickupComponent;

UCLASS()
class PROJECTSHOOTING_API ASXAmmoTypePickup : public AActor
{
	GENERATED_BODY()

public:
	ASXAmmoTypePickup();

	UFUNCTION(BlueprintPure, Category="SX|Ammo Pickup")
	ESXAmmoType GetDesiredAmmoType() const { return DesiredAmmoType; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOnPickUp(ASXPlayerCharacter* InPickUpCharacter);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USXPickupComponent> PickupComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo Pickup")
	ESXAmmoType DesiredAmmoType = ESXAmmoType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo Pickup")
	bool bDestroyOnPickup = true;
};
