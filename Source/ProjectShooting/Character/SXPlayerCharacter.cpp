// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/SXPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "ProjectShooting.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/SXAnimInstance.h"
#include "Item/SXWeapon.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SXPickupComponent.h"
#include "Components/SXStatusComponent.h"
#include "Components/SXInventoryComponent.h"
#include "Input/SXInputConfig.h"
#include "Interaction/SXInteractableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Skill/SXSkillComponent.h"

ASXPlayerCharacter::ASXPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 96.0f);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FirstPersonCameraLocation);
	CameraComponent->bUsePawnControlRotation = true;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(CameraComponent);
	SkeletalMeshComponent->SetOnlyOwnerSee(true);
	SkeletalMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	SkeletalMeshComponent->SetCastShadow(false);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->SetRelativeLocation(ThirdPersonPivotLocation);
	SpringArmComponent->TargetArmLength = ThirdPersonArmLength;
	SpringArmComponent->SocketOffset = ThirdPersonCameraOffset;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->bInheritPitch = true;
	SpringArmComponent->bInheritYaw = true;
	SpringArmComponent->bInheritRoll = false;
	SpringArmComponent->bDoCollisionTest = true;
	SpringArmComponent->SetActive(false);

	SkillComponent = CreateDefaultSubobject<USXSkillComponent>(TEXT("SkillComponent"));
	InventoryComponent = CreateDefaultSubobject<USXInventoryComponent>(TEXT("InventoryComponent"));

	GetMesh()->SetOwnerNoSee(true);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->MaxWalkSpeed = GetStatusComponent()->GetWalkSpeed();
	MovementComponent->BrakingDecelerationFalling = 1500.0f;
	MovementComponent->AirControl = 0.5f;
	MovementComponent->bOrientRotationToMovement = true;
	MovementComponent->RotationRate = FRotator(0.0f, 720.0f, 0.0f);

	bUseDeathDissolve = true;
	bDestroyAfterDeathDissolve = false;

	TimeBetweenFire = GetStatusComponent()->GetTimeBetweenFire();
}

void ASXPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetViewMode(DefaultViewMode);
	EquipDefaultWeapon();
	ReloadGameplayOptions();

	GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::ReattachCurrentWeaponToViewMesh);
}

void ASXPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaSeconds, 35.f);
	CameraComponent->SetFieldOfView(CurrentFOV);
}

void ASXPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (CurrentViewMode == EViewMode::None)
	{
		SetViewMode(DefaultViewMode);
	}
	else
	{
		ReattachCurrentWeaponToViewMesh();
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IsValid(EnhancedInputComponent))
	{
		return;
	}

	if (IsValid(PlayerCharacterInputConfig.Get()) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no PlayerCharacterInputConfig."), *GetName());
		return;
	}

	if (PlayerCharacterInputConfig->MoveAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
	if (PlayerCharacterInputConfig->LookAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
	if (PlayerCharacterInputConfig->JumpAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->JumpAction, ETriggerEvent::Started, this, &ThisClass::StartJump);
	if (PlayerCharacterInputConfig->JumpAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJump);
	if (PlayerCharacterInputConfig->SprintAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->SprintAction, ETriggerEvent::Started, this, &ThisClass::StartSprint);
	if (PlayerCharacterInputConfig->SprintAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->SprintAction, ETriggerEvent::Completed, this, &ThisClass::StopSprint);
	if (PlayerCharacterInputConfig->FireAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->FireAction, ETriggerEvent::Started, this, &ThisClass::StartFire);
	if (PlayerCharacterInputConfig->FireAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->FireAction, ETriggerEvent::Completed, this, &ThisClass::StopFire);
	if (PlayerCharacterInputConfig->AttackMeleeAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->AttackMeleeAction, ETriggerEvent::Started, this, &ThisClass::AttackMelee);
	if (PlayerCharacterInputConfig->ReloadAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->ReloadAction, ETriggerEvent::Started, this, &ThisClass::Reload);
	if (PlayerCharacterInputConfig->InteractAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->InteractAction, ETriggerEvent::Started, this, &ThisClass::Interact);
	if (PlayerCharacterInputConfig->ChangeViewAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->ChangeViewAction, ETriggerEvent::Started, this, &ThisClass::ChangeView);
	if (PlayerCharacterInputConfig->ToggleSelectorAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->ToggleSelectorAction, ETriggerEvent::Started, this, &ThisClass::InputToggleSelector);
	if (PlayerCharacterInputConfig->IronSightAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->IronSightAction, ETriggerEvent::Started, this, &ThisClass::StartIronSightInput);
	if (PlayerCharacterInputConfig->IronSightAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->IronSightAction, ETriggerEvent::Completed, this, &ThisClass::StopIronSightInput);
	if (PlayerCharacterInputConfig->WeaponSlot1Action) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->WeaponSlot1Action, ETriggerEvent::Started, this, &ThisClass::InputWeaponSlot1);
	if (PlayerCharacterInputConfig->WeaponSlot2Action) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->WeaponSlot2Action, ETriggerEvent::Started, this, &ThisClass::InputWeaponSlot2);
	if (PlayerCharacterInputConfig->WeaponSlot3Action) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->WeaponSlot3Action, ETriggerEvent::Started, this, &ThisClass::InputWeaponSlot3);
	if (PlayerCharacterInputConfig->NextWeaponAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->NextWeaponAction, ETriggerEvent::Started, this, &ThisClass::InputNextWeapon);
	if (PlayerCharacterInputConfig->PreviousWeaponAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->PreviousWeaponAction, ETriggerEvent::Started, this, &ThisClass::InputPreviousWeapon);
	if (PlayerCharacterInputConfig->DropWeaponAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->DropWeaponAction, ETriggerEvent::Started, this, &ThisClass::InputDropWeapon);
	if (PlayerCharacterInputConfig->MovementSkillAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->MovementSkillAction, ETriggerEvent::Started, this, &ThisClass::InputMovementSkill);
	if (PlayerCharacterInputConfig->Skill1Action) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Skill1Action, ETriggerEvent::Started, this, &ThisClass::InputSkill1);
	if (PlayerCharacterInputConfig->Skill2Action) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Skill2Action, ETriggerEvent::Started, this, &ThisClass::InputSkill2);
}

void ASXPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller || !IsAlive())
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASXPlayerCharacter::Look(const FInputActionValue& Value)
{
	if (!Controller || !IsAlive())
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	const float Sensitivity = FMath::Max(0.01f, GameplayOptions.MouseSensitivity);
	const float InvertYMultiplier = GameplayOptions.bInvertLookY ? -1.0f : 1.0f;

	AddControllerYawInput(LookAxisVector.X * Sensitivity);
	AddControllerPitchInput(LookAxisVector.Y * Sensitivity * InvertYMultiplier);
}

void ASXPlayerCharacter::ApplyGameplayOptions(const FSXOptionsSnapshot& Options)
{
	GameplayOptions = Options;
	GameplayOptions.MouseSensitivity = FMath::Max(0.01f, GameplayOptions.MouseSensitivity);
}

void ASXPlayerCharacter::ReloadGameplayOptions()
{
	FSXOptionsSnapshot LoadedOptions;
	if (UGameplayStatics::DoesSaveGameExist(TEXT("SXOptions"), 0))
	{
		if (USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(TEXT("SXOptions"), 0))
		{
			if (const USXOptionsSaveGame* OptionsSaveGame = Cast<USXOptionsSaveGame>(SaveGame))
			{
				LoadedOptions = OptionsSaveGame->Options;
			}
		}
	}

	ApplyGameplayOptions(LoadedOptions);
}

void ASXPlayerCharacter::StartJump()
{
	if (IsAlive())
	{
		Jump();
	}
}

void ASXPlayerCharacter::StopJump()
{
	StopJumping();
}

void ASXPlayerCharacter::StartSprint()
{
	if (IsAlive())
	{
		GetCharacterMovement()->MaxWalkSpeed = GetStatusComponent()->GetSprintSpeed();
	}
}

void ASXPlayerCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = GetStatusComponent()->GetWalkSpeed();
}

