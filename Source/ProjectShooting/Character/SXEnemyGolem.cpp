// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/SXEnemyGolem.h"

#include "Camera/CameraShakeBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SXStatusComponent.h"
#include "Controller/SXEnemyAIController.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

ASXEnemyGolem::ASXEnemyGolem()
{
	if (IsValid(StatusComponent))
	{
		StatusComponent->SetMaxHealth(200.0f);
	}

	AttackDamage = 20.0f;
	AttackRange = 200.0f;
	AttackInterval = 1.8f;

	GetCharacterMovement()->MaxWalkSpeed = 280.0f;
}

void ASXEnemyGolem::UpdateAIBehavior(ASXEnemyAIController* AIController, APawn* TargetPawn)
{
	if (IsValid(AIController) == false || IsAlive() == false || IsValid(TargetPawn) == false)
	{
		return;
	}

	if (LeapState != ESXGolemLeapState::None)
	{
		AIController->StopMovement();
		AIController->SetFocus(TargetPawn);
		return;
	}

	if (CanStartLeapAttack(TargetPawn))
	{
		AIController->StopMovement();
		AIController->SetFocus(TargetPawn);
		StartLeapAttack(TargetPawn);
		return;
	}

	Super::UpdateAIBehavior(AIController, TargetPawn);
}

bool ASXEnemyGolem::StartLeapAttack(APawn* TargetPawn)
{
	if (CanStartLeapAttack(TargetPawn) == false)
	{
		return false;
	}

	ASXCharacterBase* TargetCharacter = Cast<ASXCharacterBase>(TargetPawn);
	if (IsValid(TargetCharacter) == false)
	{
		return false;
	}

	LeapTarget = TargetCharacter;
	LeapTargetLocation = ResolveLeapTargetLocation(TargetPawn);
	LeapState = ESXGolemLeapState::Windup;
	bIsAttacking = true;

	const FVector HorizontalDirection = FVector(
		LeapTargetLocation.X - GetActorLocation().X,
		LeapTargetLocation.Y - GetActorLocation().Y,
		0.0f
	).GetSafeNormal();

	if (HorizontalDirection.IsNearlyZero() == false)
	{
		SetActorRotation(HorizontalDirection.Rotation());
	}

	GetCharacterMovement()->StopMovementImmediately();
	SpawnLandingWarning();

	const float MontageDuration = PlayAnimMontage(LeapWindupMontage);
	if (MontageDuration > 0.0f && LeapLaunchFallbackDelay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			LeapLaunchFallbackTimerHandle,
			this,
			&ThisClass::HandleLeapLaunchNotify,
			FMath::Min(LeapLaunchFallbackDelay, MontageDuration),
			false
		);
	}
	else
	{
		HandleLeapLaunchNotify();
	}

	return true;
}

void ASXEnemyGolem::HandleLeapLaunchNotify()
{
	if (IsAlive() == false || LeapState != ESXGolemLeapState::Windup)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(LeapLaunchFallbackTimerHandle);

	const float FlightTime = FMath::Max(0.1f, LeapFlightTime);
	const FVector StartLocation = GetActorLocation();
	const FVector Delta = LeapTargetLocation - StartLocation;
	const float GravityZ = GetCharacterMovement()->GetGravityZ();

	FVector LaunchVelocity(Delta.X / FlightTime, Delta.Y / FlightTime, 0.0f);
	LaunchVelocity.Z = (Delta.Z - 0.5f * GravityZ * FlightTime * FlightTime) / FlightTime;

	LeapState = ESXGolemLeapState::Airborne;
	LaunchCharacter(LaunchVelocity, true, true);
}

void ASXEnemyGolem::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (LeapState != ESXGolemLeapState::Airborne)
	{
		return;
	}

	LeapState = ESXGolemLeapState::Recovery;
	ClearLandingWarning();
	ApplySlamImpact();
	PlayAnimMontage(LeapLandMontage);

	GetWorldTimerManager().SetTimer(
		LeapRecoveryTimerHandle,
		this,
		&ThisClass::FinishLeapAttack,
		FMath::Max(0.01f, RecoveryDuration),
		false
	);
}

