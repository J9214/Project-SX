// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Item/SXWeapon.h"
#include "SXShopItemData.generated.h"

UENUM(BlueprintType)
enum class ESXShopItemType : uint8
{
	Heal,
	Ammo,
	Weapon,
	AmmoType,
	Custom
};

UCLASS(BlueprintType)
class PROJECTSHOOTING_API USXShopItemData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop", meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop", meta=(ClampMin="0"))
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop")
	ESXShopItemType ItemType = ESXShopItemType::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop|Heal", meta=(ClampMin="0.0", EditCondition="ItemType == ESXShopItemType::Heal", EditConditionHides))
	float HealAmount = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop|Ammo", meta=(ClampMin="1", EditCondition="ItemType == ESXShopItemType::Ammo", EditConditionHides))
	int32 AmmoAmount = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop|Ammo", meta=(EditCondition="ItemType == ESXShopItemType::AmmoType", EditConditionHides))
	ESXAmmoType AmmoType = ESXAmmoType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop|Weapon", meta=(EditCondition="ItemType == ESXShopItemType::Weapon", EditConditionHides))
	TSubclassOf<ASXWeapon> WeaponClass;
};