void ASXPlayerCharacter::StartFire()
{
	if (!IsAlive())
	{
		return;
	}

	if (IsValid(CurrentWeapon.Get()) == false)
	{
		return;
	}

	if (CurrentWeapon->IsReloading())
	{
		return;
	}

	if (CurrentWeapon->IsFullAutoEnabled())
	{
		TryFire();

		TimeBetweenFire = FMath::Max(0.01f, CurrentWeapon->GetFireInterval());

		GetWorldTimerManager().SetTimer(
			FullAutoTimerHandle,
			this,
			&ThisClass::TryFire,
			TimeBetweenFire,
			true
		);
	}
	else
	{
		TryFire();
	}

}

void ASXPlayerCharacter::StopFire()
{
	GetWorldTimerManager().ClearTimer(FullAutoTimerHandle);
}

void ASXPlayerCharacter::Die(AActor* DamageCauser)
{
	if (bIsDead)
	{
		return;
	}

	StopFire();

	if (IsValid(CurrentWeapon.Get()))
	{
		CurrentWeapon->CancelReload();
	}

	if (IsValid(DeathMontage.Get()))
	{
		PlayAnimMontage(DeathMontage);
	}

	Super::Die(DamageCauser);
}

void ASXPlayerCharacter::Reload()
{
	if (!IsAlive())
	{
		return;
	}

	if (IsValid(CurrentWeapon.Get()) == false)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(FullAutoTimerHandle);
	CurrentWeapon->StartReload(this);
}

void ASXPlayerCharacter::SetInteractionCandidate(UObject* NewInteractionCandidate)
{
	if (CurrentInteractionCandidate == NewInteractionCandidate)
	{
		return;
	}

	if (IsValid(NewInteractionCandidate) == true && NewInteractionCandidate->GetClass()->ImplementsInterface(USXInteractableInterface::StaticClass()) == false)
	{
		return;
	}

	CurrentInteractionCandidate = NewInteractionCandidate;
	OnInteractionTargetChanged.Broadcast(CurrentInteractionCandidate);
}

void ASXPlayerCharacter::ClearInteractionCandidate(UObject* InteractionCandidateToClear)
{
	if (CurrentInteractionCandidate != InteractionCandidateToClear)
	{
		return;
	}

	CurrentInteractionCandidate = nullptr;
	OnInteractionTargetChanged.Broadcast(nullptr);
}

void ASXPlayerCharacter::SetPickupCandidate(USXPickupComponent* NewPickupCandidate)
{
	SetInteractionCandidate(NewPickupCandidate);
}

void ASXPlayerCharacter::ClearPickupCandidate(USXPickupComponent* PickupCandidateToClear)
{
	ClearInteractionCandidate(PickupCandidateToClear);
}

USXPickupComponent* ASXPlayerCharacter::GetCurrentPickupCandidate() const
{
	return Cast<USXPickupComponent>(CurrentInteractionCandidate);
}

USkeletalMeshComponent* ASXPlayerCharacter::GetWeaponAttachMesh() const
{
	if (CurrentViewMode == EViewMode::FirstPersonView && IsValid(SkeletalMeshComponent.Get()))
	{
		const bool bHasFirstPersonWeaponSocket = SkeletalMeshComponent->DoesSocketExist(TEXT("WeaponSocket"))
			|| SkeletalMeshComponent->DoesSocketExist(TEXT("Weapon_Socket"));
		if (bHasFirstPersonWeaponSocket)
		{
			return SkeletalMeshComponent;
		}
	}

	return GetMesh();
}

USkeletalMeshComponent* ASXPlayerCharacter::FindWeaponAttachMesh(FName PreferredSocketName, FName LegacySocketName) const
{
	USkeletalMeshComponent* PreferredMesh = GetWeaponAttachMesh();
	if (IsValid(PreferredMesh)
		&& (PreferredMesh->DoesSocketExist(PreferredSocketName) || PreferredMesh->DoesSocketExist(LegacySocketName)))
	{
		return PreferredMesh;
	}

	if (IsValid(SkeletalMeshComponent.Get())
		&& SkeletalMeshComponent.Get() != PreferredMesh
		&& (SkeletalMeshComponent->DoesSocketExist(PreferredSocketName) || SkeletalMeshComponent->DoesSocketExist(LegacySocketName)))
	{
		return SkeletalMeshComponent.Get();
	}

	USkeletalMeshComponent* ThirdPersonMesh = GetMesh();
	if (IsValid(ThirdPersonMesh)
		&& ThirdPersonMesh != PreferredMesh
		&& (ThirdPersonMesh->DoesSocketExist(PreferredSocketName) || ThirdPersonMesh->DoesSocketExist(LegacySocketName)))
	{
		return ThirdPersonMesh;
	}

	return PreferredMesh;
}

