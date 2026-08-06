// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/SXAnimInstance.h"
#include "SXNPCAnimInstance.generated.h"

class ASXEnemyCharacterBase;

/**
 * Common animation instance for SX enemy NPCs.
 *
 * Enemy archetype-specific animation instances can derive from this class
 * when melee, ranged, or charge-only animation data is required.
 */
UCLASS()
class PROJECTSHOOTING_API USXNPCAnimInstance : public USXAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	void CacheEnemyReferences();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Animation|NPC")
	TObjectPtr<ASXEnemyCharacterBase> EnemyCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Animation|NPC")
	bool bIsDead = false;
};
