// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/SXWaveSpawner.h"

#include "Character/SXEnemyCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

ASXWaveSpawner::ASXWaveSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASXWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bStartOnBeginPlay)
	{
		StartWave();
	}
}

void ASXWaveSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	PendingSpawnRequests.Reset();
	Super::EndPlay(EndPlayReason);
}

void ASXWaveSpawner::StartWave()
{
	StartWaveWithCount(SpawnCount);
}

void ASXWaveSpawner::StartWaveWithCount(int32 InSpawnCount)
{
	FSXWaveEnemySpawnData SpawnData;
	SpawnData.EnemyClass = EnemyClass;
	SpawnData.SpawnCount = InSpawnCount;
	SpawnData.SpawnGroup = TEXT("Default");

	TArray<FSXWaveEnemySpawnData> SpawnEntries;
	SpawnEntries.Add(SpawnData);
	StartWaveWithEntries(SpawnEntries, FSXDropModifier());
}

void ASXWaveSpawner::StartWaveFromData(const FSXStageWaveData& WaveData)
{
	StartWaveWithEntries(WaveData.Enemies, WaveData.DropModifier);
}

void ASXWaveSpawner::StartWaveWithEntries(const TArray<FSXWaveEnemySpawnData>& SpawnEntries, const FSXDropModifier& InDropModifier)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	++CurrentWaveIndex;
	bWaveInProgress = true;
	bWaveCleared = false;

	int32 FinalSpawnCount = 0;
	for (const FSXWaveEnemySpawnData& SpawnEntry : SpawnEntries)
	{
		if (IsValid(SpawnEntry.EnemyClass))
		{
			FinalSpawnCount += FMath::Max(0, SpawnEntry.SpawnCount);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SXWaveSpawner %s starting Wave %d. Entries=%d, ExpectedSpawnCount=%d"),
		*GetName(),
		CurrentWaveIndex,
		SpawnEntries.Num(),
		FinalSpawnCount);

	OnWaveStarted.Broadcast(CurrentWaveIndex, FinalSpawnCount);

	int32 TotalSpawnIndex = 0;
	for (const FSXWaveEnemySpawnData& SpawnEntry : SpawnEntries)
	{
		if (IsValid(SpawnEntry.EnemyClass) == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("SXWaveSpawner %s has an invalid EnemyClass in wave data."), *GetName());
			continue;
		}

		const int32 EntrySpawnCount = FMath::Max(0, SpawnEntry.SpawnCount);
		UE_LOG(LogTemp, Log, TEXT("  SpawnEntry: Class=%s, Count=%d"),
			*GetNameSafe(SpawnEntry.EnemyClass.Get()),
			EntrySpawnCount);

		for (int32 SpawnIndex = 0; SpawnIndex < EntrySpawnCount; ++SpawnIndex)
		{
			FSXPendingWaveSpawnRequest& SpawnRequest = PendingSpawnRequests.AddDefaulted_GetRef();
			SpawnRequest.EnemyClass = SpawnEntry.EnemyClass;
			SpawnRequest.SpawnGroup = SpawnEntry.SpawnGroup;
			SpawnRequest.SpawnIndex = TotalSpawnIndex;
			SpawnRequest.WaveIndex = CurrentWaveIndex;
			SpawnRequest.DropModifier = InDropModifier;
			SpawnRequest.DropDatabase = ActiveDropDatabase;
			SpawnRequest.DropStageId = ActiveDropStageId;
			SpawnRequest.DropStageWaveNumber = ActiveDropStageWaveNumber;
			++TotalSpawnIndex;
		}
	}

	BroadcastEnemyCountChanged();

	if (PendingSpawnRequests.Num() > 0 && GetWorldTimerManager().IsTimerActive(SpawnTimerHandle) == false)
	{
		ProcessNextSpawnRequest();
	}
	else if (AliveEnemyCount <= 0 && PendingSpawnRequests.Num() <= 0)
	{
		CompleteWave();
	}
}

