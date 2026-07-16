// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXWaveSpawner.generated.h"

class ASXEnemyCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSXOnWaveStartedSignature, int32, WaveIndex, int32, SpawnCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSXOnWaveEnemyCountChangedSignature, int32, AliveEnemyCount, int32, SpawnedEnemyCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXOnWaveClearedSignature, int32, WaveIndex);

USTRUCT(BlueprintType)
struct FSXWaveEnemySpawnData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	TSubclassOf<ASXEnemyCharacterBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave", meta=(ClampMin="0"))
	int32 SpawnCount = 1;
};

USTRUCT(BlueprintType)
struct FSXStageWaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	TArray<FSXWaveEnemySpawnData> Enemies;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave", meta=(ClampMin="0.0"))
	float StartDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave", meta=(ClampMin="0.0"))
	float TimeLimit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	bool bBossWave = false;
};

UCLASS()
class PROJECTSHOOTING_API ASXWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	ASXWaveSpawner();

	UFUNCTION(BlueprintCallable, Category="SX|Wave")
	void StartWave();

	UFUNCTION(BlueprintCallable, Category="SX|Wave")
	void StartWaveWithCount(int32 InSpawnCount);

	UFUNCTION(BlueprintCallable, Category="SX|Wave")
	void StartWaveFromData(const FSXStageWaveData& WaveData);

	UFUNCTION(BlueprintCallable, Category="SX|Wave")
	void BroadcastWaveState();

	UFUNCTION(BlueprintPure, Category="SX|Wave")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	UFUNCTION(BlueprintPure, Category="SX|Wave")
	int32 GetAliveEnemyCount() const { return AliveEnemyCount; }

	UFUNCTION(BlueprintPure, Category="SX|Wave")
	int32 GetSpawnedEnemyCount() const { return SpawnedEnemyCount; }

	UFUNCTION(BlueprintPure, Category="SX|Wave")
	bool IsWaveInProgress() const { return bWaveInProgress; }

	UFUNCTION(BlueprintPure, Category="SX|Wave")
	bool IsWaveCleared() const { return bWaveCleared; }

	UPROPERTY(BlueprintAssignable, Category="SX|Wave")
	FSXOnWaveStartedSignature OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category="SX|Wave")
	FSXOnWaveEnemyCountChangedSignature OnEnemyCountChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|Wave")
	FSXOnWaveClearedSignature OnWaveCleared;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleEnemyDeath(ASXEnemyCharacterBase* DeadEnemy);

	UFUNCTION()
	void HandleSpawnedEnemyDestroyed(AActor* DestroyedActor);

	FTransform GetSpawnTransform(int32 SpawnIndex) const;
	void StartWaveWithEntries(const TArray<FSXWaveEnemySpawnData>& SpawnEntries);
	void RegisterSpawnedEnemy(ASXEnemyCharacterBase* SpawnedEnemy);
	void RemoveAliveEnemy(ASXEnemyCharacterBase* Enemy);
	void BroadcastEnemyCountChanged();
	void CompleteWave();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	TSubclassOf<ASXEnemyCharacterBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	TArray<TObjectPtr<AActor>> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave", meta=(ClampMin="1"))
	int32 SpawnCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	bool bStartOnBeginPlay = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Wave")
	int32 CurrentWaveIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Wave")
	int32 AliveEnemyCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Wave")
	int32 SpawnedEnemyCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Wave")
	bool bWaveInProgress = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Wave")
	bool bWaveCleared = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Wave")
	TArray<TObjectPtr<ASXEnemyCharacterBase>> AliveEnemies;
};
