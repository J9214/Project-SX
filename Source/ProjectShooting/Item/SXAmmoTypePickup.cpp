// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/SXAmmoTypePickup.h"

#include "Character/SXPlayerCharacter.h"
#include "Components/SXInventoryComponent.h"
#include "Components/SXPickupComponent.h"
#include "Item/SXWeapon.h"

ASXAmmoTypePickup::ASXAmmoTypePickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupComponent = CreateDefaultSubobject<USXPickupComponent>(TEXT("PickupComponent"));
	SetRootComponent(PickupComponent);
	PickupComponent->SetPickupMethod(ESXPickupMethod::Interaction);
}

void ASXAmmoTypePickup::BeginPlay()
{
	Super::BeginPlay();

	PickupComponent->OnPickUp.AddDynamic(this, &ThisClass::HandleOnPickUp);
}

void ASXAmmoTypePickup::HandleOnPickUp(ASXPlayerCharacter* InPickUpCharacter)
{
	if (IsValid(InPickUpCharacter) == false)
	{
		return;
	}

	USXInventoryComponent* InventoryComponent = InPickUpCharacter->GetInventoryComponent();
	if (IsValid(InventoryComponent) == false)
	{
		return;
	}

	InventoryComponent->AddAmmo(DesiredAmmoType, AmmoAmount);

	ASXWeapon* CurrentWeapon = InPickUpCharacter->GetCurrentWeapon();
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->BroadcastAmmoChanged();
	}

	UE_LOG(LogTemp, Log, TEXT("%s picked up ammo %s x%d. NewCount=%d"),
		*GetNameSafe(InPickUpCharacter),
		*UEnum::GetValueAsString(DesiredAmmoType),
		AmmoAmount,
		InventoryComponent->GetAmmoCount(DesiredAmmoType));

	if (bDestroyOnPickup)
	{
		Destroy();
	}
}
