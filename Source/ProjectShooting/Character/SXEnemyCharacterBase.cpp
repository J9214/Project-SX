// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SXEnemyCharacterBase.h"

#include "Controller/SXEnemyAIController.h"
#include "Engine/DamageEvents.h"

ASXEnemyCharacterBase::ASXEnemyCharacterBase()
{
	AIControllerClass = ASXEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

bool ASXEnemyCharacterBase::TryAttack(AActor* TargetActor)
{
	if (IsAlive() == false || IsValid(TargetActor) == false)
	{
		return false;
	}

	ASXCharacterBase* TargetCharacter = Cast<ASXCharacterBase>(TargetActor);
	if (IsValid(TargetCharacter) == false || TargetCharacter->IsAlive() == false)
	{
		return false;
	}

	const float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
	if (DistanceToTarget > AttackRange)
	{
		return false;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackInterval)
	{
		return false;
	}

	LastAttackTime = CurrentTime;

	FDamageEvent DamageEvent;
	TargetCharacter->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
	return true;
}

void ASXEnemyCharacterBase::Die(AActor* DamageCauser)
{
	const bool bWasAlive = bIsDead == false;

	Super::Die(DamageCauser);

	if (bWasAlive)
	{
		OnEnemyDeath.Broadcast(this);
	}
}
