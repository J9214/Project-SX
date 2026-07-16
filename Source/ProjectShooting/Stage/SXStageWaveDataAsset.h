// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Stage/SXWaveSpawner.h"
#include "SXStageWaveDataAsset.generated.h"

UCLASS(BlueprintType)
class PROJECTSHOOTING_API USXStageWaveDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="SX|Stage|WaveData")
	int32 GetWaveCount() const { return StageWaves.Num(); }

	UFUNCTION(BlueprintPure, Category="SX|Stage|WaveData")
	bool GetWaveData(int32 WaveIndex, FSXStageWaveData& OutWaveData) const;

	const TArray<FSXStageWaveData>& GetStageWaves() const { return StageWaves; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Stage|WaveData")
	TArray<FSXStageWaveData> StageWaves;
};
