// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/SXNPCAnimInstance.h"

#include "Character/SXEnemyCharacterBase.h"

void USXNPCAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CacheEnemyReferences();
}

void USXNPCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(EnemyCharacter))
	{
		CacheEnemyReferences();
	}

	bIsDead = IsValid(EnemyCharacter) && !EnemyCharacter->IsAlive();
}

void USXNPCAnimInstance::CacheEnemyReferences()
{
	EnemyCharacter = Cast<ASXEnemyCharacterBase>(Character);
}
