// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SXInventoryComponent.h"

#include "Skill/SXSkillData.h"

USXInventoryComponent::USXInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	WeaponSlots.SetNum(MaxWeaponSlots);
}

void USXInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	NormalizeWeaponSlots();
	InitializeAmmoStacks();
}

TSubclassOf<ASXWeapon> USXInventoryComponent::GetWeaponInSlot(int32 SlotIndex) const
{
	return WeaponSlots.IsValidIndex(SlotIndex) ? WeaponSlots[SlotIndex] : nullptr;
}

int32 USXInventoryComponent::FindEmptyWeaponSlot() const
{
	for (int32 SlotIndex = 0; SlotIndex < WeaponSlots.Num(); ++SlotIndex)
	{
		if (WeaponSlots[SlotIndex] == nullptr)
		{
			return SlotIndex;
		}
	}

	return INDEX_NONE;
}

bool USXInventoryComponent::HasWeaponClass(TSubclassOf<ASXWeapon> WeaponClass) const
{
	if (WeaponClass == nullptr)
	{
		return false;
	}

	return WeaponSlots.Contains(WeaponClass);
}

bool USXInventoryComponent::AddWeaponClass(TSubclassOf<ASXWeapon> WeaponClass, int32& OutSlotIndex)
{
	OutSlotIndex = INDEX_NONE;
	if (WeaponClass == nullptr)
	{
		return false;
	}

	const int32 ExistingSlotIndex = WeaponSlots.Find(WeaponClass);
	if (ExistingSlotIndex != INDEX_NONE)
	{
		OutSlotIndex = ExistingSlotIndex;
		return true;
	}

	const int32 EmptySlotIndex = FindEmptyWeaponSlot();
	if (EmptySlotIndex == INDEX_NONE)
	{
		return false;
	}

	WeaponSlots[EmptySlotIndex] = WeaponClass;
	OutSlotIndex = EmptySlotIndex;
	OnWeaponSlotChanged.Broadcast(EmptySlotIndex, WeaponClass);
	return true;
}

bool USXInventoryComponent::SetWeaponSlot(int32 SlotIndex, TSubclassOf<ASXWeapon> WeaponClass)
{
	if (WeaponSlots.IsValidIndex(SlotIndex) == false)
	{
		return false;
	}

	if (WeaponClass != nullptr)
	{
		const int32 ExistingSlotIndex = WeaponSlots.Find(WeaponClass);
		if (ExistingSlotIndex != INDEX_NONE && ExistingSlotIndex != SlotIndex)
		{
			return false;
		}
	}

	if (WeaponSlots[SlotIndex] == WeaponClass)
	{
		return true;
	}

	WeaponSlots[SlotIndex] = WeaponClass;
	OnWeaponSlotChanged.Broadcast(SlotIndex, WeaponClass);
	return true;
}

bool USXInventoryComponent::RemoveWeaponAt(int32 SlotIndex)
{
	if (WeaponSlots.IsValidIndex(SlotIndex) == false || WeaponSlots[SlotIndex] == nullptr)
	{
		return false;
	}

	WeaponSlots[SlotIndex] = nullptr;
	OnWeaponSlotChanged.Broadcast(SlotIndex, nullptr);
	return true;
}

int32 USXInventoryComponent::GetAmmoCount(ESXAmmoType AmmoType) const
{
	const int32* Count = AmmoCounts.Find(AmmoType);
	return Count != nullptr ? *Count : 0;
}

bool USXInventoryComponent::HasAmmo(ESXAmmoType AmmoType, int32 Amount) const
{
	return Amount <= 0 || GetAmmoCount(AmmoType) >= Amount;
}

TArray<FSXInventoryAmmoStack> USXInventoryComponent::GetAmmoStacks() const
{
	TArray<FSXInventoryAmmoStack> Result;
	Result.Reserve(AmmoCounts.Num());

	for (const TPair<ESXAmmoType, int32>& AmmoPair : AmmoCounts)
	{
		FSXInventoryAmmoStack Stack;
		Stack.AmmoType = AmmoPair.Key;
		Stack.Count = AmmoPair.Value;
		Result.Add(Stack);
	}

	Result.Sort([](const FSXInventoryAmmoStack& Left, const FSXInventoryAmmoStack& Right)
	{
		return static_cast<uint8>(Left.AmmoType) < static_cast<uint8>(Right.AmmoType);
	});

	return Result;
}

void USXInventoryComponent::SetAmmoCount(ESXAmmoType AmmoType, int32 Count)
{
	const int32 NewCount = FMath::Max(0, Count);
	int32& StoredCount = AmmoCounts.FindOrAdd(AmmoType);
	if (StoredCount == NewCount)
	{
		return;
	}

	StoredCount = NewCount;
	OnAmmoChanged.Broadcast(AmmoType, StoredCount);
}

