// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SXCharacterBase.h"
#include "SXPlayerCharacter.generated.h"

struct FInputActionValue;
class UCameraComponent;
class UInputAction;
class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraShakeBase;
class USXInputConfig;

UENUM(BlueprintType)
enum class EViewMode : uint8
{
	None,
	FirstPersonView,
	ThirdPersonView,
	BackView,
	End
};

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXPlayerCharacter : public ASXCharacterBase
{
	GENERATED_BODY()

public:
	ASXPlayerCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartJump();
	void StopJump();
	void StartSprint();
	void StopSprint();
	void StartFire();
	void StopFire();
	void AttackMelee();
	void Reload();
	void Interact();

	UFUNCTION(BlueprintCallable, Category="SX|View")
	void ChangeView();

	UFUNCTION(BlueprintCallable, Category="SX|View")
	void SetViewMode(EViewMode InViewMode);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SX|Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	FVector FirstPersonCameraLocation = FVector(0.0f, 0.0f, 64.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	float ThirdPersonArmLength = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	FVector ThirdPersonPivotLocation = FVector(0.0f, 0.0f, 64.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	FVector ThirdPersonCameraOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input", meta = (AllowPrivateAccess))
	TObjectPtr<USXInputConfig> PlayerCharacterInputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	EViewMode DefaultViewMode = EViewMode::BackView;

	EViewMode CurrentViewMode = EViewMode::None;

#pragma region Effect
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UCameraShakeBase> AttackFireCameraShake;
#pragma endregion

#pragma region Selector
public:
	bool bIsFullAutoFire = false;

	FTimerHandle FullAutoTimerHandle;

	float TimeBetweenFire;

protected:
	void InputToggleSelector(const FInputActionValue& InValue);

	void TryFire();
#pragma endregion

#pragma region IronSight
protected:
	void IronSight();

	bool Zoomed = false;

	float TargetFOV = 70.f;

	float CurrentFOV = 70.f;

#pragma endregion
};
