// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/SXWaveStatusWidget.h"

#include "Kismet/GameplayStatics.h"
#include "Stage/SXStageFlowManager.h"
#include "Stage/SXWaveSpawner.h"

void USXWaveStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AutoFindWaveSources();
	BindWaveSources();
	RefreshWaveStatus();
}

void USXWaveStatusWidget::NativeDestruct()
{
	UnbindWaveSources();

	Super::NativeDestruct();
}

void USXWaveStatusWidget::InitWaveStatus(ASXStageFlowManager* InStageFlowManager, ASXWaveSpawner* InWaveSpawner)
{
	UnbindWaveSources();

	StageFlowManager = InStageFlowManager;
	WaveSpawner = InWaveSpawner;

	if (IsValid(StageFlowManager) && IsValid(WaveSpawner) == false)
	{
		WaveSpawner = StageFlowManager->GetWaveSpawner();
	}

	BindWaveSources();
	RefreshWaveStatus();
}

void USXWaveStatusWidget::AutoFindWaveSources()
{
	if (IsValid(StageFlowManager) == false)
	{
		StageFlowManager = Cast<ASXStageFlowManager>(UGameplayStatics::GetActorOfClass(this, ASXStageFlowManager::StaticClass()));
	}

	if (IsValid(WaveSpawner) == false)
	{
		if (IsValid(StageFlowManager))
		{
			WaveSpawner = StageFlowManager->GetWaveSpawner();
		}

		if (IsValid(WaveSpawner) == false)
		{
			WaveSpawner = Cast<ASXWaveSpawner>(UGameplayStatics::GetActorOfClass(this, ASXWaveSpawner::StaticClass()));
		}
	}
}

void USXWaveStatusWidget::RefreshWaveStatus()
{
	if (IsValid(StageFlowManager))
	{
		CachedStageWaveNumber = StageFlowManager->GetCurrentStageWaveNumber();
		CachedTotalWaveCount = StageFlowManager->GetTotalWaveCount();
		CachedExpectedSpawnCount = StageFlowManager->GetCurrentWaveSpawnCount();
		bCachedWaveInProgress = StageFlowManager->IsStageStarted()
			&& StageFlowManager->IsStageCleared() == false
			&& StageFlowManager->IsStageFailed() == false
			&& CachedStageWaveNumber > 0;
	}
	else if (IsValid(WaveSpawner))
	{
		CachedStageWaveNumber = WaveSpawner->GetCurrentWaveIndex();
		CachedTotalWaveCount = 0;
		CachedExpectedSpawnCount = WaveSpawner->GetSpawnedEnemyCount();
		bCachedWaveInProgress = WaveSpawner->IsWaveInProgress();
	}
	else
	{
		CachedStageWaveNumber = 0;
		CachedTotalWaveCount = 0;
		CachedExpectedSpawnCount = 0;
		bCachedWaveInProgress = false;
	}

	if (IsValid(WaveSpawner))
	{
		CachedAliveEnemyCount = WaveSpawner->GetAliveEnemyCount();
		CachedSpawnedEnemyCount = WaveSpawner->GetSpawnedEnemyCount();
	}
	else
	{
		CachedAliveEnemyCount = 0;
		CachedSpawnedEnemyCount = 0;
	}

	BroadcastCachedStatus();
}

void USXWaveStatusWidget::BindWaveSources()
{
	if (IsValid(StageFlowManager))
	{
		StageFlowManager->OnStageStarted.AddUniqueDynamic(this, &ThisClass::HandleStageStarted);
		StageFlowManager->OnStageWaveStarted.AddUniqueDynamic(this, &ThisClass::HandleStageWaveStarted);
		StageFlowManager->OnStageCleared.AddUniqueDynamic(this, &ThisClass::HandleStageCleared);
		StageFlowManager->OnStageFailed.AddUniqueDynamic(this, &ThisClass::HandleStageFailed);
	}

	if (IsValid(WaveSpawner))
	{
		WaveSpawner->OnWaveStarted.AddUniqueDynamic(this, &ThisClass::HandleSpawnerWaveStarted);
		WaveSpawner->OnEnemyCountChanged.AddUniqueDynamic(this, &ThisClass::HandleSpawnerEnemyCountChanged);
		WaveSpawner->OnWaveCleared.AddUniqueDynamic(this, &ThisClass::HandleSpawnerWaveCleared);
	}
}

