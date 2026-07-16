// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SXEnemyCharacterBase.h"
#include "SXEnemyRanged.generated.h"

class ASXEnemyAIController;
class ASXEnemyProjectile;

UCLASS()
class PROJECTSHOOTING_API ASXEnemyRanged : public ASXEnemyCharacterBase
{
	GENERATED_BODY()

public:
	ASXEnemyRanged();

	virtual void UpdateAIBehavior(ASXEnemyAIController* AIController, APawn* TargetPawn) override;

protected:
	bool TryRangedAttack(APawn* TargetPawn);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Ranged")
	TSubclassOf<ASXEnemyProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Ranged", meta=(ClampMin="0.0", Units=cm))
	float PreferredRange = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Ranged", meta=(ClampMin="0.0", Units=cm))
	float RetreatRange = 450.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Ranged", meta=(ClampMin="0.0", Units=cm))
	float RangedAttackRange = 1400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Ranged", meta=(ClampMin="0.0"))
	float ProjectileDamage = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Ranged")
	FVector ProjectileSpawnOffset = FVector(80.0f, 0.0f, 70.0f);
};
