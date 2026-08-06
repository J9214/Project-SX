// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Item/SXWeapon.h"
#include "SXAnimInstance.generated.h"

class ASXCharacterBase;
class UCharacterMovementComponent;


/**
 * 
 */
UCLASS()
class PROJECTSHOOTING_API USXAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	void CacheOwnerReferences();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SX|Animation")
	TObjectPtr<ASXCharacterBase> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SX|Animation")
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SX|Animation")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SX|Animation")
	float GroundSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SX|Animation")
	float Direction = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SX|Animation")
	bool ShouldMove = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SX|Animation")
	bool IsFalling = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SX|Animation")
	bool IsUnarmed = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Animation")
	ESXWeaponType WeaponType = ESXWeaponType::Unarmed;

	UPROPERTY(BlueprintReadOnly)
	float NormalizedCurrentPitch;
};
