// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/SXWaveSpawner.h"

#include "Character/SXEnemyCharacterBase.h"
#include "Engine/World.h"

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

void ASXWaveSpawner::StartWave()
{
	StartWaveWithCount(SpawnCount);
}

void ASXWaveSpawner::StartWaveWithCount(int32 InSpawnCount)
{
	FSXWaveEnemySpawnData SpawnData;
	SpawnData.EnemyClass = EnemyClass;
	SpawnData.SpawnCount = InSpawnCount;

	TArray<FSXWaveEnemySpawnData> SpawnEntries;
	SpawnEntries.Add(SpawnData);
	StartWaveWithEntries(SpawnEntries);
}

void ASXWaveSpawner::StartWaveFromData(const FSXStageWaveData& WaveData)
{
	StartWaveWithEntries(WaveData.Enemies);
}

void ASXWaveSpawner::StartWaveWithEntries(const TArray<FSXWaveEnemySpawnData>& SpawnEntries)
{
	if (bWaveInProgress)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	++CurrentWaveIndex;
	bWaveInProgress = true;
	bWaveCleared = false;
	SpawnedEnemyCount = 0;
	AliveEnemyCount = 0;
	AliveEnemies.Reset();

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
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			ASXEnemyCharacterBase* SpawnedEnemy = World->SpawnActor<ASXEnemyCharacterBase>(
				SpawnEntry.EnemyClass,
				GetSpawnTransform(TotalSpawnIndex),
				SpawnParams
			);

			++TotalSpawnIndex;
			RegisterSpawnedEnemy(SpawnedEnemy);
		}
	}

	BroadcastEnemyCountChanged();

	if (AliveEnemyCount <= 0)
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

void ASXWaveSpawner::HandleEnemyDeath(ASXEnemyCharacterBase* DeadEnemy)
{
	RemoveAliveEnemy(DeadEnemy);
}

void ASXWaveSpawner::HandleSpawnedEnemyDestroyed(AActor* DestroyedActor)
{
	ASXEnemyCharacterBase* DestroyedEnemy = Cast<ASXEnemyCharacterBase>(DestroyedActor);
	RemoveAliveEnemy(DestroyedEnemy);
}

FTransform ASXWaveSpawner::GetSpawnTransform(int32 SpawnIndex) const
{
	if (SpawnPoints.Num() <= 0)
	{
		return GetActorTransform();
	}

	const int32 SpawnPointIndex = SpawnIndex % SpawnPoints.Num();
	if (IsValid(SpawnPoints[SpawnPointIndex]) == false)
	{
		return GetActorTransform();
	}

	return SpawnPoints[SpawnPointIndex]->GetActorTransform();
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

	if (bWaveInProgress && AliveEnemyCount <= 0)
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
