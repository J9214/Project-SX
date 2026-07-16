// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/SXEnemyCharger.h"

#include "Components/SXStatusComponent.h"
#include "Controller/SXEnemyAIController.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

ASXEnemyCharger::ASXEnemyCharger()
{
	PrimaryActorTick.bCanEverTick = true;

	if (IsValid(StatusComponent))
	{
		StatusComponent->SetMaxHealth(80.0f);
	}

	AttackDamage = 8.0f;
	AttackRange = 150.0f;
	AttackInterval = 1.2f;

	DefaultWalkSpeed = 520.0f;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
}

void ASXEnemyCharger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCharging == false)
	{
		return;
	}

	if (IsAlive() == false)
	{
		StopCharge();
		return;
	}

	AddMovementInput(ChargeDirection, 1.0f);

	APawn* TargetPawn = ChargeTarget.Get();
	if (IsValid(TargetPawn))
	{
		TryApplyChargeDamage(TargetPawn);
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - ChargeStartTime >= ChargeDuration)
	{
		StopCharge();
	}
}

void ASXEnemyCharger::UpdateAIBehavior(ASXEnemyAIController* AIController, APawn* TargetPawn)
{
	if (IsValid(AIController) == false || IsAlive() == false || IsValid(TargetPawn) == false)
	{
		return;
	}

	if (bIsCharging)
	{
		AIController->SetFocus(TargetPawn);
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
	const bool bCanCharge = DistanceToTarget <= ChargeTriggerRange && DistanceToTarget > AttackRange && CurrentTime - LastChargeEndTime >= ChargeCooldown;

	if (bCanCharge)
	{
		AIController->StopMovement();
		AIController->SetFocus(TargetPawn);
		StartCharge(TargetPawn);
		return;
	}

	Super::UpdateAIBehavior(AIController, TargetPawn);
}

void ASXEnemyCharger::StartCharge(APawn* TargetPawn)
{
	if (IsValid(TargetPawn) == false)
	{
		return;
	}

	const FVector ToTarget = TargetPawn->GetActorLocation() - GetActorLocation();
	ChargeDirection = FVector(ToTarget.X, ToTarget.Y, 0.0f).GetSafeNormal();
	if (ChargeDirection.IsNearlyZero())
	{
		return;
	}

	bIsCharging = true;
	bChargeDamageApplied = false;
	ChargeTarget = TargetPawn;
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	GetCharacterMovement()->MaxWalkSpeed = ChargeSpeed;
}

void ASXEnemyCharger::StopCharge()
{
	if (bIsCharging == false)
	{
		return;
	}

	bIsCharging = false;
	ChargeTarget.Reset();
	LastChargeEndTime = GetWorld()->GetTimeSeconds();
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
}

void ASXEnemyCharger::TryApplyChargeDamage(APawn* TargetPawn)
{
	if (bChargeDamageApplied || IsValid(TargetPawn) == false)
	{
		return;
	}

	const float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
	if (DistanceToTarget > ChargeHitRadius)
	{
		return;
	}

	ASXCharacterBase* TargetCharacter = Cast<ASXCharacterBase>(TargetPawn);
	if (IsValid(TargetCharacter) == false || TargetCharacter->IsAlive() == false)
	{
		return;
	}

	bChargeDamageApplied = true;

	FDamageEvent DamageEvent;
	TargetCharacter->TakeDamage(ChargeDamage, DamageEvent, GetController(), this);
}
