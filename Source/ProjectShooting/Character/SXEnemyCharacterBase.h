// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SXCharacterBase.h"
#include "SXEnemyCharacterBase.generated.h"

class ASXEnemyCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXOnEnemyDeathSignature, ASXEnemyCharacterBase*, DeadEnemy);

UCLASS()
class PROJECTSHOOTING_API ASXEnemyCharacterBase : public ASXCharacterBase
{
	GENERATED_BODY()

public:
	ASXEnemyCharacterBase();

	UFUNCTION(BlueprintCallable, Category="SX|Enemy|Combat")
	bool TryAttack(AActor* TargetActor);

	UFUNCTION(BlueprintPure, Category="SX|Enemy|Combat")
	float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure, Category="SX|Enemy|Combat")
	float GetAttackDamage() const { return AttackDamage; }

	UFUNCTION(BlueprintPure, Category="SX|Enemy|Combat")
	float GetAttackInterval() const { return AttackInterval; }

	virtual void Die(AActor* DamageCauser) override;

	UPROPERTY(BlueprintAssignable, Category="SX|Enemy")
	FSXOnEnemyDeathSignature OnEnemyDeath;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Combat", meta=(ClampMin="0.0"))
	float AttackDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Combat", meta=(ClampMin="0.0", Units=cm))
	float AttackRange = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Combat", meta=(ClampMin="0.01", Units=s))
	float AttackInterval = 1.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Combat")
	float LastAttackTime = -10000.0f;
	
};