void ASXPlayerCharacter::Interact()
{
	if (!IsAlive())
	{
		return;
	}

	if (IsValid(CurrentInteractionCandidate.Get()) == true && CurrentInteractionCandidate->GetClass()->ImplementsInterface(USXInteractableInterface::StaticClass()) == true)
	{
		if (ISXInteractableInterface::Execute_CanInteract(CurrentInteractionCandidate, this) == true)
		{
			ISXInteractableInterface::Execute_Interact(CurrentInteractionCandidate, this);
		}
	}
}

void ASXPlayerCharacter::InputMovementSkill()
{
	if (!IsAlive() || IsValid(SkillComponent.Get()) == false)
	{
		return;
	}

	SkillComponent->ActivateSkill(ESXSkillSlot::Movement);
}

void ASXPlayerCharacter::InputSkill1()
{
	if (!IsAlive() || IsValid(SkillComponent.Get()) == false)
	{
		return;
	}

	SkillComponent->ActivateSkill(ESXSkillSlot::Skill1);
}

void ASXPlayerCharacter::InputSkill2()
{
	if (!IsAlive() || IsValid(SkillComponent.Get()) == false)
	{
		return;
	}

	SkillComponent->ActivateSkill(ESXSkillSlot::Skill2);
}

void ASXPlayerCharacter::InputWeaponSlot1()
{
	EquipWeaponSlot(0);
}

void ASXPlayerCharacter::InputWeaponSlot2()
{
	EquipWeaponSlot(1);
}

void ASXPlayerCharacter::InputWeaponSlot3()
{
	EquipWeaponSlot(2);
}

void ASXPlayerCharacter::InputNextWeapon()
{
	EquipNextWeapon();
}

void ASXPlayerCharacter::InputPreviousWeapon()
{
	EquipPreviousWeapon();
}

void ASXPlayerCharacter::InputDropWeapon()
{
	DropCurrentWeapon();
}

void ASXPlayerCharacter::StartIronSightInput()
{
	if (GameplayOptions.bToggleAim)
	{
		IronSight();
		return;
	}

	SetIronSightZoomed(true);
}

void ASXPlayerCharacter::StopIronSightInput()
{
	if (GameplayOptions.bToggleAim)
	{
		return;
	}

	SetIronSightZoomed(false);
}

void ASXPlayerCharacter::TryFire()
{
	if (IsValid(CurrentWeapon.Get()) == false)
	{
		return;
	}

	CurrentWeapon->TryFire(this);
}

void ASXPlayerCharacter::ChangeView()
{
	if (!IsAlive())
	{
		return;
	}

	switch (CurrentViewMode)
	{
	case EViewMode::BackView:
		SetViewMode(EViewMode::FirstPersonView);
		break;
	case EViewMode::FirstPersonView:
		SetViewMode(EViewMode::ThirdPersonView);
		break;
	case EViewMode::ThirdPersonView:
		SetViewMode(EViewMode::BackView);
		break;
	case EViewMode::None:
	case EViewMode::End:
	default:
		SetViewMode(DefaultViewMode);
		break;
	}
}

