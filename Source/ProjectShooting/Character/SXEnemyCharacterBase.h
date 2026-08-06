// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SXCharacterBase.h"
#include "Item/SXCollectiblePickup.h"
#include "SXEnemyCharacterBase.generated.h"

class ASXEnemyCharacterBase;
class ASXEnemyAIController;
class ASXCharacterBase;
class USXDropDatabase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXOnEnemyDeathSignature, ASXEnemyCharacterBase*, DeadEnemy);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSXOnEnemyKilledNativeSignature, ASXEnemyCharacterBase* /*DeadEnemy*/, AActor* /*DamageCauser*/);

USTRUCT(BlueprintType)
struct FSXDropModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="0.0"))
	float GlobalDropChanceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="0.0"))
	float GoldDropChanceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="0.0"))
	float GoldAmountMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="0.0"))
	float AmmoDropChanceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="0.0"))
	float AmmoAmountMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="0.0"))
	float ExperienceDropChanceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="0.0"))
	float ExperienceAmountMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="0.0", ToolTip="Applied to non-collectible drop actors such as weapons or health kits."))
	float OtherDropChanceMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FSXDropItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop")
	TSubclassOf<AActor> DropActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DropChance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="1"))
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="1"))
	int32 MaxCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop", meta=(ClampMin="0.0", Units=cm))
	float SpawnRadius = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Collectible")
	bool bOverrideCollectible = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Collectible", meta=(EditCondition="bOverrideCollectible", EditConditionHides))
	ESXCollectibleType CollectibleType = ESXCollectibleType::Gold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Collectible", meta=(ClampMin="1", EditCondition="bOverrideCollectible", EditConditionHides))
	int32 MinAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Collectible", meta=(ClampMin="1", EditCondition="bOverrideCollectible", EditConditionHides))
	int32 MaxAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Drop|Collectible", meta=(EditCondition="bOverrideCollectible && CollectibleType == ESXCollectibleType::Ammo", EditConditionHides))
	ESXAmmoType AmmoType = ESXAmmoType::Normal;
};

UCLASS()
class PROJECTSHOOTING_API ASXEnemyCharacterBase : public ASXCharacterBase
{
	GENERATED_BODY()

public:
	ASXEnemyCharacterBase();

	UFUNCTION(BlueprintCallable, Category="SX|Enemy|Combat")
	bool TryAttack(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category="SX|Enemy|Combat")
	void HandleAttackHitNotify();

	UFUNCTION(BlueprintCallable, Category="SX|Enemy|Combat")
	virtual void FinishAttack();

	UFUNCTION(BlueprintPure, Category="SX|Enemy|Combat")
	bool IsAttacking() const { return bIsAttacking; }

	virtual void UpdateAIBehavior(ASXEnemyAIController* AIController, APawn* TargetPawn);

	UFUNCTION(BlueprintPure, Category="SX|Enemy|Combat")
	float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure, Category="SX|Enemy|Combat")
	float GetAttackDamage() const { return AttackDamage; }

	UFUNCTION(BlueprintPure, Category="SX|Enemy|Combat")
	float GetAttackInterval() const { return AttackInterval; }

	virtual void Die(AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category="SX|Enemy|Drop")
	void SetDropModifier(const FSXDropModifier& InDropModifier);

	UFUNCTION(BlueprintCallable, Category="SX|Enemy|Drop")
	void SetDropDatabaseContext(USXDropDatabase* InDropDatabase, FName InDropStageId, int32 InDropWaveNumber);

	UFUNCTION(BlueprintPure, Category="SX|Enemy|Drop")
	FSXDropModifier GetDropModifier() const { return DropModifier; }

	UPROPERTY(BlueprintAssignable, Category="SX|Enemy")
	FSXOnEnemyDeathSignature OnEnemyDeath;

	static FSXOnEnemyKilledNativeSignature OnEnemyKilledNative;

protected:
	void SpawnDropItems(AActor* DamageCauser);

	FVector GetDropSpawnLocation(const FSXDropItemData& DropItemData) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Combat", meta=(ClampMin="0.0"))
	float AttackDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Combat", meta=(ClampMin="0.0", Units=cm))
	float AttackRange = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Combat", meta=(ClampMin="0.01", Units=s))
	float AttackInterval = 1.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Combat")
	float LastAttackTime = -10000.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Combat")
	bool bIsAttacking = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Combat")
	bool bHasAppliedDamageThisAttack = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Combat")
	TObjectPtr<ASXCharacterBase> CurrentAttackTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Enemy|Drop")
	TArray<FSXDropItemData> DropItemList;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Drop")
	FSXDropModifier DropModifier;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Drop")
	TObjectPtr<USXDropDatabase> DropDatabase;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Drop")
	FName DropStageId = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Drop")
	int32 DropWaveNumber = 0;

	FTimerHandle AttackFinishTimerHandle;
	
};
