// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/SXInteractableInterface.h"
#include "SXShopActor.generated.h"

class ASXPlayerCharacter;
class UBoxComponent;
class USXShopDataAsset;
class USXShopItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSXOnShopOpenedSignature, ASXPlayerCharacter*, InteractingCharacter, USXShopDataAsset*, ShopData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSXOnShopItemPurchasedSignature, ASXPlayerCharacter*, Buyer, USXShopDataAsset*, ShopData, USXShopItemData*, ShopItemData);

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXShopActor : public AActor, public ISXInteractableInterface
{
	GENERATED_BODY()

public:
	ASXShopActor();

	virtual bool CanInteract_Implementation(ASXPlayerCharacter* InteractingCharacter) const override;

	virtual void Interact_Implementation(ASXPlayerCharacter* InteractingCharacter) override;

	virtual FText GetInteractionText_Implementation() const override;

	UFUNCTION(BlueprintPure, Category="SX|Shop")
	USXShopDataAsset* GetShopData() const { return ShopData; }

	UFUNCTION(BlueprintCallable, Category="SX|Shop")
	bool TryPurchaseItem(ASXPlayerCharacter* Buyer, USXShopItemData* ShopItemData);

	UFUNCTION(BlueprintImplementableEvent, Category="SX|Shop")
	void BP_OnShopOpened(ASXPlayerCharacter* InteractingCharacter, USXShopDataAsset* InShopData);

	UFUNCTION(BlueprintImplementableEvent, Category="SX|Shop")
	void BP_OnShopItemPurchased(ASXPlayerCharacter* Buyer, USXShopDataAsset* InShopData, USXShopItemData* ShopItemData);

	UPROPERTY(BlueprintAssignable, Category="SX|Shop")
	FSXOnShopOpenedSignature OnShopOpened;

	UPROPERTY(BlueprintAssignable, Category="SX|Shop")
	FSXOnShopItemPurchasedSignature OnShopItemPurchased;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UBoxComponent> InteractionCollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop")
	TObjectPtr<USXShopDataAsset> ShopData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Shop")
	FText InteractionText = FText::FromString(TEXT("[E] Shop"));
};
