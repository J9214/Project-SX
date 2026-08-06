// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SXEnemyCharacterBase.h"
#include "GameFramework/Actor.h"
#include "SXWaveSpawner.generated.h"

class USXDropDatabase;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	FName SpawnGroup = TEXT("Default");
};

USTRUCT(BlueprintType)
struct FSXWaveSpawnPointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	TObjectPtr<AActor> SpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	FName GroupName = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave", meta=(ClampMin="0.0"))
	float Weight = 1.0f;
};

USTRUCT(BlueprintType)
struct FSXStageWaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	TArray<FSXWaveEnemySpawnData> Enemies;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave", meta=(ClampMin="0.0"))
	float StartDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave", meta=(ClampMin="0.0", DisplayName="Next Wave Time", ToolTip="If greater than 0, the next wave starts after this many seconds even while enemies from this wave are still alive."))
	float TimeLimit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave")
	bool bBossWave = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave|Drop")
	FSXDropModifier DropModifier;
};

USTRUCT()
struct FSXPendingWaveSpawnRequest
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<ASXEnemyCharacterBase> EnemyClass;

	UPROPERTY()
	FName SpawnGroup = TEXT("Default");

	UPROPERTY()
	int32 SpawnIndex = 0;

	UPROPERTY()
	int32 WaveIndex = 0;

	UPROPERTY()
	FSXDropModifier DropModifier;

	UPROPERTY()
	TObjectPtr<USXDropDatabase> DropDatabase;

	UPROPERTY()
	FName DropStageId = NAME_None;

	UPROPERTY()
	int32 DropStageWaveNumber = 0;
};

UCLASS()
class PROJECTSHOOTING_API ASXWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	ASXWaveSpawner();

	UFUNCTION(BlueprintCallable, Category="SX|Wave|Deprecated", meta=(DeprecatedFunction, DeprecationMessage="Use SXStageFlowManager with StageWaveDataAsset, or StartWaveFromData()."))
	void StartWave();

	UFUNCTION(BlueprintCallable, Category="SX|Wave|Deprecated", meta=(DeprecatedFunction, DeprecationMessage="Use SXStageFlowManager with StageWaveDataAsset, or StartWaveFromData()."))
	void StartWaveWithCount(int32 InSpawnCount);

	UFUNCTION(BlueprintCallable, Category="SX|Wave")
	void StartWaveFromData(const FSXStageWaveData& WaveData);

	UFUNCTION(BlueprintCallable, Category="SX|Wave")
	void BroadcastWaveState();

	UFUNCTION(BlueprintCallable, Category="SX|Wave|Drop")
	void SetDropDatabaseContext(USXDropDatabase* InDropDatabase, FName InStageId, int32 InStageWaveNumber);

	UFUNCTION(BlueprintPure, Category="SX|Wave")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	UFUNCTION(BlueprintPure, Category="SX|Wave")
	int32 GetAliveEnemyCount() const { return AliveEnemyCount; }

	UFUNCTION(BlueprintPure, Category="SX|Wave")
	int32 GetSpawnedEnemyCount() const { return SpawnedEnemyCount; }

	UFUNCTION(BlueprintPure, Category="SX|Wave")
	bool HasAliveEnemies() const { return AliveEnemyCount > 0 || PendingSpawnRequests.Num() > 0; }

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleEnemyDeath(ASXEnemyCharacterBase* DeadEnemy);

	UFUNCTION()
	void HandleSpawnedEnemyDestroyed(AActor* DestroyedActor);

	FTransform GetSpawnTransform(FName SpawnGroup, int32 SpawnIndex, int32 WaveIndex) const;
	bool FindSafeSpawnTransform(const FSXPendingWaveSpawnRequest& SpawnRequest, FTransform& OutSpawnTransform) const;
	void StartWaveWithEntries(const TArray<FSXWaveEnemySpawnData>& SpawnEntries, const FSXDropModifier& InDropModifier);
	void ProcessNextSpawnRequest();
	void RegisterSpawnedEnemy(ASXEnemyCharacterBase* SpawnedEnemy);
	void RemoveAliveEnemy(ASXEnemyCharacterBase* Enemy);
	void BroadcastEnemyCountChanged();
	void CompleteWave();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave", meta=(TitleProperty="GroupName"))
	TArray<FSXWaveSpawnPointData> GroupedSpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave|Spawn Safety", meta=(ClampMin="0.01", Units=s, ToolTip="Delay between individual enemy spawns. This prevents a full wave from spawning in one frame."))
	float SpawnInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave|Spawn Safety", meta=(ClampMin="0.0", Units=cm, ToolTip="Radius around each spawn point used to find a reachable NavMesh position."))
	float SpawnSearchRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave|Spawn Safety", meta=(ClampMin="0.0", Units=cm, ToolTip="Additional empty space required between a new enemy and blocking geometry or existing enemies."))
	float MinimumEnemySeparation = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave|Spawn Safety", meta=(ClampMin="1", ClampMax="100", ToolTip="Maximum number of candidate NavMesh positions tested for each enemy."))
	int32 MaxSpawnLocationAttempts = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave|Spawn Safety", meta=(ClampMin="0.0", Units=cm))
	float SpawnGroundOffset = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave|Deprecated", meta=(DeprecatedProperty, DeprecationMessage="Use StageWaveDataAsset enemy entries instead."))
	TSubclassOf<ASXEnemyCharacterBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave|Deprecated", meta=(DeprecatedProperty, ClampMin="1", DeprecationMessage="Use StageWaveDataAsset enemy entries instead."))
	int32 SpawnCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave|Deprecated", meta=(DeprecatedProperty, DeprecationMessage="Use SXStageFlowManager to start waves instead."))
	bool bStartOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wave|Deprecated", meta=(DeprecatedProperty, DeprecationMessage="Use Grouped Spawn Points instead."))
	TArray<TObjectPtr<AActor>> SpawnPoints;

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

	UPROPERTY(Transient)
	TArray<TObjectPtr<ASXEnemyCharacterBase>> AliveEnemies;

	UPROPERTY(Transient)
	TArray<FSXPendingWaveSpawnRequest> PendingSpawnRequests;

	UPROPERTY(Transient)
	TObjectPtr<USXDropDatabase> ActiveDropDatabase;

	UPROPERTY(Transient)
	FName ActiveDropStageId = NAME_None;

	UPROPERTY(Transient)
	int32 ActiveDropStageWaveNumber = 0;

	FTimerHandle SpawnTimerHandle;
};
