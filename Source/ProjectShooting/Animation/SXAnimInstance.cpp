// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/SXAnimInstance.h"
#include "Character/SXCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/SXPlayerCharacter.h"
#include "Kismet/KismetMathLibrary.h"
void USXAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CacheOwnerReferences();
}

void USXAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(Character) || !IsValid(MovementComponent))
	{
		CacheOwnerReferences();
	}

	if (!IsValid(Character) || !IsValid(MovementComponent))
	{
		Velocity = FVector::ZeroVector;
		GroundSpeed = 0.0f;
		Direction = 0.0f;
		ShouldMove = false;
		IsFalling = false;
		IsUnarmed = true;
		return;
	}

	Velocity = MovementComponent->Velocity;
	GroundSpeed = Velocity.Size2D();
	IsFalling = MovementComponent->IsFalling();
	IsUnarmed = Character->CurrentWeapon == nullptr;

	if (GroundSpeed > KINDA_SMALL_NUMBER)
	{
		const FVector LocalVelocity = Character->GetActorTransform().InverseTransformVectorNoScale(Velocity);
		Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
	}
	else
	{
		Direction = 0.0f;
	}

	const float GroundAcceleration = MovementComponent->GetCurrentAcceleration().Size2D();
	ShouldMove = GroundSpeed > KINDA_SMALL_NUMBER && GroundAcceleration > KINDA_SMALL_NUMBER;

	if (APlayerController* OwnerPlayerController = Cast<APlayerController>(Character->GetController()))
	{
		NormalizedCurrentPitch = UKismetMathLibrary::NormalizeAxis(OwnerPlayerController->GetControlRotation().Pitch);
	}
}

void USXAnimInstance::CacheOwnerReferences()
{
	APawn* OwnerPawn = TryGetPawnOwner();
	Character = Cast<ASXCharacterBase>(OwnerPawn);
	MovementComponent = IsValid(Character) ? Character->GetCharacterMovement() : nullptr;
}
