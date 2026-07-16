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
class USXPickupComponent;
class USXSkillComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXOnInteractionTargetChangedSignature, UObject*, InteractableObject);

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

	UFUNCTION(BlueprintCallable, Category="SX|Interaction")
	void SetInteractionCandidate(UObject* NewInteractionCandidate);

	UFUNCTION(BlueprintCallable, Category="SX|Interaction")
	void ClearInteractionCandidate(UObject* InteractionCandidateToClear);

	UFUNCTION(BlueprintCallable, Category="SX|Interaction")
	void SetPickupCandidate(USXPickupComponent* NewPickupCandidate);

	UFUNCTION(BlueprintCallable, Category="SX|Interaction")
	void ClearPickupCandidate(USXPickupComponent* PickupCandidateToClear);

	UFUNCTION(BlueprintPure, Category="SX|Interaction")
	UObject* GetCurrentInteractionCandidate() const { return CurrentInteractionCandidate; }

	UFUNCTION(BlueprintPure, Category="SX|Interaction")
	USXPickupComponent* GetCurrentPickupCandidate() const;

	UPROPERTY(BlueprintAssignable, Category="SX|Interaction")
	FSXOnInteractionTargetChangedSignature OnInteractionTargetChanged;

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
	void InputMovementSkill();
	void InputSkill1();
	void InputSkill2();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USXSkillComponent> SkillComponent;

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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Interaction")
	TObjectPtr<UObject> CurrentInteractionCandidate;

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