void ASXPlayerCharacter::SetViewMode(EViewMode InViewMode)
{
	if (CurrentViewMode == InViewMode)
	{
		return;
	}

	CurrentViewMode = InViewMode;

	switch (CurrentViewMode)
	{
	case EViewMode::BackView:
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = true;
		bUseControllerRotationRoll = false;

		SkeletalMeshComponent->SetOwnerNoSee(true);
		SkeletalMeshComponent->SetHiddenInGame(true);
		GetMesh()->SetOwnerNoSee(false);
		GetMesh()->SetHiddenInGame(false);

		CameraComponent->AttachToComponent(SpringArmComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, USpringArmComponent::SocketName);
		CameraComponent->SetRelativeLocation(ThirdPersonCameraLocalLocation);
		CameraComponent->SetRelativeRotation(ThirdPersonCameraLocalRotation);
		CameraComponent->bUsePawnControlRotation = false;

		SpringArmComponent->SetActive(true);
		SpringArmComponent->SetRelativeLocation(ThirdPersonPivotLocation);
		SpringArmComponent->TargetArmLength = 400.0f;
		SpringArmComponent->SocketOffset = ThirdPersonCameraOffset;
		SpringArmComponent->SetRelativeRotation(FRotator::ZeroRotator);
		SpringArmComponent->bUsePawnControlRotation = true;
		SpringArmComponent->bInheritPitch = true;
		SpringArmComponent->bInheritYaw = true;
		SpringArmComponent->bInheritRoll = false;
		SpringArmComponent->bDoCollisionTest = true;

		GetCharacterMovement()->bOrientRotationToMovement = false;

		break;
	case EViewMode::FirstPersonView:
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = true;
		bUseControllerRotationRoll = false;

		SkeletalMeshComponent->SetOwnerNoSee(false);
		SkeletalMeshComponent->SetHiddenInGame(false);
		GetMesh()->SetOwnerNoSee(true);
		GetMesh()->SetHiddenInGame(true);

		CameraComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		CameraComponent->SetRelativeLocation(FirstPersonCameraLocation);
		CameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
		CameraComponent->bUsePawnControlRotation = true;

		SpringArmComponent->SetActive(false);

		GetCharacterMovement()->bOrientRotationToMovement = false;

		break;
	case EViewMode::ThirdPersonView:
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = true;
		bUseControllerRotationRoll = false;

		SkeletalMeshComponent->SetOwnerNoSee(true);
		SkeletalMeshComponent->SetHiddenInGame(true);
		GetMesh()->SetOwnerNoSee(false);
		GetMesh()->SetHiddenInGame(false);

		CameraComponent->AttachToComponent(SpringArmComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, USpringArmComponent::SocketName);
		CameraComponent->SetRelativeLocation(ThirdPersonCameraLocalLocation);
		CameraComponent->SetRelativeRotation(ThirdPersonCameraLocalRotation);
		CameraComponent->bUsePawnControlRotation = false;

		SpringArmComponent->SetActive(true);
		SpringArmComponent->SetRelativeLocation(ThirdPersonPivotLocation);
		SpringArmComponent->TargetArmLength = ThirdPersonArmLength;
		SpringArmComponent->SocketOffset = ThirdPersonCameraOffset;
		SpringArmComponent->SetRelativeRotation(FRotator::ZeroRotator);
		SpringArmComponent->bUsePawnControlRotation = true;
		SpringArmComponent->bInheritPitch = true;
		SpringArmComponent->bInheritYaw = true;
		SpringArmComponent->bInheritRoll = false;
		SpringArmComponent->bDoCollisionTest = true;

		GetCharacterMovement()->bOrientRotationToMovement = false;

		break;
	case EViewMode::None:
	case EViewMode::End:
	default:
		break;
	}

	ReattachCurrentWeaponToViewMesh();
}

void ASXPlayerCharacter::ReattachCurrentWeaponToViewMesh()
{
	ASXWeapon* Weapon = CurrentWeapon.Get();
	if (IsValid(Weapon) == false)
	{
		return;
	}

	USkeletalMeshComponent* TargetMesh = FindWeaponAttachMesh(Weapon->GetCharacterAttachSocketName(), Weapon->GetLegacyCharacterAttachSocketName());
	if (IsValid(TargetMesh) == false)
	{
		return;
	}

	FName SocketNameToUse = Weapon->GetCharacterAttachSocketName();
	if (TargetMesh->DoesSocketExist(SocketNameToUse) == false && TargetMesh->DoesSocketExist(Weapon->GetLegacyCharacterAttachSocketName()))
	{
		SocketNameToUse = Weapon->GetLegacyCharacterAttachSocketName();
	}

	const bool bSocketExists = TargetMesh->DoesSocketExist(SocketNameToUse);
	if (bSocketExists == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot attach weapon %s to socket. Mesh=%s Socket=%s LegacySocket=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Weapon),
			*GetNameSafe(TargetMesh),
			*Weapon->GetCharacterAttachSocketName().ToString(),
			*Weapon->GetLegacyCharacterAttachSocketName().ToString());
	}

	Weapon->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, bSocketExists ? SocketNameToUse : NAME_None);

	// BeginPlay equips the default weapon before some HUD widgets finish
	// constructing. Re-broadcast on the scheduled reattach tick so the initial
	// revolver ammo display binds to the same data as subsequently equipped guns.
	Weapon->BroadcastAmmoChanged();
}

