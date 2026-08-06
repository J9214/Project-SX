// Fill out your copyright notice in the Description page of Project Settings.

#include "Shop/SXShopActor.h"

#include "Character/SXPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SXInventoryComponent.h"
#include "Components/SXStatusComponent.h"
#include "Item/SXWeapon.h"
#include "Shop/SXShopDataAsset.h"
#include "Shop/SXShopItemData.h"

ASXShopActor::ASXShopActor()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollisionComponent"));
	SetRootComponent(InteractionCollisionComponent);
	InteractionCollisionComponent->SetBoxExtent(FVector(120.0f, 120.0f, 120.0f));
	InteractionCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionCollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ASXShopActor::BeginPlay()
{
	Super::BeginPlay();

	InteractionCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleInteractionBeginOverlap);
	InteractionCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleInteractionEndOverlap);
}

bool ASXShopActor::CanInteract_Implementation(ASXPlayerCharacter* InteractingCharacter) const
{
	return IsValid(InteractingCharacter) == true && IsValid(ShopData.Get()) == true;
}

void ASXShopActor::Interact_Implementation(ASXPlayerCharacter* InteractingCharacter)
{
	if (ISXInteractableInterface::Execute_CanInteract(this, InteractingCharacter) == false)
	{
		return;
	}

	OnShopOpened.Broadcast(InteractingCharacter, ShopData);
	BP_OnShopOpened(InteractingCharacter, ShopData);
}

FText ASXShopActor::GetInteractionText_Implementation() const
{
	return InteractionText;
}

bool ASXShopActor::TryPurchaseItem(ASXPlayerCharacter* Buyer, USXShopItemData* ShopItemData)
{
	if (IsValid(Buyer) == false || IsValid(ShopItemData) == false)
	{
		return false;
	}

	if (ShopItemData->ItemType == ESXShopItemType::AmmoType)
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop item %s uses deprecated AmmoType purchase. Ammo type is fixed per weapon now."),
			*GetNameSafe(ShopItemData));
		return false;
	}

	USXStatusComponent* StatusComponent = Buyer->GetStatusComponent();
	if (IsValid(StatusComponent) == false || StatusComponent->SpendGold(ShopItemData->Price) == false)
	{
		return false;
	}

	switch (ShopItemData->ItemType)
	{
	case ESXShopItemType::Heal:
		StatusComponent->Heal(ShopItemData->HealAmount, this);
		break;
	case ESXShopItemType::AmmoType:
		break;
	case ESXShopItemType::Ammo:
		if (USXInventoryComponent* InventoryComponent = Buyer->GetInventoryComponent())
		{
			InventoryComponent->AddAmmo(ShopItemData->AmmoType, ShopItemData->AmmoAmount);
		}
		if (ASXWeapon* CurrentWeapon = Buyer->GetCurrentWeapon())
		{
			CurrentWeapon->BroadcastAmmoChanged();
		}
		break;
	case ESXShopItemType::Weapon:
	case ESXShopItemType::Custom:
	default:
		break;
	}

	OnShopItemPurchased.Broadcast(Buyer, ShopData, ShopItemData);
	BP_OnShopItemPurchased(Buyer, ShopData, ShopItemData);
	return true;
}

void ASXShopActor::HandleInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASXPlayerCharacter* PlayerCharacter = Cast<ASXPlayerCharacter>(OtherActor);
	if (IsValid(PlayerCharacter) == true)
	{
		PlayerCharacter->SetInteractionCandidate(this);
	}
}

void ASXShopActor::HandleInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASXPlayerCharacter* PlayerCharacter = Cast<ASXPlayerCharacter>(OtherActor);
	if (IsValid(PlayerCharacter) == true)
	{
		PlayerCharacter->ClearInteractionCandidate(this);
	}
}
