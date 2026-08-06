// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/SXShopWidget.h"

#include "Character/SXPlayerCharacter.h"
#include "Components/SXStatusComponent.h"
#include "GameFramework/PlayerController.h"
#include "Shop/SXShopActor.h"
#include "Shop/SXShopDataAsset.h"
#include "Shop/SXShopItemData.h"

void USXShopWidget::InitShop(ASXShopActor* InShopActor, ASXPlayerCharacter* InBuyer)
{
	UnbindBuyer();

	ShopActor = InShopActor;
	Buyer = InBuyer;

	if (IsValid(InShopActor))
	{
		SetShopData(InShopActor->GetShopData());
	}

	if (IsValid(InBuyer))
	{
		if (USXStatusComponent* StatusComponent = InBuyer->GetStatusComponent())
		{
			StatusComponent->OnGoldChanged.AddUniqueDynamic(this, &ThisClass::HandleGoldChanged);
			OnCreditsChanged.Broadcast(StatusComponent->GetGold());
		}
	}
}

void USXShopWidget::SetShopData(USXShopDataAsset* InShopData)
{
	if (ShopData == InShopData)
	{
		return;
	}

	ShopData = InShopData;
	SelectedItem = nullptr;

	OnShopDataChanged.Broadcast(ShopData);
	OnSelectionChanged.Broadcast(nullptr);
}

TArray<USXShopItemData*> USXShopWidget::GetShopItems() const
{
	TArray<USXShopItemData*> Result;
	if (IsValid(ShopData))
	{
		Result.Reserve(ShopData->ShopItems.Num());
		for (USXShopItemData* Item : ShopData->ShopItems)
		{
			if (IsValid(Item))
			{
				Result.Add(Item);
			}
		}
	}

	return Result;
}

void USXShopWidget::SelectItem(USXShopItemData* Item)
{
	if (SelectedItem == Item)
	{
		return;
	}

	SelectedItem = Item;
	OnSelectionChanged.Broadcast(SelectedItem);
}

bool USXShopWidget::TryPurchaseItem(USXShopItemData* Item)
{
	const bool bSucceeded = IsValid(Item)
		&& ShopActor.IsValid()
		&& Buyer.IsValid()
		&& ShopActor->TryPurchaseItem(Buyer.Get(), Item);

	OnPurchaseCompleted.Broadcast(Item, bSucceeded);
	return bSucceeded;
}

bool USXShopWidget::PurchaseSelectedItem()
{
	return TryPurchaseItem(SelectedItem);
}

void USXShopWidget::CloseShop(bool bRestoreGameInput)
{
	if (bRestoreGameInput)
	{
		APlayerController* PlayerController = GetOwningPlayer();
		if (IsValid(PlayerController) == false && Buyer.IsValid())
		{
			PlayerController = Cast<APlayerController>(Buyer->GetController());
		}

		if (IsValid(PlayerController))
		{
			PlayerController->SetInputMode(FInputModeGameOnly());
			PlayerController->bShowMouseCursor = false;
		}
	}

	RemoveFromParent();
}

int32 USXShopWidget::GetCredits() const
{
	if (Buyer.IsValid())
	{
		if (const USXStatusComponent* StatusComponent = Buyer->GetStatusComponent())
		{
			return StatusComponent->GetGold();
		}
	}

	return 0;
}

bool USXShopWidget::CanAffordItem(USXShopItemData* Item) const
{
	return IsValid(Item) && Item->Price >= 0 && GetCredits() >= Item->Price;
}

void USXShopWidget::NativeDestruct()
{
	UnbindBuyer();
	Super::NativeDestruct();
}

void USXShopWidget::HandleGoldChanged(USXStatusComponent* /*StatusComponent*/, int32 /*OldGold*/, int32 NewGold, int32 /*Delta*/)
{
	OnCreditsChanged.Broadcast(NewGold);
}

void USXShopWidget::UnbindBuyer()
{
	if (Buyer.IsValid())
	{
		if (USXStatusComponent* StatusComponent = Buyer->GetStatusComponent())
		{
			StatusComponent->OnGoldChanged.RemoveDynamic(this, &ThisClass::HandleGoldChanged);
		}
	}
}
