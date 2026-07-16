// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/SXAmmoTypePickup.h"

#include "Character/SXPlayerCharacter.h"
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

	ASXWeapon* CurrentWeapon = InPickUpCharacter->GetCurrentWeapon();
	if (IsValid(CurrentWeapon) == false)
	{
		return;
	}

	CurrentWeapon->SetCurrentAmmoType(DesiredAmmoType);

	if (bDestroyOnPickup)
	{
		Destroy();
	}
}