void ASXPlayerCharacter::EquipDefaultWeapon()
{
	if (bEquipDefaultWeaponOnBeginPlay == false || DefaultWeaponClass == nullptr || IsValid(CurrentWeapon.Get()))
	{
		return;
	}

	if (IsValid(InventoryComponent.Get()))
	{
		if (DefaultWeaponSlotIndex >= 0 && DefaultWeaponSlotIndex < InventoryComponent->GetMaxWeaponSlots())
		{
			InventoryComponent->SetWeaponSlot(DefaultWeaponSlotIndex, DefaultWeaponClass);
		}
		else
		{
			int32 AddedSlotIndex = INDEX_NONE;
			InventoryComponent->AddWeaponClass(DefaultWeaponClass, AddedSlotIndex);
			DefaultWeaponSlotIndex = AddedSlotIndex;
		}
	}

	EquipWeaponClassInternal(DefaultWeaponClass, DefaultWeaponSlotIndex, true);
}

bool ASXPlayerCharacter::EquipWeaponSlot(int32 SlotIndex)
{
	if (!IsAlive() || IsValid(InventoryComponent.Get()) == false)
	{
		return false;
	}

	TSubclassOf<ASXWeapon> WeaponClass = InventoryComponent->GetWeaponInSlot(SlotIndex);
	if (WeaponClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot equip weapon slot %d: slot is empty."),
			*GetNameSafe(this),
			SlotIndex);
		return false;
	}

	return EquipWeaponClassInternal(WeaponClass, SlotIndex, false);
}

bool ASXPlayerCharacter::EquipWeaponClass(TSubclassOf<ASXWeapon> WeaponClass, bool bGrantInitialAmmo)
{
	if (!IsAlive())
	{
		return false;
	}

	int32 SlotIndex = INDEX_NONE;
	if (IsValid(InventoryComponent.Get()))
	{
		SlotIndex = InventoryComponent->GetWeaponSlots().Find(WeaponClass);
	}

	return EquipWeaponClassInternal(WeaponClass, SlotIndex, bGrantInitialAmmo);
}

bool ASXPlayerCharacter::AddPickedWeaponClassToInventory(TSubclassOf<ASXWeapon> WeaponClass, int32& OutSlotIndex)
{
	TSubclassOf<ASXWeapon> ReplacedWeaponClass = nullptr;
	return AddPickedWeaponClassToInventory(WeaponClass, OutSlotIndex, ReplacedWeaponClass);
}

bool ASXPlayerCharacter::AddPickedWeaponClassToInventory(TSubclassOf<ASXWeapon> WeaponClass, int32& OutSlotIndex, TSubclassOf<ASXWeapon>& OutReplacedWeaponClass)
{
	OutSlotIndex = INDEX_NONE;
	OutReplacedWeaponClass = nullptr;
	if (WeaponClass == nullptr || IsValid(InventoryComponent.Get()) == false)
	{
		return false;
	}

	const int32 ExistingSlotIndex = InventoryComponent->GetWeaponSlots().Find(WeaponClass);
	if (ExistingSlotIndex != INDEX_NONE)
	{
		OutSlotIndex = ExistingSlotIndex;
		return true;
	}

	const int32 SlotIndex = ChoosePickedWeaponSlot(WeaponClass);
	if (SlotIndex == INDEX_NONE)
	{
		return false;
	}

	OutReplacedWeaponClass = InventoryComponent->GetWeaponInSlot(SlotIndex);
	if (OutReplacedWeaponClass == WeaponClass)
	{
		OutReplacedWeaponClass = nullptr;
	}

	const bool bSlotSet = InventoryComponent->SetWeaponSlot(SlotIndex, WeaponClass);
	if (bSlotSet)
	{
		OutSlotIndex = SlotIndex;
	}

	return bSlotSet;
}

