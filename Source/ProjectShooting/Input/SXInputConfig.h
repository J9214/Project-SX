// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SXInputConfig.generated.h"

class UInputAction;
/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTSHOOTING_API USXInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> AttackMeleeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> ChangeViewAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> ToggleSelectorAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input")
	TObjectPtr<UInputAction> IronSightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input|Weapon")
	TObjectPtr<UInputAction> WeaponSlot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input|Weapon")
	TObjectPtr<UInputAction> WeaponSlot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input|Weapon")
	TObjectPtr<UInputAction> WeaponSlot3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input|Weapon")
	TObjectPtr<UInputAction> NextWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input|Weapon")
	TObjectPtr<UInputAction> PreviousWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input|Weapon")
	TObjectPtr<UInputAction> DropWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input|Skill")
	TObjectPtr<UInputAction> MovementSkillAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input|Skill")
	TObjectPtr<UInputAction> Skill1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input|Skill")
	TObjectPtr<UInputAction> Skill2Action;
};
