// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/SXWeapon.h"
#include "SXInventoryComponent.generated.h"

class USXSkillData;

USTRUCT(BlueprintType)
struct FSXInventoryAmmoStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Inventory|Ammo")
	ESXAmmoType AmmoType = ESXAmmoType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Inventory|Ammo", meta=(ClampMin="0"))
	int32 Count = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSXOnInventoryWeaponSlotChangedSignature, int32, SlotIndex, TSubclassOf<ASXWeapon>, WeaponClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSXOnInventoryAmmoChangedSignature, ESXAmmoType, AmmoType, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSXOnInventorySkillsChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSXOnInventoryAugmentsChangedSignature);

UCLASS(ClassGroup=(SX), meta=(BlueprintSpawnableComponent))
class PROJECTSHOOTING_API USXInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USXInventoryComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Weapon")
	int32 GetMaxWeaponSlots() const { return MaxWeaponSlots; }

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Weapon")
	TArray<TSubclassOf<ASXWeapon>> GetWeaponSlots() const { return WeaponSlots; }

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Weapon")
	TSubclassOf<ASXWeapon> GetWeaponInSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Weapon")
	int32 FindEmptyWeaponSlot() const;

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Weapon")
	bool HasWeaponClass(TSubclassOf<ASXWeapon> WeaponClass) const;

	UFUNCTION(BlueprintCallable, Category="SX|Inventory|Weapon")
	bool AddWeaponClass(TSubclassOf<ASXWeapon> WeaponClass, int32& OutSlotIndex);

	UFUNCTION(BlueprintCallable, Category="SX|Inventory|Weapon")
	bool SetWeaponSlot(int32 SlotIndex, TSubclassOf<ASXWeapon> WeaponClass);

	UFUNCTION(BlueprintCallable, Category="SX|Inventory|Weapon")
	bool RemoveWeaponAt(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Ammo")
	int32 GetAmmoCount(ESXAmmoType AmmoType) const;

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Ammo")
	bool HasAmmo(ESXAmmoType AmmoType, int32 Amount = 1) const;

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Ammo")
	TArray<FSXInventoryAmmoStack> GetAmmoStacks() const;

	UFUNCTION(BlueprintCallable, Category="SX|Inventory|Ammo")
	void SetAmmoCount(ESXAmmoType AmmoType, int32 Count);

	UFUNCTION(BlueprintCallable, Category="SX|Inventory|Ammo")
	int32 AddAmmo(ESXAmmoType AmmoType, int32 Amount);

	UFUNCTION(BlueprintCallable, Category="SX|Inventory|Ammo")
	bool ConsumeAmmo(ESXAmmoType AmmoType, int32 Amount);

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Skill")
	TArray<USXSkillData*> GetOwnedSkills() const;

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Skill")
	bool HasSkill(USXSkillData* SkillData) const;

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Skill")
	bool IsSkillInventoryFull() const;

	UFUNCTION(BlueprintCallable, Category="SX|Inventory|Skill")
	bool AddSkill(USXSkillData* SkillData);

	UFUNCTION(BlueprintCallable, Category="SX|Inventory|Skill")
	bool RemoveSkill(USXSkillData* SkillData);

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Augment")
	TArray<FName> GetOwnedAugments() const { return OwnedAugments; }

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Augment")
	bool HasAugment(FName AugmentId) const;

	UFUNCTION(BlueprintPure, Category="SX|Inventory|Augment")
	bool IsAugmentInventoryFull() const;

	UFUNCTION(BlueprintCallable, Category="SX|Inventory|Augment")
	bool AddAugment(FName AugmentId);

	UFUNCTION(BlueprintCallable, Category="SX|Inventory|Augment")
	bool RemoveAugment(FName AugmentId);

	UPROPERTY(BlueprintAssignable, Category="SX|Inventory")
	FSXOnInventoryWeaponSlotChangedSignature OnWeaponSlotChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|Inventory")
	FSXOnInventoryAmmoChangedSignature OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|Inventory")
	FSXOnInventorySkillsChangedSignature OnSkillsChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|Inventory")
	FSXOnInventoryAugmentsChangedSignature OnAugmentsChanged;

protected:
	void NormalizeWeaponSlots();
	void InitializeAmmoStacks();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Inventory|Weapon", meta=(ClampMin="1"))
	int32 MaxWeaponSlots = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Inventory|Weapon")
	TArray<TSubclassOf<ASXWeapon>> WeaponSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Inventory|Ammo")
	TMap<ESXAmmoType, int32> AmmoCounts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Inventory|Skill", meta=(ClampMin="0"))
	int32 MaxOwnedSkillSlots = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Inventory|Skill")
	TArray<TObjectPtr<USXSkillData>> OwnedSkills;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Inventory|Augment", meta=(ClampMin="0"))
	int32 MaxOwnedAugmentSlots = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Inventory|Augment")
	TArray<FName> OwnedAugments;
};