bool ASXPlayerCharacter::EquipNextWeapon()
{
	return EquipWeaponByOffset(1);
}

bool ASXPlayerCharacter::EquipPreviousWeapon()
{
	return EquipWeaponByOffset(-1);
}

bool ASXPlayerCharacter::DropCurrentWeapon()
{
	if (!IsAlive() || IsValid(CurrentWeapon.Get()) == false || IsValid(InventoryComponent.Get()) == false)
	{
		return false;
	}

	const int32 SlotIndexToDrop = CurrentWeaponSlotIndex;
	if (SlotIndexToDrop == INDEX_NONE || InventoryComponent->GetWeaponInSlot(SlotIndexToDrop) == nullptr)
	{
		return false;
	}

	if (bCanDropDefaultWeaponSlot == false && SlotIndexToDrop == DefaultWeaponSlotIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot drop default weapon slot %d."),
			*GetNameSafe(this),
			SlotIndexToDrop);
		return false;
	}

	TSubclassOf<ASXWeapon> WeaponClassToDrop = InventoryComponent->GetWeaponInSlot(SlotIndexToDrop);
	if (WeaponClassToDrop == nullptr)
	{
		return false;
	}

	const FVector DropSourceLocation = CurrentWeapon->GetActorLocation();

	StopFire();
	CurrentWeapon->CancelReload();
	CurrentWeapon->Destroy();
	CurrentWeapon = nullptr;

	InventoryComponent->RemoveWeaponAt(SlotIndexToDrop);
	CurrentWeaponSlotIndex = INDEX_NONE;

	DropWeaponClass(WeaponClassToDrop, DropSourceLocation);

	if (bCanDropDefaultWeaponSlot == false
		&& DefaultWeaponSlotIndex != SlotIndexToDrop
		&& InventoryComponent->GetWeaponInSlot(DefaultWeaponSlotIndex) != nullptr)
	{
		return EquipWeaponSlot(DefaultWeaponSlotIndex);
	}

	return EquipNextWeapon();
}

bool ASXPlayerCharacter::EquipWeaponByOffset(int32 SlotOffset)
{
	if (!IsAlive() || IsValid(InventoryComponent.Get()) == false || InventoryComponent->GetMaxWeaponSlots() <= 0)
	{
		return false;
	}

	const int32 MaxSlots = InventoryComponent->GetMaxWeaponSlots();
	int32 StartSlotIndex = CurrentWeaponSlotIndex;
	if (StartSlotIndex == INDEX_NONE)
	{
		StartSlotIndex = SlotOffset >= 0 ? -1 : 0;
	}

	const int32 Direction = SlotOffset >= 0 ? 1 : -1;
	for (int32 Step = 1; Step <= MaxSlots; ++Step)
	{
		const int32 CandidateSlotIndex = (StartSlotIndex + Direction * Step + MaxSlots) % MaxSlots;
		if (InventoryComponent->GetWeaponInSlot(CandidateSlotIndex) != nullptr)
		{
			return EquipWeaponSlot(CandidateSlotIndex);
		}
	}

	return false;
}

int32 ASXPlayerCharacter::ChoosePickedWeaponSlot(TSubclassOf<ASXWeapon> WeaponClass) const
{
	if (WeaponClass == nullptr || IsValid(InventoryComponent.Get()) == false)
	{
		return INDEX_NONE;
	}

	constexpr int32 PrimarySlot0 = 0;
	constexpr int32 PrimarySlot1 = 1;

	if (InventoryComponent->GetMaxWeaponSlots() <= PrimarySlot0)
	{
		return INDEX_NONE;
	}

	if (InventoryComponent->GetWeaponInSlot(PrimarySlot0) == nullptr)
	{
		return PrimarySlot0;
	}

	if (InventoryComponent->GetMaxWeaponSlots() > PrimarySlot1 && InventoryComponent->GetWeaponInSlot(PrimarySlot1) == nullptr)
	{
		return PrimarySlot1;
	}

	if (CurrentWeaponSlotIndex == PrimarySlot0 || CurrentWeaponSlotIndex == PrimarySlot1)
	{
		return CurrentWeaponSlotIndex;
	}

	return PrimarySlot0;
}

