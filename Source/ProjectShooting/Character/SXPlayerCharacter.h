// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SXCharacterBase.h"
#include "UI/SXOptionsSaveGame.h"
#include "SXPlayerCharacter.generated.h"

struct FInputActionValue;
class UCameraComponent;
class UInputAction;
class UAnimMontage;
class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraShakeBase;
class USXInputConfig;
class USXInventoryComponent;
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

	virtual void Die(AActor* DamageCauser) override;

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

	UFUNCTION(BlueprintPure, Category="SX|Inventory")
	USXInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon")
	USkeletalMeshComponent* GetWeaponAttachMesh() const;

	UFUNCTION(BlueprintPure, Category="SX|Weapon")
	USkeletalMeshComponent* FindWeaponAttachMesh(FName PreferredSocketName, FName LegacySocketName) const;

	UFUNCTION(BlueprintCallable, Category="SX|Weapon")
	bool EquipWeaponSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="SX|Weapon")
	bool EquipWeaponClass(TSubclassOf<ASXWeapon> WeaponClass, bool bGrantInitialAmmo = false);

	UFUNCTION(BlueprintCallable, Category="SX|Weapon")
	bool EquipNextWeapon();

	UFUNCTION(BlueprintCallable, Category="SX|Weapon")
	bool EquipPreviousWeapon();

	UFUNCTION(BlueprintCallable, Category="SX|Weapon")
	bool DropCurrentWeapon();

	UFUNCTION(BlueprintPure, Category="SX|Weapon")
	int32 GetCurrentWeaponSlotIndex() const { return CurrentWeaponSlotIndex; }

	UFUNCTION(BlueprintCallable, Category="SX|Weapon")
	void SetCurrentWeaponSlotIndex(int32 SlotIndex) { CurrentWeaponSlotIndex = SlotIndex; }

	UFUNCTION(BlueprintCallable, Category="SX|Options|Gameplay")
	void ApplyGameplayOptions(const FSXOptionsSnapshot& Options);

	UFUNCTION(BlueprintCallable, Category="SX|Options|Gameplay")
	void ReloadGameplayOptions();

	UFUNCTION(BlueprintCallable, Category="SX|Weapon")
	bool AddPickedWeaponClassToInventory(TSubclassOf<ASXWeapon> WeaponClass, int32& OutSlotIndex);

	bool AddPickedWeaponClassToInventory(TSubclassOf<ASXWeapon> WeaponClass, int32& OutSlotIndex, TSubclassOf<ASXWeapon>& OutReplacedWeaponClass);

	void DropWeaponClass(TSubclassOf<ASXWeapon> WeaponClass, const FVector& DropSourceLocation);

	UPROPERTY(BlueprintAssignable, Category="SX|Interaction")
	FSXOnInteractionTargetChangedSignature OnInteractionTargetChanged;

protected:
	virtual void BeginPlay() override;

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
	void InputWeaponSlot1();
	void InputWeaponSlot2();
	void InputWeaponSlot3();
	void InputNextWeapon();
	void InputPreviousWeapon();
	void InputDropWeapon();
	void StartIronSightInput();
	void StopIronSightInput();

	UFUNCTION(BlueprintCallable, Category="SX|View")
	void ChangeView();

	UFUNCTION(BlueprintCallable, Category="SX|View")
	void SetViewMode(EViewMode InViewMode);

	void ReattachCurrentWeaponToViewMesh();
	void EquipDefaultWeapon();
	bool EquipWeaponClassInternal(TSubclassOf<ASXWeapon> WeaponClass, int32 SlotIndex, bool bGrantInitialAmmo);
	bool EquipWeaponByOffset(int32 SlotOffset);
	int32 ChoosePickedWeaponSlot(TSubclassOf<ASXWeapon> WeaponClass) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SX|Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USXSkillComponent> SkillComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USXInventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|Default")
	TSubclassOf<ASXWeapon> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|Default")
	bool bEquipDefaultWeaponOnBeginPlay = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|Default", meta=(ClampMin="0"))
	int32 DefaultWeaponSlotIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|Drop")
	bool bCanDropDefaultWeaponSlot = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Weapon")
	int32 CurrentWeaponSlotIndex = INDEX_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	FVector FirstPersonCameraLocation = FVector(0.0f, 0.0f, 64.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	float ThirdPersonArmLength = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	FVector ThirdPersonPivotLocation = FVector(0.0f, 0.0f, 64.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	FVector ThirdPersonCameraOffset = FVector(0.0f, -30.0f, 30.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	FVector ThirdPersonCameraLocalLocation = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	FRotator ThirdPersonCameraLocalRotation = FRotator(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SX|Input", meta = (AllowPrivateAccess))
	TObjectPtr<USXInputConfig> PlayerCharacterInputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|View")
	EViewMode DefaultViewMode = EViewMode::ThirdPersonView;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Death")
	TObjectPtr<UAnimMontage> DeathMontage;

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
	void SetIronSightZoomed(bool bNewZoomed);

	bool Zoomed = false;

	float TargetFOV = 70.f;

	float CurrentFOV = 70.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Options|Gameplay")
	FSXOptionsSnapshot GameplayOptions;

#pragma endregion
};
