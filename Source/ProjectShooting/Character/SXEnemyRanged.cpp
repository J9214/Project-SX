// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/SXEnemyRanged.h"

#include "Components/SXStatusComponent.h"
#include "Controller/SXEnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Item/SXEnemyProjectile.h"

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
	if (IsValid(TargetPawn) == false || IsAlive() == false || IsValid(ProjectileClass) == false)
	{
		return false;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackInterval)
	{
		return false;
	}

	LastAttackTime = CurrentTime;

	const FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(ProjectileSpawnOffset);
	const FVector AimLocation = TargetPawn->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
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
