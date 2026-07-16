// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/SXStageWaveDataAsset.h"

bool USXStageWaveDataAsset::GetWaveData(int32 WaveIndex, FSXStageWaveData& OutWaveData) const
{
	if (StageWaves.IsValidIndex(WaveIndex) == false)
	{
		return false;
	}

	OutWaveData = StageWaves[WaveIndex];
	return true;
}
