// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SXEnemyCharacterBase.h"

#include "Controller/SXEnemyAIController.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Pawn.h"
#include "Item/SXCollectiblePickup.h"

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

void ASXEnemyCharacterBase::UpdateAIBehavior(ASXEnemyAIController* AIController, APawn* TargetPawn)
{
	if (IsValid(AIController) == false || IsAlive() == false || IsValid(TargetPawn) == false)
	{
		return;
	}

	const float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
	if (DistanceToTarget > AttackRange)
	{
		AIController->MoveToActor(TargetPawn, FMath::Max(0.0f, AttackRange - 20.0f));
		return;
	}

	AIController->StopMovement();
	AIController->SetFocus(TargetPawn);
	TryAttack(TargetPawn);
}

void ASXEnemyCharacterBase::Die(AActor* DamageCauser)
{
	const bool bWasAlive = bIsDead == false;

	Super::Die(DamageCauser);

	if (bWasAlive)
	{
		SpawnDropItems(DamageCauser);
		OnEnemyDeath.Broadcast(this);
	}
}

void ASXEnemyCharacterBase::SpawnDropItems(AActor* DamageCauser)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	for (const FSXDropItemData& DropItemData : DropItemList)
	{
		if (DropItemData.DropActorClass == nullptr || FMath::FRand() > DropItemData.DropChance)
		{
			continue;
		}

		const int32 MinCount = FMath::Max(1, DropItemData.MinCount);
		const int32 MaxCount = FMath::Max(MinCount, DropItemData.MaxCount);
		const int32 DropCount = FMath::RandRange(MinCount, MaxCount);

		for (int32 Index = 0; Index < DropCount; ++Index)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = this;
			SpawnParameters.Instigator = GetInstigator();
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AActor* DropActor = World->SpawnActor<AActor>(
				DropItemData.DropActorClass,
				GetDropSpawnLocation(DropItemData),
				FRotator::ZeroRotator,
				SpawnParameters
			);

			ASXCollectiblePickup* CollectiblePickup = Cast<ASXCollectiblePickup>(DropActor);
			if (IsValid(CollectiblePickup) == true && DropItemData.bOverrideCollectible == true)
			{
				const int32 MinAmount = FMath::Max(1, DropItemData.MinAmount);
				const int32 MaxAmount = FMath::Max(MinAmount, DropItemData.MaxAmount);
				CollectiblePickup->InitializeCollectible(DropItemData.CollectibleType, FMath::RandRange(MinAmount, MaxAmount));
			}
		}
	}
}

FVector ASXEnemyCharacterBase::GetDropSpawnLocation(const FSXDropItemData& DropItemData) const
{
	const float RandomDistance = FMath::RandRange(0.0f, DropItemData.SpawnRadius);
	const float RandomAngle = FMath::RandRange(0.0f, 360.0f);
	const FVector RandomOffset = FVector(FMath::Cos(FMath::DegreesToRadians(RandomAngle)), FMath::Sin(FMath::DegreesToRadians(RandomAngle)), 0.0f) * RandomDistance;

	return GetActorLocation() + RandomOffset + FVector(0.0f, 0.0f, 30.0f);
}