void ASXEnemyGolem::FinishLeapAttack()
{
	GetWorldTimerManager().ClearTimer(LeapLaunchFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(LeapRecoveryTimerHandle);
	ClearLandingWarning();

	LeapState = ESXGolemLeapState::None;
	LeapTarget = nullptr;
	bIsAttacking = false;
	LastLeapEndTime = GetWorld()->GetTimeSeconds();
}

void ASXEnemyGolem::Die(AActor* DamageCauser)
{
	GetWorldTimerManager().ClearTimer(LeapLaunchFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(LeapRecoveryTimerHandle);
	ClearLandingWarning();

	LeapState = ESXGolemLeapState::None;
	LeapTarget = nullptr;

	Super::Die(DamageCauser);
}

bool ASXEnemyGolem::CanStartLeapAttack(const APawn* TargetPawn) const
{
	if (IsAlive() == false || bIsAttacking || LeapState != ESXGolemLeapState::None || IsValid(TargetPawn) == false)
	{
		return false;
	}

	const ASXCharacterBase* TargetCharacter = Cast<ASXCharacterBase>(TargetPawn);
	if (IsValid(TargetCharacter) == false || TargetCharacter->IsAlive() == false)
	{
		return false;
	}

	if (GetCharacterMovement()->IsMovingOnGround() == false)
	{
		return false;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastLeapEndTime < LeapCooldown)
	{
		return false;
	}

	const float DistanceToTarget = FVector::Dist2D(GetActorLocation(), TargetPawn->GetActorLocation());
	return DistanceToTarget >= LeapMinRange && DistanceToTarget <= LeapMaxRange;
}

FVector ASXEnemyGolem::ResolveLeapTargetLocation(const APawn* TargetPawn)
{
	const FVector ToTarget = TargetPawn->GetActorLocation() - GetActorLocation();
	const FVector ApproachDirection = FVector(ToTarget.X, ToTarget.Y, 0.0f).GetSafeNormal();
	FVector PredictedLocation = TargetPawn->GetActorLocation() + TargetPawn->GetVelocity() * TargetPredictionTime;
	PredictedLocation -= ApproachDirection * LandingOffset;

	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation ProjectedNavLocation;
	if (IsValid(NavigationSystem) && NavigationSystem->ProjectPointToNavigation(
		PredictedLocation,
		ProjectedNavLocation,
		FVector(250.0f, 250.0f, 500.0f)
	))
	{
		LandingWarningLocation = ProjectedNavLocation.Location + FVector(0.0f, 0.0f, 2.0f);
		PredictedLocation = ProjectedNavLocation.Location + FVector(
			0.0f,
			0.0f,
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		);
		return PredictedLocation;
	}

	const FVector TraceStart = PredictedLocation + FVector(0.0f, 0.0f, 1000.0f);
	const FVector TraceEnd = PredictedLocation - FVector(0.0f, 0.0f, 2000.0f);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SXGolemLeapGround), false, this);
	QueryParams.AddIgnoredActor(TargetPawn);

	FHitResult GroundHit;
	if (GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		LandingWarningLocation = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 2.0f);
		PredictedLocation.Z = GroundHit.ImpactPoint.Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}
	else
	{
		LandingWarningLocation = PredictedLocation - FVector(
			0.0f,
			0.0f,
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		);
	}

	return PredictedLocation;
}

void ASXEnemyGolem::ApplySlamImpact()
{
	ASXCharacterBase* TargetCharacter = LeapTarget.Get();
	if (IsValid(TargetCharacter) && TargetCharacter->IsAlive())
	{
		const FVector ToTarget = TargetCharacter->GetActorLocation() - GetActorLocation();
		const bool bWithinRadius = FVector::Dist2D(GetActorLocation(), TargetCharacter->GetActorLocation()) <= SlamRadius;
		const bool bWithinHeight = FMath::Abs(ToTarget.Z) <= SlamMaxHeightDifference;

		if (bWithinRadius && bWithinHeight)
		{
			FDamageEvent DamageEvent;
			TargetCharacter->TakeDamage(SlamDamage, DamageEvent, GetController(), this);

			const FVector KnockbackDirection = FVector(ToTarget.X, ToTarget.Y, 0.0f).GetSafeNormal();
			TargetCharacter->LaunchCharacter(
				KnockbackDirection * KnockbackHorizontalSpeed + FVector(0.0f, 0.0f, KnockbackVerticalSpeed),
				true,
				true
			);
		}

		APlayerController* PlayerController = TargetCharacter->GetController<APlayerController>();
		const bool bWithinShakeRange = FVector::Dist2D(GetActorLocation(), TargetCharacter->GetActorLocation()) <= SlamRadius * 2.0f;
		if (IsValid(PlayerController) && SlamCameraShake != nullptr && bWithinShakeRange)
		{
			PlayerController->ClientStartCameraShake(SlamCameraShake);
		}
	}

	const FVector ImpactLocation = GetActorLocation() - FVector(
		0.0f,
		0.0f,
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
	);

	if (IsValid(SlamEffect))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SlamEffect, ImpactLocation);
	}

	if (IsValid(SlamSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, SlamSound, ImpactLocation);
	}
}

void ASXEnemyGolem::SpawnLandingWarning()
{
	ClearLandingWarning();

	if (LandingWarningActorClass == nullptr)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	LandingWarningActor = GetWorld()->SpawnActor<AActor>(
		LandingWarningActorClass,
		LandingWarningLocation,
		FRotator::ZeroRotator,
		SpawnParameters
	);
}

void ASXEnemyGolem::ClearLandingWarning()
{
	if (IsValid(LandingWarningActor))
	{
		LandingWarningActor->Destroy();
	}

	LandingWarningActor = nullptr;
}
