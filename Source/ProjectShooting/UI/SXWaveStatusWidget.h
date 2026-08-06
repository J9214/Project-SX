// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SXWaveStatusWidget.generated.h"

class ASXStageFlowManager;
class ASXWaveSpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSXWaveUIStageEventSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSXWaveUIStartedSignature, int32, StageWaveNumber, int32, TotalWaveCount, int32, SpawnCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSXWaveUIEnemyCountChangedSignature, int32, AliveEnemyCount, int32, SpawnedEnemyCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXWaveUIClearedSignature, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FSXWaveUIStatusChangedSignature, int32, StageWaveNumber, int32, TotalWaveCount, int32, AliveEnemyCount, int32, SpawnedEnemyCount, int32, ExpectedSpawnCount, bool, bWaveInProgress);

UCLASS(Blueprintable)
class PROJECTSHOOTING_API USXWaveStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="SX|WaveUI")
	void InitWaveStatus(ASXStageFlowManager* InStageFlowManager, ASXWaveSpawner* InWaveSpawner);

	UFUNCTION(BlueprintCallable, Category="SX|WaveUI")
	void AutoFindWaveSources();

	UFUNCTION(BlueprintCallable, Category="SX|WaveUI")
	void RefreshWaveStatus();

	UFUNCTION(BlueprintPure, Category="SX|WaveUI")
	int32 GetCachedStageWaveNumber() const { return CachedStageWaveNumber; }

	UFUNCTION(BlueprintPure, Category="SX|WaveUI")
	int32 GetCachedTotalWaveCount() const { return CachedTotalWaveCount; }

	UFUNCTION(BlueprintPure, Category="SX|WaveUI")
	int32 GetCachedExpectedSpawnCount() const { return CachedExpectedSpawnCount; }

	UFUNCTION(BlueprintPure, Category="SX|WaveUI")
	int32 GetCachedAliveEnemyCount() const { return CachedAliveEnemyCount; }

	UFUNCTION(BlueprintPure, Category="SX|WaveUI")
	int32 GetCachedSpawnedEnemyCount() const { return CachedSpawnedEnemyCount; }

	UFUNCTION(BlueprintPure, Category="SX|WaveUI")
	bool IsCachedWaveInProgress() const { return bCachedWaveInProgress; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|WaveUI", meta=(ExposeOnSpawn="true"))
	TObjectPtr<ASXStageFlowManager> StageFlowManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|WaveUI", meta=(ExposeOnSpawn="true"))
	TObjectPtr<ASXWaveSpawner> WaveSpawner;

	UPROPERTY(BlueprintAssignable, Category="SX|WaveUI")
	FSXWaveUIStageEventSignature OnWaveUIStageStarted;

	UPROPERTY(BlueprintAssignable, Category="SX|WaveUI")
	FSXWaveUIStartedSignature OnWaveUIStarted;

	UPROPERTY(BlueprintAssignable, Category="SX|WaveUI")
	FSXWaveUIEnemyCountChangedSignature OnWaveUIEnemyCountChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|WaveUI")
	FSXWaveUIClearedSignature OnWaveUICleared;

	UPROPERTY(BlueprintAssignable, Category="SX|WaveUI")
	FSXWaveUIStageEventSignature OnWaveUIStageCleared;

	UPROPERTY(BlueprintAssignable, Category="SX|WaveUI")
	FSXWaveUIStageEventSignature OnWaveUIStageFailed;

	UPROPERTY(BlueprintAssignable, Category="SX|WaveUI")
	FSXWaveUIStatusChangedSignature OnWaveUIStatusChanged;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindWaveSources();
	void UnbindWaveSources();
	void BroadcastCachedStatus();

	UFUNCTION()
	void HandleStageStarted();

	UFUNCTION()
	void HandleStageWaveStarted(int32 StageWaveNumber, int32 TotalWaveCount, int32 SpawnCount);

	UFUNCTION()
	void HandleStageCleared();

	UFUNCTION()
	void HandleStageFailed();

	UFUNCTION()
	void HandleSpawnerWaveStarted(int32 WaveNumber, int32 SpawnCount);

	UFUNCTION()
	void HandleSpawnerEnemyCountChanged(int32 AliveEnemyCount, int32 SpawnedEnemyCount);

	UFUNCTION()
	void HandleSpawnerWaveCleared(int32 WaveNumber);

	int32 CachedStageWaveNumber = 0;
	int32 CachedTotalWaveCount = 0;
	int32 CachedExpectedSpawnCount = 0;
	int32 CachedAliveEnemyCount = 0;
	int32 CachedSpawnedEnemyCount = 0;
	bool bCachedWaveInProgress = false;
};
