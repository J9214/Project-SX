// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/SXWeapon.h"
#include "SXCollectiblePickup.generated.h"

class ASXPlayerCharacter;
class USXPickupComponent;

UENUM(BlueprintType)
enum class ESXCollectibleType : uint8
{
	Gold,
	Experience,
	Ammo
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSXOnCollectiblePickedUpSignature, ASXPlayerCharacter*, PickUpCharacter, ESXCollectibleType, CollectibleType, int32, Amount);

UCLASS()
class PROJECTSHOOTING_API ASXCollectiblePickup : public AActor
{
	GENERATED_BODY()

public:
	ASXCollectiblePickup();

	UFUNCTION(BlueprintPure, Category="SX|Collectible")
	ESXCollectibleType GetCollectibleType() const { return CollectibleType; }

	UFUNCTION(BlueprintPure, Category="SX|Collectible")
	int32 GetAmount() const { return Amount; }

	UFUNCTION(BlueprintCallable, Category="SX|Collectible")
	void InitializeCollectible(ESXCollectibleType InCollectibleType, int32 InAmount);

	UFUNCTION(BlueprintCallable, Category="SX|Collectible")
	void InitializeAmmoCollectible(ESXAmmoType InAmmoType, int32 InAmount);

	UPROPERTY(BlueprintAssignable, Category="SX|Collectible")
	FSXOnCollectiblePickedUpSignature OnCollectiblePickedUp;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOnPickUp(ASXPlayerCharacter* InPickUpCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category="SX|Collectible")
	void BP_OnCollected(ASXPlayerCharacter* InPickUpCharacter, ESXCollectibleType InCollectibleType, int32 InAmount);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USXPickupComponent> PickupComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Collectible")
	ESXCollectibleType CollectibleType = ESXCollectibleType::Gold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Collectible", meta=(ClampMin="1"))
	int32 Amount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Collectible|Ammo", meta=(EditCondition="CollectibleType == ESXCollectibleType::Ammo", EditConditionHides))
	ESXAmmoType AmmoType = ESXAmmoType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Collectible")
	bool bDestroyOnPickup = true;
};
