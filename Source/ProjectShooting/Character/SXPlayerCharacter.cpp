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
#include "Input/SXInputConfig.h"

ASXPlayerCharacter::ASXPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 96.0f);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(GetMesh());
	SkeletalMeshComponent->SetOnlyOwnerSee(true);
	SkeletalMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FirstPersonCameraLocation);
	CameraComponent->bUsePawnControlRotation = true;

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

	TimeBetweenFire = GetStatusComponent()->GetTimeBetweenFire();
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

	SetViewMode(DefaultViewMode);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IsValid(EnhancedInputComponent))
	{
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
	if (PlayerCharacterInputConfig->ReloadAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->ReloadAction, ETriggerEvent::Started, this, &ThisClass::Reload);
	if (PlayerCharacterInputConfig->InteractAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->InteractAction, ETriggerEvent::Started, this, &ThisClass::Interact);
	if (PlayerCharacterInputConfig->ChangeViewAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->ChangeViewAction, ETriggerEvent::Started, this, &ThisClass::ChangeView);
	if (PlayerCharacterInputConfig->ToggleSelectorAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->ToggleSelectorAction, ETriggerEvent::Started, this, &ThisClass::InputToggleSelector);
	if (PlayerCharacterInputConfig->IronSightAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->IronSightAction, ETriggerEvent::Started, this, &ThisClass::IronSight);
	if (PlayerCharacterInputConfig->IronSightAction) EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->IronSightAction, ETriggerEvent::Completed, this, &ThisClass::IronSight);

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
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
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

	if (bIsFullAutoFire)
	{
		TryFire(); // 첫 발 즉시 발사

		TimeBetweenFire = GetStatusComponent()->GetTimeBetweenFire();

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
	UE_LOG(LogTemp, Warning, TEXT("End"));
	GetWorldTimerManager().ClearTimer(FullAutoTimerHandle);
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
	CurrentWeapon->Reload();
}

void ASXPlayerCharacter::Interact()
{
	if (!IsAlive())
	{
		return;
	}

	// Interaction component will be connected here when stage objects are added.
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
		GetMesh()->SetOwnerNoSee(false);

		CameraComponent->AttachToComponent(SpringArmComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, USpringArmComponent::SocketName);
		CameraComponent->SetRelativeLocation(FVector::ZeroVector);
		CameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
		CameraComponent->bUsePawnControlRotation = false;

		SpringArmComponent->SetActive(true);
		SpringArmComponent->SetRelativeLocation(ThirdPersonPivotLocation);
		SpringArmComponent->TargetArmLength = 400.0f;
		SpringArmComponent->SocketOffset = FVector::ZeroVector;
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
		GetMesh()->SetOwnerNoSee(true);

		CameraComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		CameraComponent->SetRelativeLocation(FirstPersonCameraLocation);
		CameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
		CameraComponent->bUsePawnControlRotation = true;

		SpringArmComponent->SetActive(false);

		GetCharacterMovement()->bOrientRotationToMovement = false;

		break;
	case EViewMode::ThirdPersonView:
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;

		SkeletalMeshComponent->SetOwnerNoSee(true);
		GetMesh()->SetOwnerNoSee(false);

		CameraComponent->AttachToComponent(SpringArmComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, USpringArmComponent::SocketName);
		CameraComponent->SetRelativeLocation(FVector::ZeroVector);
		CameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
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

		GetCharacterMovement()->bOrientRotationToMovement = true;

		break;
	case EViewMode::None:
	case EViewMode::End:
	default:
		break;
	}
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
	bIsFullAutoFire = !bIsFullAutoFire;
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
	
	Zoomed = !Zoomed;

	if (Zoomed) {
		TargetFOV = 45.f;
	}
	else {
		TargetFOV = 70.f;
	}
}
