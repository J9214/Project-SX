// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SXEnemyCharacterBase.h"

#include "Controller/SXEnemyAIController.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Item/SXCollectiblePickup.h"
#include "Item/SXDropDatabase.h"
#include "TimerManager.h"

FSXOnEnemyKilledNativeSignature ASXEnemyCharacterBase::OnEnemyKilledNative;

ASXEnemyCharacterBase::ASXEnemyCharacterBase()
{
	AIControllerClass = ASXEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	bUseDeathDissolve = true;
	bDestroyAfterDeathDissolve = true;
}

bool ASXEnemyCharacterBase::TryAttack(AActor* TargetActor)
{
	if (IsAlive() == false || bIsAttacking || IsValid(TargetActor) == false)
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
	bIsAttacking = true;
	bHasAppliedDamageThisAttack = false;
	CurrentAttackTarget = TargetCharacter;

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
		// Preserve functional attacks for enemy blueprints that do not have
		// an attack montage assigned yet.
		HandleAttackHitNotify();
		FinishAttack();
	}

	return true;
}

void ASXEnemyCharacterBase::HandleAttackHitNotify()
{
	if (IsAlive() == false || bIsAttacking == false || bHasAppliedDamageThisAttack)
	{
		return;
	}

	ASXCharacterBase* TargetCharacter = CurrentAttackTarget.Get();
	if (IsValid(TargetCharacter) == false || TargetCharacter->IsAlive() == false)
	{
		return;
	}

	const float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetCharacter->GetActorLocation());
	if (DistanceToTarget > AttackRange)
	{
		return;
	}

	bHasAppliedDamageThisAttack = true;

	FDamageEvent DamageEvent;
	TargetCharacter->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
}

void ASXEnemyCharacterBase::FinishAttack()
{
	GetWorldTimerManager().ClearTimer(AttackFinishTimerHandle);

	bIsAttacking = false;
	bHasAppliedDamageThisAttack = false;
	CurrentAttackTarget = nullptr;
}

void ASXEnemyCharacterBase::UpdateAIBehavior(ASXEnemyAIController* AIController, APawn* TargetPawn)
{
	if (IsValid(AIController) == false || IsAlive() == false || IsValid(TargetPawn) == false)
	{
		return;
	}

	if (bIsAttacking)
	{
		AIController->StopMovement();
		AIController->SetFocus(TargetPawn);
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

	GetWorldTimerManager().ClearTimer(AttackFinishTimerHandle);
	StopAnimMontage(AttackMeleeMontage);
	bIsAttacking = false;
	bHasAppliedDamageThisAttack = false;
	CurrentAttackTarget = nullptr;

	Super::Die(DamageCauser);

	if (bWasAlive)
	{
		SpawnDropItems(DamageCauser);
		OnEnemyDeath.Broadcast(this);
		OnEnemyKilledNative.Broadcast(this, DamageCauser);
	}
}

void ASXEnemyCharacterBase::SetDropModifier(const FSXDropModifier& InDropModifier)
{
	DropModifier = InDropModifier;
}

void ASXEnemyCharacterBase::SetDropDatabaseContext(USXDropDatabase* InDropDatabase, FName InDropStageId, int32 InDropWaveNumber)
{
	DropDatabase = InDropDatabase;
	DropStageId = InDropStageId;
	DropWaveNumber = InDropWaveNumber;
}

void ASXEnemyCharacterBase::SpawnDropItems(AActor* DamageCauser)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	TArray<FSXDropItemData> FinalDropItemList = DropItemList;
	FSXDropModifier FinalDropModifier = DropModifier;
	if (IsValid(DropDatabase.Get()))
	{
		DropDatabase->BuildDropData(GetClass(), DropStageId, DropWaveNumber, DropItemList, DropModifier, FinalDropItemList, FinalDropModifier);
	}

	for (const FSXDropItemData& DropItemData : FinalDropItemList)
	{
		float FinalDropChance = DropItemData.DropChance * FinalDropModifier.GlobalDropChanceMultiplier;
		if (DropItemData.bOverrideCollectible)
		{
			switch (DropItemData.CollectibleType)
			{
			case ESXCollectibleType::Gold:
				FinalDropChance *= FinalDropModifier.GoldDropChanceMultiplier;
				break;
			case ESXCollectibleType::Ammo:
				FinalDropChance *= FinalDropModifier.AmmoDropChanceMultiplier;
				break;
			case ESXCollectibleType::Experience:
				FinalDropChance *= FinalDropModifier.ExperienceDropChanceMultiplier;
				break;
			default:
				break;
			}
		}
		else
		{
			FinalDropChance *= FinalDropModifier.OtherDropChanceMultiplier;
		}

		FinalDropChance = FMath::Clamp(FinalDropChance, 0.0f, 1.0f);
		if (DropItemData.DropActorClass == nullptr || FMath::FRand() > FinalDropChance)
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
				float AmountMultiplier = 1.0f;
				if (DropItemData.CollectibleType == ESXCollectibleType::Ammo)
				{
					AmountMultiplier = FinalDropModifier.AmmoAmountMultiplier;
				}
				else if (DropItemData.CollectibleType == ESXCollectibleType::Gold)
				{
					AmountMultiplier = FinalDropModifier.GoldAmountMultiplier;
				}
				else if (DropItemData.CollectibleType == ESXCollectibleType::Experience)
				{
					AmountMultiplier = FinalDropModifier.ExperienceAmountMultiplier;
				}

				const int32 DropAmount = FMath::Max(1, FMath::RoundToInt(FMath::RandRange(MinAmount, MaxAmount) * AmountMultiplier));
				if (DropItemData.CollectibleType == ESXCollectibleType::Ammo)
				{
					CollectiblePickup->InitializeAmmoCollectible(DropItemData.AmmoType, DropAmount);
				}
				else
				{
					CollectiblePickup->InitializeCollectible(DropItemData.CollectibleType, DropAmount);
				}
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
