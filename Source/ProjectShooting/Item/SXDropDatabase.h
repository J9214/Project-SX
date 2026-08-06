// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Character/SXEnemyCharacterBase.h"
#include "SXDropDatabase.generated.h"

USTRUCT(BlueprintType)
struct FSXMonsterDropRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Monster")
	TSubclassOf<ASXEnemyCharacterBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Monster")
	bool bAppendEnemyDefaultDropList = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Monster")
	TArray<FSXDropItemData> DropItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Monster")
	FSXDropModifier DropModifier;
};

USTRUCT(BlueprintType)
struct FSXWaveDropRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Wave", meta=(ToolTip="None means this rule can match every stage."))
	FName StageId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Wave", meta=(ClampMin="0", ToolTip="0 means this rule can match every wave. Otherwise use 1-based wave number."))
	int32 WaveNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Wave")
	TArray<FSXDropItemData> AdditionalDropItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Wave")
	FSXDropModifier DropModifier;
};

UCLASS(BlueprintType)
class PROJECTSHOOTING_API USXDropDatabase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="SX|Drop")
	bool BuildDropData(
		TSubclassOf<ASXEnemyCharacterBase> EnemyClass,
		FName StageId,
		int32 WaveNumber,
		const TArray<FSXDropItemData>& EnemyDefaultDropItems,
		const FSXDropModifier& RuntimeDropModifier,
		TArray<FSXDropItemData>& OutDropItems,
		FSXDropModifier& OutDropModifier) const;

protected:
	const FSXMonsterDropRule* FindMonsterDropRule(TSubclassOf<ASXEnemyCharacterBase> EnemyClass) const;
	bool DoesWaveRuleMatch(const FSXWaveDropRule& WaveRule, FName StageId, int32 WaveNumber) const;
	static void MultiplyDropModifier(FSXDropModifier& Target, const FSXDropModifier& Source);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Monster")
	TArray<FSXMonsterDropRule> MonsterDropRules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Wave")
	TArray<FSXWaveDropRule> WaveDropRules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Fallback")
	bool bUseEnemyDefaultDropListWhenNoMonsterRule = true;
};
