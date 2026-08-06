// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/SXCollectiblePickup.h"

#include "Character/SXPlayerCharacter.h"
#include "Components/SXPickupComponent.h"
#include "Components/SXInventoryComponent.h"
#include "Components/SXStatusComponent.h"
#include "Item/SXWeapon.h"

ASXCollectiblePickup::ASXCollectiblePickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupComponent = CreateDefaultSubobject<USXPickupComponent>(TEXT("PickupComponent"));
	SetRootComponent(PickupComponent);
	PickupComponent->SetPickupMethod(ESXPickupMethod::Magnet);
}

void ASXCollectiblePickup::BeginPlay()
{
	Super::BeginPlay();

	PickupComponent->OnPickUp.AddDynamic(this, &ThisClass::HandleOnPickUp);
}

void ASXCollectiblePickup::InitializeCollectible(ESXCollectibleType InCollectibleType, int32 InAmount)
{
	CollectibleType = InCollectibleType;
	Amount = FMath::Max(1, InAmount);
}

void ASXCollectiblePickup::InitializeAmmoCollectible(ESXAmmoType InAmmoType, int32 InAmount)
{
	CollectibleType = ESXCollectibleType::Ammo;
	AmmoType = InAmmoType;
	Amount = FMath::Max(1, InAmount);
}

void ASXCollectiblePickup::HandleOnPickUp(ASXPlayerCharacter* InPickUpCharacter)
{
	if (IsValid(InPickUpCharacter) == false)
	{
		return;
	}

	USXStatusComponent* StatusComponent = InPickUpCharacter->GetStatusComponent();
	if (IsValid(StatusComponent) == true)
	{
		switch (CollectibleType)
		{
		case ESXCollectibleType::Experience:
			StatusComponent->AddExperience(Amount);
			break;
		case ESXCollectibleType::Ammo:
			if (USXInventoryComponent* InventoryComponent = InPickUpCharacter->GetInventoryComponent())
			{
				InventoryComponent->AddAmmo(AmmoType, Amount);
			}
			if (ASXWeapon* CurrentWeapon = InPickUpCharacter->GetCurrentWeapon())
			{
				CurrentWeapon->BroadcastAmmoChanged();
			}
			break;
		case ESXCollectibleType::Gold:
		default:
			StatusComponent->AddGold(Amount);
			break;
		}
	}

	OnCollectiblePickedUp.Broadcast(InPickUpCharacter, CollectibleType, Amount);
	BP_OnCollected(InPickUpCharacter, CollectibleType, Amount);

	UE_LOG(LogTemp, Log, TEXT("%s picked up %s x%d"),
		*GetNameSafe(InPickUpCharacter),
		*UEnum::GetValueAsString(CollectibleType),
		Amount);

	if (bDestroyOnPickup)
	{
		Destroy();
	}
}
