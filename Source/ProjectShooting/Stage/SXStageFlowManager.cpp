// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/SXStageFlowManager.h"

#include "Character/SXEnemyCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Stage/SXStageDoor.h"
#include "Stage/SXStageWaveDataAsset.h"
#include "TimerManager.h"

ASXStageFlowManager::ASXStageFlowManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASXStageFlowManager::BeginPlay()
{
	Super::BeginPlay();

	FindStageActors();

	if (IsValid(WaveSpawner))
	{
		WaveSpawner->OnWaveCleared.AddUniqueDynamic(this, &ThisClass::HandleWaveCleared);
	}

	if (bOpenEntranceOnBeginPlay)
	{
		OpenEntrance();
	}

	if (IsValid(ExitDoor))
	{
		ExitDoor->CloseDoor();
	}

	if (bStartStageOnBeginPlay)
	{
		StartStage();
	}
}

void ASXStageFlowManager::OpenEntrance()
{
	if (IsValid(EntranceDoor))
	{
		EntranceDoor->OpenDoor();
	}
}

int32 ASXStageFlowManager::GetTotalWaveCount() const
{
	return GetActiveStageWaves().Num();
}

void ASXStageFlowManager::LogActiveWaveData() const
{
	const TArray<FSXStageWaveData>& ActiveStageWaves = GetActiveStageWaves();
	UE_LOG(LogTemp, Log, TEXT("SXStageFlowManager %s uses %s. Total Waves: %d"),
		*GetName(),
		*GetActiveWaveDataSourceName(),
		ActiveStageWaves.Num());

	for (int32 WaveIndex = 0; WaveIndex < ActiveStageWaves.Num(); ++WaveIndex)
	{
		const FSXStageWaveData& WaveData = ActiveStageWaves[WaveIndex];
		UE_LOG(LogTemp, Log, TEXT("  Wave %d: SpawnCount=%d, Entries=%d, StartDelay=%.2f, TimeLimit=%.2f, Boss=%s"),
			WaveIndex + 1,
			GetWaveSpawnCount(WaveData),
			WaveData.Enemies.Num(),
			WaveData.StartDelay,
			WaveData.TimeLimit,
			WaveData.bBossWave ? TEXT("true") : TEXT("false"));

		for (int32 EntryIndex = 0; EntryIndex < WaveData.Enemies.Num(); ++EntryIndex)
		{
			const FSXWaveEnemySpawnData& SpawnData = WaveData.Enemies[EntryIndex];
			UE_LOG(LogTemp, Log, TEXT("    Entry %d: Class=%s, Count=%d"),
				EntryIndex,
				*GetNameSafe(SpawnData.EnemyClass.Get()),
				SpawnData.SpawnCount);
		}
	}
}

void ASXStageFlowManager::StartStage()
{
	if (bStageStarted && !bStageCleared && !bStageFailed)
	{
		return;
	}

	FindStageActors();

	if (IsValid(WaveSpawner) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXStageFlowManager %s has no WaveSpawner."), *GetName());
		return;
	}

	bStageStarted = true;
	bStageCleared = false;
	bStageFailed = false;
	CurrentStageWaveIndex = INDEX_NONE;

	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveTimeLimitTimerHandle);

	if (IsValid(EntranceDoor))
	{
		EntranceDoor->CloseDoor();
	}

	if (IsValid(EntranceBarrier))
	{
		EntranceBarrier->CloseDoor();
	}

	if (IsValid(ExitDoor))
	{
		ExitDoor->CloseDoor();
	}

	OnStageStarted.Broadcast();
	StartNextWave();
}

void ASXStageFlowManager::FailStage()
{
	if (bStageFailed || bStageCleared)
	{
		return;
	}

	bStageFailed = true;
	bStageStarted = false;

	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveTimeLimitTimerHandle);

	OnStageFailed.Broadcast();
}

void ASXStageFlowManager::ClearStage()
{
	if (bStageCleared)
	{
		return;
	}

	bStageCleared = true;
	bStageStarted = false;

	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveTimeLimitTimerHandle);

	if (IsValid(ExitDoor))
	{
		ExitDoor->OpenDoor();
	}

	OnStageCleared.Broadcast();
}

