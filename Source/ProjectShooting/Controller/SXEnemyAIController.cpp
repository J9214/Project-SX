// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/SXEnemyAIController.h"

#include "Character/SXEnemyCharacterBase.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ASXEnemyAIController::ASXEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASXEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GetWorldTimerManager().SetTimer(
		AIUpdateTimerHandle,
		this,
		&ThisClass::UpdateAI,
		AIUpdateInterval,
		true,
		0.0f
	);
}

void ASXEnemyAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(AIUpdateTimerHandle);

	Super::OnUnPossess();
}

void ASXEnemyAIController::UpdateAI()
{
	ASXEnemyCharacterBase* Enemy = Cast<ASXEnemyCharacterBase>(GetPawn());
	APawn* TargetPawn = GetTargetPlayerPawn();

	if (IsValid(Enemy) == false || Enemy->IsAlive() == false || IsValid(TargetPawn) == false)
	{
		StopMovement();
		return;
	}

	const float AttackRange = Enemy->GetAttackRange();
	const float DistanceToTarget = FVector::Dist(Enemy->GetActorLocation(), TargetPawn->GetActorLocation());

	if (DistanceToTarget > AttackRange)
	{
		MoveToActor(TargetPawn, FMath::Max(0.0f, AttackRange - AcceptanceRadiusPadding));
		return;
	}

	StopMovement();
	SetFocus(TargetPawn);
	Enemy->TryAttack(TargetPawn);
}

APawn* ASXEnemyAIController::GetTargetPlayerPawn() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}
