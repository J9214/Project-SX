// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/SXDropDatabase.h"

bool USXDropDatabase::BuildDropData(
	TSubclassOf<ASXEnemyCharacterBase> EnemyClass,
	FName StageId,
	int32 WaveNumber,
	const TArray<FSXDropItemData>& EnemyDefaultDropItems,
	const FSXDropModifier& RuntimeDropModifier,
	TArray<FSXDropItemData>& OutDropItems,
	FSXDropModifier& OutDropModifier) const
{
	OutDropItems.Reset();
	OutDropModifier = FSXDropModifier();

	const FSXMonsterDropRule* MonsterRule = FindMonsterDropRule(EnemyClass);
	if (MonsterRule != nullptr)
	{
		if (MonsterRule->bAppendEnemyDefaultDropList)
		{
			OutDropItems.Append(EnemyDefaultDropItems);
		}

		OutDropItems.Append(MonsterRule->DropItems);
		MultiplyDropModifier(OutDropModifier, MonsterRule->DropModifier);
	}
	else if (bUseEnemyDefaultDropListWhenNoMonsterRule)
	{
		OutDropItems.Append(EnemyDefaultDropItems);
	}

	for (const FSXWaveDropRule& WaveRule : WaveDropRules)
	{
		if (DoesWaveRuleMatch(WaveRule, StageId, WaveNumber) == false)
		{
			continue;
		}

		OutDropItems.Append(WaveRule.AdditionalDropItems);
		MultiplyDropModifier(OutDropModifier, WaveRule.DropModifier);
	}

	MultiplyDropModifier(OutDropModifier, RuntimeDropModifier);
	return OutDropItems.Num() > 0;
}

const FSXMonsterDropRule* USXDropDatabase::FindMonsterDropRule(TSubclassOf<ASXEnemyCharacterBase> EnemyClass) const
{
	if (EnemyClass == nullptr)
	{
		return nullptr;
	}

	const FSXMonsterDropRule* BestRule = nullptr;
	for (const FSXMonsterDropRule& MonsterRule : MonsterDropRules)
	{
		if (MonsterRule.EnemyClass == nullptr)
		{
			continue;
		}

		if (EnemyClass == MonsterRule.EnemyClass)
		{
			return &MonsterRule;
		}

		if (EnemyClass->IsChildOf(MonsterRule.EnemyClass))
		{
			BestRule = &MonsterRule;
		}
	}

	return BestRule;
}

bool USXDropDatabase::DoesWaveRuleMatch(const FSXWaveDropRule& WaveRule, FName StageId, int32 WaveNumber) const
{
	const bool bMatchesStage = WaveRule.StageId.IsNone() || WaveRule.StageId == StageId;
	const bool bMatchesWave = WaveRule.WaveNumber <= 0 || WaveRule.WaveNumber == WaveNumber;
	return bMatchesStage && bMatchesWave;
}

void USXDropDatabase::MultiplyDropModifier(FSXDropModifier& Target, const FSXDropModifier& Source)
{
	Target.GlobalDropChanceMultiplier *= Source.GlobalDropChanceMultiplier;
	Target.GoldDropChanceMultiplier *= Source.GoldDropChanceMultiplier;
	Target.GoldAmountMultiplier *= Source.GoldAmountMultiplier;
	Target.AmmoDropChanceMultiplier *= Source.AmmoDropChanceMultiplier;
	Target.AmmoAmountMultiplier *= Source.AmmoAmountMultiplier;
	Target.ExperienceDropChanceMultiplier *= Source.ExperienceDropChanceMultiplier;
	Target.ExperienceAmountMultiplier *= Source.ExperienceAmountMultiplier;
	Target.OtherDropChanceMultiplier *= Source.OtherDropChanceMultiplier;
}