void ASXWaveSpawner::ProcessNextSpawnRequest()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	if (PendingSpawnRequests.Num() <= 0)
	{
		if (AliveEnemyCount <= 0)
		{
			CompleteWave();
		}
		return;
	}

	const FSXPendingWaveSpawnRequest SpawnRequest = PendingSpawnRequests[0];
	PendingSpawnRequests.RemoveAt(0, 1, EAllowShrinking::No);

	FTransform SpawnTransform;
	if (FindSafeSpawnTransform(SpawnRequest, SpawnTransform))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		// FindSafeSpawnTransform already validates NavMesh reachability, blocking
		// geometry and enemy separation. Using DontSpawnIfColliding here performs
		// another engine encroachment test and can reject a valid capsule merely
		// touching the floor, reducing the actual wave count.
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASXEnemyCharacterBase* SpawnedEnemy = GetWorld()->SpawnActor<ASXEnemyCharacterBase>(
			SpawnRequest.EnemyClass,
			SpawnTransform,
			SpawnParams
		);

		if (IsValid(SpawnedEnemy))
		{
			SpawnedEnemy->SetDropDatabaseContext(
				SpawnRequest.DropDatabase.Get(),
				SpawnRequest.DropStageId,
				SpawnRequest.DropStageWaveNumber
			);
			SpawnedEnemy->SetDropModifier(SpawnRequest.DropModifier);
		}

		RegisterSpawnedEnemy(SpawnedEnemy);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SXWaveSpawner %s skipped %s after %d failed safe-location attempts."),
			*GetName(),
			*GetNameSafe(SpawnRequest.EnemyClass.Get()),
			FMath::Max(1, MaxSpawnLocationAttempts));
	}

	BroadcastEnemyCountChanged();

	if (PendingSpawnRequests.Num() > 0)
	{
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&ThisClass::ProcessNextSpawnRequest,
			FMath::Max(0.01f, SpawnInterval),
			false
		);
	}
	else if (AliveEnemyCount <= 0)
	{
		CompleteWave();
	}
}

void ASXWaveSpawner::BroadcastWaveState()
{
	if (CurrentWaveIndex > 0)
	{
		OnWaveStarted.Broadcast(CurrentWaveIndex, SpawnedEnemyCount);
	}

	BroadcastEnemyCountChanged();
}

void ASXWaveSpawner::SetDropDatabaseContext(USXDropDatabase* InDropDatabase, FName InStageId, int32 InStageWaveNumber)
{
	ActiveDropDatabase = InDropDatabase;
	ActiveDropStageId = InStageId;
	ActiveDropStageWaveNumber = InStageWaveNumber;
}

void ASXWaveSpawner::HandleEnemyDeath(ASXEnemyCharacterBase* DeadEnemy)
{
	RemoveAliveEnemy(DeadEnemy);
}

void ASXWaveSpawner::HandleSpawnedEnemyDestroyed(AActor* DestroyedActor)
{
	ASXEnemyCharacterBase* DestroyedEnemy = Cast<ASXEnemyCharacterBase>(DestroyedActor);
	RemoveAliveEnemy(DestroyedEnemy);
}

