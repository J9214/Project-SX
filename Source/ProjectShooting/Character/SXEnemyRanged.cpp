// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/SXEnemyRanged.h"

#include "Components/SXStatusComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Controller/SXEnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Item/SXEnemyProjectile.h"
#include "TimerManager.h"

ASXEnemyRanged::ASXEnemyRanged()
{
	if (IsValid(StatusComponent))
	{
		StatusComponent->SetMaxHealth(40.0f);
	}

	AttackDamage = 6.0f;
	AttackRange = 160.0f;
	AttackInterval = 1.5f;
	ProjectileClass = ASXEnemyProjectile::StaticClass();

	GetCharacterMovement()->MaxWalkSpeed = 360.0f;
}

void ASXEnemyRanged::UpdateAIBehavior(ASXEnemyAIController* AIController, APawn* TargetPawn)
{
	if (IsValid(AIController) == false || IsAlive() == false || IsValid(TargetPawn) == false)
	{
		return;
	}

	const FVector ToTarget = TargetPawn->GetActorLocation() - GetActorLocation();
	const float DistanceToTarget = ToTarget.Size();

	AIController->SetFocus(TargetPawn);

	if (DistanceToTarget > RangedAttackRange)
	{
		AIController->MoveToActor(TargetPawn, PreferredRange);
		return;
	}

	if (DistanceToTarget < RetreatRange)
	{
		AIController->StopMovement();
		const FVector RetreatDirection = FVector(-ToTarget.X, -ToTarget.Y, 0.0f).GetSafeNormal();
		AddMovementInput(RetreatDirection, 1.0f);
		TryRangedAttack(TargetPawn);
		return;
	}

	AIController->StopMovement();
	TryRangedAttack(TargetPawn);
}

bool ASXEnemyRanged::TryRangedAttack(APawn* TargetPawn)
{
	if (IsValid(TargetPawn) == false || IsAlive() == false || bIsAttacking || IsValid(ProjectileClass) == false)
	{
		return false;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackInterval)
	{
		return false;
	}

	LastAttackTime = CurrentTime;
	bIsAttacking = true;
	bHasFiredProjectileThisAttack = false;
	CurrentRangedTarget = TargetPawn;
	FaceRangedTarget();

	const float AttackDuration = PlayAnimMontage(AttackMeleeMontage);
	if (AttackDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			AttackFinishTimerHandle,
			this,
			&ThisClass::FinishAttack,
			AttackDuration,
			false
		);
	}
	else
	{
		// Keep the ranged enemy functional until an attack montage is assigned.
		HandleProjectileFireNotify();
		FinishAttack();
	}

	return true;
}

void ASXEnemyRanged::HandleProjectileFireNotify()
{
	if (IsAlive() == false || bIsAttacking == false || bHasFiredProjectileThisAttack || IsValid(CurrentRangedTarget) == false)
	{
		return;
	}

	// The target may have moved during the attack wind-up. Face it again at
	// the exact notify frame so the projectile never appears to fire backward.
	FaceRangedTarget();
	bHasFiredProjectileThisAttack = true;
	SpawnProjectile();
}

void ASXEnemyRanged::FaceRangedTarget()
{
	if (IsValid(CurrentRangedTarget) == false)
	{
		return;
	}

	FVector ToTarget = CurrentRangedTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	SetActorRotation(FRotator(0.0f, ToTarget.Rotation().Yaw, 0.0f));
}

bool ASXEnemyRanged::SpawnProjectile()
{
	if (IsValid(ProjectileClass) == false || IsValid(CurrentRangedTarget) == false || IsValid(GetMesh()) == false)
	{
		return false;
	}

	const FVector SpawnLocation = GetMesh()->DoesSocketExist(ProjectileSocketName)
		? GetMesh()->GetSocketLocation(ProjectileSocketName)
		: GetActorLocation() + GetActorRotation().RotateVector(ProjectileSpawnOffset);
	const FVector AimLocation = CurrentRangedTarget->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	const FRotator SpawnRotation = (AimLocation - SpawnLocation).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASXEnemyProjectile* Projectile = GetWorld()->SpawnActor<ASXEnemyProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (IsValid(Projectile))
	{
		Projectile->InitializeProjectile(ProjectileDamage, this);
		return true;
	}

	return false;
}

void ASXEnemyRanged::FinishAttack()
{
	Super::FinishAttack();

	CurrentRangedTarget = nullptr;
	bHasFiredProjectileThisAttack = false;
}
