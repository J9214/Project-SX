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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo Pickup", meta=(ClampMin="1"))
	int32 AmmoAmount = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo Pickup|Deprecated", meta=(DeprecatedProperty, DeprecationMessage="Ammo pickups no longer change the current weapon ammo type. Weapons have one fixed ammo type."))
	bool bSetCurrentWeaponAmmoTypeOnPickup = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo Pickup")
	bool bDestroyOnPickup = true;
};
