// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SXShopWidget.generated.h"

class ASXPlayerCharacter;
class ASXShopActor;
class USXShopDataAsset;
class USXShopItemData;
class USXStatusComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXOnShopDataChangedSignature, USXShopDataAsset*, ShopData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXOnShopSelectionChangedSignature, USXShopItemData*, SelectedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSXOnShopPurchaseCompletedSignature, USXShopItemData*, Item, bool, bSucceeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXOnShopCreditsChangedSignature, int32, Credits);

/**
 * Functionality-only base class for WBP_Shop.
 *
 * This class deliberately creates no widgets and applies no visual styling. Build the
 * complete hierarchy in the Widget Blueprint, then call the public functions below
 * from its buttons and item-entry widgets.
 */
UCLASS(Abstract, Blueprintable)
class PROJECTSHOOTING_API USXShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Connects this widget to the shop and buyer that opened it. */
	UFUNCTION(BlueprintCallable, Category="SX|Shop")
	void InitShop(ASXShopActor* InShopActor, ASXPlayerCharacter* InBuyer);

	UFUNCTION(BlueprintCallable, Category="SX|Shop")
	void SetShopData(USXShopDataAsset* InShopData);

	/** Returns the catalogue in its data-asset order for Blueprint-driven list creation. */
	UFUNCTION(BlueprintPure, Category="SX|Shop")
	TArray<USXShopItemData*> GetShopItems() const;

	UFUNCTION(BlueprintCallable, Category="SX|Shop")
	void SelectItem(USXShopItemData* Item);

	/** Attempts to purchase an explicit item and broadcasts OnPurchaseCompleted. */
	UFUNCTION(BlueprintCallable, Category="SX|Shop")
	bool TryPurchaseItem(USXShopItemData* Item);

	/** Attempts to purchase the item most recently passed to SelectItem. */
	UFUNCTION(BlueprintCallable, Category="SX|Shop")
	bool PurchaseSelectedItem();

	/** Removes the widget and optionally restores game-only input. */
	UFUNCTION(BlueprintCallable, Category="SX|Shop")
	void CloseShop(bool bRestoreGameInput = true);

	UFUNCTION(BlueprintPure, Category="SX|Shop")
	USXShopDataAsset* GetShopData() const { return ShopData; }

	UFUNCTION(BlueprintPure, Category="SX|Shop")
	USXShopItemData* GetSelectedItem() const { return SelectedItem; }

	UFUNCTION(BlueprintPure, Category="SX|Shop")
	ASXShopActor* GetShopActor() const { return ShopActor.Get(); }

	UFUNCTION(BlueprintPure, Category="SX|Shop")
	ASXPlayerCharacter* GetBuyer() const { return Buyer.Get(); }

	UFUNCTION(BlueprintPure, Category="SX|Shop")
	int32 GetCredits() const;

	UFUNCTION(BlueprintPure, Category="SX|Shop")
	bool CanAffordItem(USXShopItemData* Item) const;

	UPROPERTY(BlueprintAssignable, Category="SX|Shop")
	FSXOnShopDataChangedSignature OnShopDataChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|Shop")
	FSXOnShopSelectionChangedSignature OnSelectionChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|Shop")
	FSXOnShopPurchaseCompletedSignature OnPurchaseCompleted;

	UPROPERTY(BlueprintAssignable, Category="SX|Shop")
	FSXOnShopCreditsChangedSignature OnCreditsChanged;

protected:
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleGoldChanged(USXStatusComponent* StatusComponent, int32 OldGold, int32 NewGold, int32 Delta);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop", meta=(ExposeOnSpawn="true"))
	TObjectPtr<USXShopDataAsset> ShopData;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Shop")
	TObjectPtr<USXShopItemData> SelectedItem;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Shop")
	TWeakObjectPtr<ASXShopActor> ShopActor;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Shop")
	TWeakObjectPtr<ASXPlayerCharacter> Buyer;

private:
	void UnbindBuyer();
};