void ASXStageFlowManager::HandleWaveCleared(int32 WaveIndex)
{
	if (bStageFailed || bStageCleared)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(WaveTimeLimitTimerHandle);

	const TArray<FSXStageWaveData>& ActiveStageWaves = GetActiveStageWaves();
	if (CurrentStageWaveIndex >= ActiveStageWaves.Num() - 1)
	{
		ClearStage();
		return;
	}

	StartNextWave();
}

void ASXStageFlowManager::FindStageActors()
{
	if (bAutoFindWaveSpawner && IsValid(WaveSpawner) == false)
	{
		TArray<AActor*> FoundSpawners;
		UGameplayStatics::GetAllActorsOfClass(this, ASXWaveSpawner::StaticClass(), FoundSpawners);

		if (FoundSpawners.Num() == 1)
		{
			WaveSpawner = Cast<ASXWaveSpawner>(FoundSpawners[0]);
			return;
		}

		if (FoundSpawners.Num() > 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("SXStageFlowManager %s found %d WaveSpawners. Set WaveSpawner explicitly and disable AutoFind."),
				*GetName(),
				FoundSpawners.Num());
		}
	}
}

void ASXStageFlowManager::StartNextWave()
{
	if (bStageFailed || bStageCleared)
	{
		return;
	}

	++CurrentStageWaveIndex;

	const TArray<FSXStageWaveData>& ActiveStageWaves = GetActiveStageWaves();
	if (ActiveStageWaves.IsValidIndex(CurrentStageWaveIndex) == false)
	{
		ClearStage();
		return;
	}

	const FSXStageWaveData& WaveData = ActiveStageWaves[CurrentStageWaveIndex];
	const float Delay = FMath::Max(0.0f, WaveData.StartDelay);

	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	if (Delay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(NextWaveTimerHandle, this, &ThisClass::BeginCurrentWave, Delay, false);
		return;
	}

	BeginCurrentWave();
}

void ASXStageFlowManager::BeginCurrentWave()
{
	const TArray<FSXStageWaveData>& ActiveStageWaves = GetActiveStageWaves();
	if (bStageFailed || bStageCleared || ActiveStageWaves.IsValidIndex(CurrentStageWaveIndex) == false)
	{
		return;
	}

	if (IsValid(WaveSpawner) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXStageFlowManager %s cannot start wave without WaveSpawner."), *GetName());
		return;
	}

	const FSXStageWaveData& WaveData = ActiveStageWaves[CurrentStageWaveIndex];
	const int32 ExpectedSpawnCount = GetWaveSpawnCount(WaveData);
	const int32 StageWaveNumber = CurrentStageWaveIndex + 1;

	UE_LOG(LogTemp, Log, TEXT("SXStageFlowManager %s starting Wave %d/%d from %s. ExpectedSpawnCount=%d"),
		*GetName(),
		StageWaveNumber,
		ActiveStageWaves.Num(),
		*GetActiveWaveDataSourceName(),
		ExpectedSpawnCount);

	OnStageWaveStarted.Broadcast(StageWaveNumber, ActiveStageWaves.Num(), ExpectedSpawnCount);

	if (WaveData.TimeLimit > 0.0f)
	{
		GetWorldTimerManager().SetTimer(WaveTimeLimitTimerHandle, this, &ThisClass::FailStage, WaveData.TimeLimit, false);
	}

	WaveSpawner->StartWaveFromData(WaveData);
}

const TArray<FSXStageWaveData>& ASXStageFlowManager::GetActiveStageWaves() const
{
	if (bUseStageWaveDataAsset && IsValid(StageWaveDataAsset))
	{
		return StageWaveDataAsset->GetStageWaves();
	}

	return StageWaves;
}

FString ASXStageFlowManager::GetActiveWaveDataSourceName() const
{
	if (bUseStageWaveDataAsset && IsValid(StageWaveDataAsset))
	{
		return FString::Printf(TEXT("DataAsset:%s"), *GetNameSafe(StageWaveDataAsset.Get()));
	}

	return TEXT("StageFlowManager.StageWaves");
}

int32 ASXStageFlowManager::GetWaveSpawnCount(const FSXStageWaveData& WaveData) const
{
	int32 Result = 0;
	for (const FSXWaveEnemySpawnData& SpawnData : WaveData.Enemies)
	{
		if (IsValid(SpawnData.EnemyClass))
		{
			Result += FMath::Max(0, SpawnData.SpawnCount);
		}
	}

	return Result;
}