FTransform ASXWaveSpawner::GetSpawnTransform(FName SpawnGroup, int32 SpawnIndex, int32 WaveIndex) const
{
	TArray<const FSXWaveSpawnPointData*> Candidates;
	float TotalWeight = 0.0f;

	for (const FSXWaveSpawnPointData& SpawnPointData : GroupedSpawnPoints)
	{
		if (IsValid(SpawnPointData.SpawnPoint) == false)
		{
			continue;
		}

		if (SpawnPointData.GroupName != SpawnGroup)
		{
			continue;
		}

		const float SafeWeight = FMath::Max(0.0f, SpawnPointData.Weight);
		if (SafeWeight <= 0.0f)
		{
			continue;
		}

		Candidates.Add(&SpawnPointData);
		TotalWeight += SafeWeight;
	}

	if (Candidates.Num() <= 0 && GroupedSpawnPoints.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXWaveSpawner %s cannot find spawn points for group '%s'. Falling back to all grouped spawn points."),
			*GetName(),
			*SpawnGroup.ToString());

		for (const FSXWaveSpawnPointData& SpawnPointData : GroupedSpawnPoints)
		{
			if (IsValid(SpawnPointData.SpawnPoint) == false)
			{
				continue;
			}

			const float SafeWeight = FMath::Max(0.0f, SpawnPointData.Weight);
			if (SafeWeight <= 0.0f)
			{
				continue;
			}

			Candidates.Add(&SpawnPointData);
			TotalWeight += SafeWeight;
		}
	}

	if (Candidates.Num() > 0 && TotalWeight > 0.0f)
	{
		const int32 Seed = static_cast<int32>(
			GetTypeHash(SpawnGroup)
			^ static_cast<uint32>(WaveIndex * 1009)
			^ static_cast<uint32>(SpawnIndex * 9176)
			^ static_cast<uint32>(GetUniqueID())
		);

		FRandomStream RandomStream(Seed);
		float WeightRoll = RandomStream.FRandRange(0.0f, TotalWeight);

		for (const FSXWaveSpawnPointData* Candidate : Candidates)
		{
			const float SafeWeight = FMath::Max(0.0f, Candidate->Weight);
			WeightRoll -= SafeWeight;
			if (WeightRoll <= 0.0f)
			{
				return Candidate->SpawnPoint->GetActorTransform();
			}
		}

		return Candidates.Last()->SpawnPoint->GetActorTransform();
	}

	if (SpawnPoints.Num() > 0)
	{
		const int32 SpawnPointIndex = SpawnIndex % SpawnPoints.Num();
		if (IsValid(SpawnPoints[SpawnPointIndex]))
		{
			return SpawnPoints[SpawnPointIndex]->GetActorTransform();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("SXWaveSpawner %s has no valid grouped spawn points. Falling back to spawner transform."),
		*GetName());
	return GetActorTransform();
}

