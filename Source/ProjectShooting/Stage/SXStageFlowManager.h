// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Stage/SXWaveSpawner.h"
#include "SXStageFlowManager.generated.h"

class ASXStageDoor;
class USXStageWaveDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSXOnStageStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSXOnStageWaveStartedSignature, int32, StageWaveIndex, int32, TotalWaveCount, int32, SpawnCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSXOnStageClearedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSXOnStageFailedSignature);

UCLASS()
class PROJECTSHOOTING_API ASXStageFlowManager : public AActor
{
	GENERATED_BODY()

public:
	ASXStageFlowManager();

	UFUNCTION(BlueprintCallable, Category="SX|Stage")
	void OpenEntrance();

	UFUNCTION(BlueprintCallable, Category="SX|Stage")
	void StartStage();

	UFUNCTION(BlueprintCallable, Category="SX|Stage")
	void FailStage();

	UFUNCTION(BlueprintCallable, Category="SX|Stage")
	void ClearStage();

	UFUNCTION(BlueprintPure, Category="SX|Stage")
	int32 GetCurrentStageWaveIndex() const { return CurrentStageWaveIndex; }

	UFUNCTION(BlueprintPure, Category="SX|Stage")
	int32 GetTotalWaveCount() const;

	UFUNCTION(BlueprintPure, Category="SX|Stage")
	bool IsStageStarted() const { return bStageStarted; }

	UFUNCTION(BlueprintPure, Category="SX|Stage")
	bool IsStageCleared() const { return bStageCleared; }

	UFUNCTION(BlueprintPure, Category="SX|Stage")
	bool IsStageFailed() const { return bStageFailed; }

	UFUNCTION(BlueprintCallable, Category="SX|Stage|WaveData")
	void LogActiveWaveData() const;

	UPROPERTY(BlueprintAssignable, Category="SX|Stage")
	FSXOnStageStartedSignature OnStageStarted;

	UPROPERTY(BlueprintAssignable, Category="SX|Stage")
	FSXOnStageWaveStartedSignature OnStageWaveStarted;

	UPROPERTY(BlueprintAssignable, Category="SX|Stage")
	FSXOnStageClearedSignature OnStageCleared;

	UPROPERTY(BlueprintAssignable, Category="SX|Stage")
	FSXOnStageFailedSignature OnStageFailed;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleWaveCleared(int32 WaveIndex);

	void FindStageActors();
	void StartNextWave();
	void BeginCurrentWave();
	int32 GetWaveSpawnCount(const FSXStageWaveData& WaveData) const;
	const TArray<FSXStageWaveData>& GetActiveStageWaves() const;
	FString GetActiveWaveDataSourceName() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	TObjectPtr<ASXWaveSpawner> WaveSpawner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	TObjectPtr<ASXStageDoor> EntranceDoor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	TObjectPtr<ASXStageDoor> EntranceBarrier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	TObjectPtr<ASXStageDoor> ExitDoor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage|WaveData")
	TObjectPtr<USXStageWaveDataAsset> StageWaveDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage|WaveData")
	bool bUseStageWaveDataAsset = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage|WaveData")
	TArray<FSXStageWaveData> StageWaves;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	bool bAutoFindWaveSpawner = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	bool bOpenEntranceOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage")
	bool bStartStageOnBeginPlay = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Stage")
	int32 CurrentStageWaveIndex = INDEX_NONE;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Stage")
	bool bStageStarted = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Stage")
	bool bStageCleared = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Stage")
	bool bStageFailed = false;

	FTimerHandle NextWaveTimerHandle;
	FTimerHandle WaveTimeLimitTimerHandle;
};
