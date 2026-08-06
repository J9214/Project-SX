// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXHealthPickup.generated.h"

class ASXPlayerCharacter;
class UParticleSystem;
class USoundBase;
class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSXOnHealthPickupConsumedSignature, ASXPlayerCharacter*, PickUpCharacter, float, RequestedHealAmount, float, AppliedHealAmount);

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXHealthPickup : public AActor
{
	GENERATED_BODY()

public:
	ASXHealthPickup();

	UFUNCTION(BlueprintCallable, Category="SX|Health Pickup")
	bool TryConsume(ASXPlayerCharacter* PickUpCharacter);

	UFUNCTION(BlueprintPure, Category="SX|Health Pickup")
	float GetHealAmount() const { return HealAmount; }

	UPROPERTY(BlueprintAssignable, Category="SX|Health Pickup")
	FSXOnHealthPickupConsumedSignature OnHealthPickupConsumed;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePickupBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION(BlueprintImplementableEvent, Category="SX|Health Pickup", meta=(DisplayName="On Health Pickup Consumed"))
	void BP_OnHealthPickupConsumed(ASXPlayerCharacter* PickUpCharacter, float RequestedHealAmount, float AppliedHealAmount);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Health Pickup", meta=(ClampMin="0.0"))
	float HealAmount = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Health Pickup")
	bool bDestroyOnConsumed = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Health Pickup|Feedback")
	TObjectPtr<UParticleSystem> PickupVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Health Pickup|Feedback")
	TObjectPtr<USoundBase> PickupSFX;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Health Pickup")
	bool bConsumed = false;
};