bool ASXWaveSpawner::FindSafeSpawnTransform(
	const FSXPendingWaveSpawnRequest& SpawnRequest,
	FTransform& OutSpawnTransform
) const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false || IsValid(SpawnRequest.EnemyClass) == false)
	{
		return false;
	}

	const ASXEnemyCharacterBase* EnemyCDO = SpawnRequest.EnemyClass->GetDefaultObject<ASXEnemyCharacterBase>();
	const UCapsuleComponent* Capsule = IsValid(EnemyCDO) ? EnemyCDO->GetCapsuleComponent() : nullptr;
	const float CapsuleRadius = IsValid(Capsule) ? Capsule->GetScaledCapsuleRadius() : 42.0f;
	const float CapsuleHalfHeight = IsValid(Capsule) ? Capsule->GetScaledCapsuleHalfHeight() : 96.0f;
	// Shrink the test capsule slightly so merely touching the NavMesh floor is not
	// treated as penetration. Enemy-to-enemy spacing is checked separately below.
	const float TestRadius = FMath::Max(1.0f, CapsuleRadius - 2.0f);
	const float TestHalfHeight = FMath::Max(TestRadius, CapsuleHalfHeight - 2.0f);

	const FTransform BaseTransform = GetSpawnTransform(
		SpawnRequest.SpawnGroup,
		SpawnRequest.SpawnIndex,
		SpawnRequest.WaveIndex
	);
	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (IsValid(NavigationSystem) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXWaveSpawner %s cannot find a NavigationSystem."), *GetName());
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SXWaveSafeSpawn), false, this);
	const FCollisionShape SpawnShape = FCollisionShape::MakeCapsule(TestRadius, TestHalfHeight);
	const int32 AttemptCount = FMath::Max(1, MaxSpawnLocationAttempts);

	for (int32 AttemptIndex = 0; AttemptIndex < AttemptCount; ++AttemptIndex)
	{
		FNavLocation NavLocation;
		const bool bFoundNavLocation = SpawnSearchRadius > 0.0f
			? NavigationSystem->GetRandomReachablePointInRadius(BaseTransform.GetLocation(), SpawnSearchRadius, NavLocation)
			: NavigationSystem->ProjectPointToNavigation(BaseTransform.GetLocation(), NavLocation);

		if (bFoundNavLocation == false)
		{
			continue;
		}

		FVector CandidateLocation = NavLocation.Location;
		CandidateLocation.Z += CapsuleHalfHeight + FMath::Max(0.0f, SpawnGroundOffset);

		const bool bBlocked = World->OverlapBlockingTestByChannel(
			CandidateLocation,
			FQuat::Identity,
			ECC_Pawn,
			SpawnShape,
			QueryParams
		);

		if (bBlocked)
		{
			continue;
		}

		bool bTooCloseToEnemy = false;
		for (const ASXEnemyCharacterBase* ExistingEnemy : AliveEnemies)
		{
			if (IsValid(ExistingEnemy) == false)
			{
				continue;
			}

			const UCapsuleComponent* ExistingCapsule = ExistingEnemy->GetCapsuleComponent();
			const float ExistingRadius = IsValid(ExistingCapsule)
				? ExistingCapsule->GetScaledCapsuleRadius()
				: 42.0f;
			const float RequiredDistance = CapsuleRadius + ExistingRadius + FMath::Max(0.0f, MinimumEnemySeparation);

			if (FVector::DistSquared2D(CandidateLocation, ExistingEnemy->GetActorLocation()) < FMath::Square(RequiredDistance))
			{
				bTooCloseToEnemy = true;
				break;
			}
		}

		if (bTooCloseToEnemy)
		{
			continue;
		}

		OutSpawnTransform = BaseTransform;
		OutSpawnTransform.SetLocation(CandidateLocation);
		return true;
	}

	return false;
}

void ASXWaveSpawner::RegisterSpawnedEnemy(ASXEnemyCharacterBase* SpawnedEnemy)
{
	if (IsValid(SpawnedEnemy) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXWaveSpawner %s failed to spawn enemy."), *GetName());
		return;
	}

	++SpawnedEnemyCount;
	++AliveEnemyCount;
	AliveEnemies.Add(SpawnedEnemy);

	UE_LOG(LogTemp, Log, TEXT("SXWaveSpawner %s registered enemy %s. Alive=%d, Spawned=%d"),
		*GetName(),
		*GetNameSafe(SpawnedEnemy),
		AliveEnemyCount,
		SpawnedEnemyCount);

	SpawnedEnemy->OnEnemyDeath.AddDynamic(this, &ThisClass::HandleEnemyDeath);
	SpawnedEnemy->OnDestroyed.AddDynamic(this, &ThisClass::HandleSpawnedEnemyDestroyed);
}

void ASXWaveSpawner::RemoveAliveEnemy(ASXEnemyCharacterBase* Enemy)
{
	if (IsValid(Enemy) == false)
	{
		return;
	}

	const int32 RemovedCount = AliveEnemies.Remove(Enemy);
	if (RemovedCount <= 0)
	{
		return;
	}

	AliveEnemyCount = FMath::Max(0, AliveEnemyCount - RemovedCount);
	BroadcastEnemyCountChanged();

	if (bWaveInProgress && AliveEnemyCount <= 0 && PendingSpawnRequests.Num() <= 0)
	{
		CompleteWave();
	}
}

void ASXWaveSpawner::BroadcastEnemyCountChanged()
{
	OnEnemyCountChanged.Broadcast(AliveEnemyCount, SpawnedEnemyCount);
}

void ASXWaveSpawner::CompleteWave()
{
	if (!bWaveInProgress || bWaveCleared)
	{
		return;
	}

	bWaveInProgress = false;
	bWaveCleared = true;
	OnWaveCleared.Broadcast(CurrentWaveIndex);
}