int32 USXInventoryComponent::AddAmmo(ESXAmmoType AmmoType, int32 Amount)
{
	if (Amount <= 0)
	{
		return GetAmmoCount(AmmoType);
	}

	const int32 NewCount = GetAmmoCount(AmmoType) + Amount;
	SetAmmoCount(AmmoType, NewCount);
	return NewCount;
}

bool USXInventoryComponent::ConsumeAmmo(ESXAmmoType AmmoType, int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}

	const int32 CurrentCount = GetAmmoCount(AmmoType);
	if (CurrentCount < Amount)
	{
		return false;
	}

	SetAmmoCount(AmmoType, CurrentCount - Amount);
	return true;
}

bool USXInventoryComponent::HasSkill(USXSkillData* SkillData) const
{
	if (IsValid(SkillData) == false)
	{
		return false;
	}

	for (const TObjectPtr<USXSkillData>& OwnedSkill : OwnedSkills)
	{
		if (OwnedSkill.Get() == SkillData)
		{
			return true;
		}
	}

	return false;
}

TArray<USXSkillData*> USXInventoryComponent::GetOwnedSkills() const
{
	TArray<USXSkillData*> Result;
	Result.Reserve(OwnedSkills.Num());

	for (const TObjectPtr<USXSkillData>& SkillData : OwnedSkills)
	{
		if (IsValid(SkillData.Get()))
		{
			Result.Add(SkillData.Get());
		}
	}

	return Result;
}

bool USXInventoryComponent::IsSkillInventoryFull() const
{
	return MaxOwnedSkillSlots > 0 && OwnedSkills.Num() >= MaxOwnedSkillSlots;
}

bool USXInventoryComponent::AddSkill(USXSkillData* SkillData)
{
	if (IsValid(SkillData) == false)
	{
		return false;
	}

	if (HasSkill(SkillData))
	{
		return true;
	}

	if (IsSkillInventoryFull())
	{
		return false;
	}

	OwnedSkills.Add(SkillData);
	OnSkillsChanged.Broadcast();
	return true;
}

bool USXInventoryComponent::RemoveSkill(USXSkillData* SkillData)
{
	if (IsValid(SkillData) == false)
	{
		return false;
	}

	const int32 RemovedCount = OwnedSkills.RemoveAll([SkillData](const TObjectPtr<USXSkillData>& OwnedSkill)
	{
		return OwnedSkill.Get() == SkillData;
	});
	if (RemovedCount <= 0)
	{
		return false;
	}

	OnSkillsChanged.Broadcast();
	return true;
}

bool USXInventoryComponent::HasAugment(FName AugmentId) const
{
	return AugmentId.IsNone() == false && OwnedAugments.Contains(AugmentId);
}

bool USXInventoryComponent::IsAugmentInventoryFull() const
{
	return MaxOwnedAugmentSlots > 0 && OwnedAugments.Num() >= MaxOwnedAugmentSlots;
}

bool USXInventoryComponent::AddAugment(FName AugmentId)
{
	if (AugmentId.IsNone())
	{
		return false;
	}

	if (OwnedAugments.Contains(AugmentId))
	{
		return true;
	}

	if (IsAugmentInventoryFull())
	{
		return false;
	}

	OwnedAugments.Add(AugmentId);
	OnAugmentsChanged.Broadcast();
	return true;
}

bool USXInventoryComponent::RemoveAugment(FName AugmentId)
{
	if (AugmentId.IsNone())
	{
		return false;
	}

	const int32 RemovedCount = OwnedAugments.Remove(AugmentId);
	if (RemovedCount <= 0)
	{
		return false;
	}

	OnAugmentsChanged.Broadcast();
	return true;
}

void USXInventoryComponent::NormalizeWeaponSlots()
{
	MaxWeaponSlots = FMath::Max(1, MaxWeaponSlots);
	WeaponSlots.SetNum(MaxWeaponSlots);
}

void USXInventoryComponent::InitializeAmmoStacks()
{
	if (const UEnum* AmmoEnum = StaticEnum<ESXAmmoType>())
	{
		for (int32 EnumIndex = 0; EnumIndex < AmmoEnum->NumEnums(); ++EnumIndex)
		{
			const FString EnumName = AmmoEnum->GetNameStringByIndex(EnumIndex);
			if (EnumName.IsEmpty() || EnumName.EndsWith(TEXT("_MAX")))
			{
				continue;
			}

			const int64 EnumValue = AmmoEnum->GetValueByIndex(EnumIndex);
			if (EnumValue < 0)
			{
				continue;
			}

			const ESXAmmoType AmmoType = static_cast<ESXAmmoType>(EnumValue);
			int32& Count = AmmoCounts.FindOrAdd(AmmoType);
			Count = FMath::Max(0, Count);
		}
	}
}