void USXWaveStatusWidget::UnbindWaveSources()
{
	if (IsValid(StageFlowManager))
	{
		StageFlowManager->OnStageStarted.RemoveDynamic(this, &ThisClass::HandleStageStarted);
		StageFlowManager->OnStageWaveStarted.RemoveDynamic(this, &ThisClass::HandleStageWaveStarted);
		StageFlowManager->OnStageCleared.RemoveDynamic(this, &ThisClass::HandleStageCleared);
		StageFlowManager->OnStageFailed.RemoveDynamic(this, &ThisClass::HandleStageFailed);
	}

	if (IsValid(WaveSpawner))
	{
		WaveSpawner->OnWaveStarted.RemoveDynamic(this, &ThisClass::HandleSpawnerWaveStarted);
		WaveSpawner->OnEnemyCountChanged.RemoveDynamic(this, &ThisClass::HandleSpawnerEnemyCountChanged);
		WaveSpawner->OnWaveCleared.RemoveDynamic(this, &ThisClass::HandleSpawnerWaveCleared);
	}
}

void USXWaveStatusWidget::BroadcastCachedStatus()
{
	OnWaveUIStatusChanged.Broadcast(
		CachedStageWaveNumber,
		CachedTotalWaveCount,
		CachedAliveEnemyCount,
		CachedSpawnedEnemyCount,
		CachedExpectedSpawnCount,
		bCachedWaveInProgress);
}

void USXWaveStatusWidget::HandleStageStarted()
{
	OnWaveUIStageStarted.Broadcast();
	RefreshWaveStatus();
}

void USXWaveStatusWidget::HandleStageWaveStarted(int32 StageWaveNumber, int32 TotalWaveCount, int32 SpawnCount)
{
	CachedStageWaveNumber = StageWaveNumber;
	CachedTotalWaveCount = TotalWaveCount;
	CachedExpectedSpawnCount = SpawnCount;
	bCachedWaveInProgress = true;

	if (IsValid(WaveSpawner))
	{
		CachedAliveEnemyCount = WaveSpawner->GetAliveEnemyCount();
		CachedSpawnedEnemyCount = WaveSpawner->GetSpawnedEnemyCount();
	}

	OnWaveUIStarted.Broadcast(CachedStageWaveNumber, CachedTotalWaveCount, CachedExpectedSpawnCount);
	BroadcastCachedStatus();
}

void USXWaveStatusWidget::HandleStageCleared()
{
	bCachedWaveInProgress = false;
	CachedAliveEnemyCount = 0;
	OnWaveUIStageCleared.Broadcast();
	BroadcastCachedStatus();
}

void USXWaveStatusWidget::HandleStageFailed()
{
	bCachedWaveInProgress = false;
	OnWaveUIStageFailed.Broadcast();
	BroadcastCachedStatus();
}

void USXWaveStatusWidget::HandleSpawnerWaveStarted(int32 WaveNumber, int32 SpawnCount)
{
	if (IsValid(StageFlowManager) == false)
	{
		CachedStageWaveNumber = WaveNumber;
		CachedTotalWaveCount = 0;
		CachedExpectedSpawnCount = SpawnCount;
		OnWaveUIStarted.Broadcast(CachedStageWaveNumber, CachedTotalWaveCount, CachedExpectedSpawnCount);
	}

	CachedSpawnedEnemyCount = SpawnCount;
	bCachedWaveInProgress = true;
	BroadcastCachedStatus();
}

void USXWaveStatusWidget::HandleSpawnerEnemyCountChanged(int32 AliveEnemyCount, int32 SpawnedEnemyCount)
{
	CachedAliveEnemyCount = AliveEnemyCount;
	CachedSpawnedEnemyCount = SpawnedEnemyCount;

	OnWaveUIEnemyCountChanged.Broadcast(CachedAliveEnemyCount, CachedSpawnedEnemyCount);
	BroadcastCachedStatus();
}

void USXWaveStatusWidget::HandleSpawnerWaveCleared(int32 WaveNumber)
{
	bCachedWaveInProgress = false;
	CachedAliveEnemyCount = 0;

	OnWaveUICleared.Broadcast(WaveNumber);
	BroadcastCachedStatus();
}
