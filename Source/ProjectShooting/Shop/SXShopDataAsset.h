// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SXShopDataAsset.generated.h"

class USXShopItemData;

UCLASS(BlueprintType)
class PROJECTSHOOTING_API USXShopDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop")
	FName ShopId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop")
	FText ShopName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop")
	TArray<TObjectPtr<USXShopItemData>> ShopItems;
};