bool ASXPlayerCharacter::EquipWeaponClassInternal(TSubclassOf<ASXWeapon> WeaponClass, int32 SlotIndex, bool bGrantInitialAmmo)
{
	if (WeaponClass == nullptr)
	{
		return false;
	}

	if (IsValid(CurrentWeapon.Get()) && CurrentWeapon->GetClass() == WeaponClass)
	{
		CurrentWeaponSlotIndex = SlotIndex;
		CurrentWeapon->BroadcastAmmoChanged();
		ReattachCurrentWeaponToViewMesh();
		return true;
	}

	StopFire();

	if (IsValid(CurrentWeapon.Get()))
	{
		CurrentWeapon->CancelReload();
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASXWeapon* NewWeapon = World->SpawnActor<ASXWeapon>(
		WeaponClass,
		GetActorTransform(),
		SpawnParams
	);

	if (IsValid(NewWeapon) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to spawn weapon %s."),
			*GetNameSafe(this),
			*GetNameSafe(WeaponClass.Get()));
		return false;
	}

	if (NewWeapon->EquipToCharacter(this, bGrantInitialAmmo) == false)
	{
		NewWeapon->Destroy();
		return false;
	}

	CurrentWeaponSlotIndex = SlotIndex;
	bIsFullAutoFire = NewWeapon->IsFullAutoEnabled();
	ReattachCurrentWeaponToViewMesh();
	return true;
}

void ASXPlayerCharacter::DropWeaponClass(TSubclassOf<ASXWeapon> WeaponClass, const FVector& DropSourceLocation)
{
	if (WeaponClass == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	const FVector ForwardDirection = GetActorForwardVector().GetSafeNormal();
	const FVector DropLocation = DropSourceLocation + ForwardDirection * 120.0f + FVector::UpVector * 60.0f;
	const FRotator DropRotation = FRotator(0.0f, GetActorRotation().Yaw, 0.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = nullptr;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASXWeapon* DroppedWeapon = World->SpawnActor<ASXWeapon>(
		WeaponClass,
		DropLocation,
		DropRotation,
		SpawnParams
	);

	if (IsValid(DroppedWeapon) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to drop replaced weapon %s."),
			*GetNameSafe(this),
			*GetNameSafe(WeaponClass.Get()));
		return;
	}

	DroppedWeapon->SetGrantInitialReserveAmmoOnPickup(false);
}

void ASXPlayerCharacter::AttackMelee()
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Attack()")));
	if (GetCharacterMovement()->IsFalling() == true)
	{
		return;
	}

	USXAnimInstance* AnimInstance = Cast<USXAnimInstance>(GetMesh()->GetAnimInstance());
	if (IsValid(AnimInstance) == true && IsValid(AttackMeleeMontage) == true && AnimInstance->Montage_IsPlaying(AttackMeleeMontage) == false)
	{
		AnimInstance->Montage_Play(AttackMeleeMontage);
	}
}

void ASXPlayerCharacter::InputToggleSelector(const FInputActionValue& InValue)
{
	if (IsValid(CurrentWeapon.Get()) == false)
	{
		return;
	}

	const bool bToggled = CurrentWeapon->ToggleFullAuto();
	bIsFullAutoFire = CurrentWeapon->IsFullAutoEnabled();

	if (bToggled == false)
	{
		UE_LOG(LogTemp, Log, TEXT("Weapon %s does not support full-auto fire."), *GetNameSafe(CurrentWeapon.Get()));
	}
}

void ASXPlayerCharacter::IronSight()
{
	if (!IsAlive())
	{
		return;
	}

	if (IsValid(CurrentWeapon.Get()) == false)
	{
		return;
	}

	SetIronSightZoomed(!Zoomed);
}

void ASXPlayerCharacter::SetIronSightZoomed(bool bNewZoomed)
{
	if (!IsAlive() || IsValid(CurrentWeapon.Get()) == false)
	{
		return;
	}

	Zoomed = bNewZoomed;

	if (Zoomed)
	{
		TargetFOV = 45.f;
	}
	else
	{
		TargetFOV = 70.f;
	}
}
