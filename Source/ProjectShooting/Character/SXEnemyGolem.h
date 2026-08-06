// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SXEnemyCharacterBase.h"
#include "SXEnemyGolem.generated.h"

class ASXEnemyAIController;
class UAnimMontage;
class UCameraShakeBase;
class UNiagaraSystem;
class USoundBase;

UENUM(BlueprintType)
enum class ESXGolemLeapState : uint8
{
	None,
	Windup,
	Airborne,
	Recovery
};

/**
 * Heavy enemy that approaches the player, then performs a telegraphed leap
 * toward a locked landing point and deals area damage when it lands.
 */
UCLASS()
class PROJECTSHOOTING_API ASXEnemyGolem : public ASXEnemyCharacterBase
{
	GENERATED_BODY()

public:
	ASXEnemyGolem();

	virtual void UpdateAIBehavior(ASXEnemyAIController* AIController, APawn* TargetPawn) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void Die(AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category="SX|Enemy|Golem")
	bool StartLeapAttack(APawn* TargetPawn);

	UFUNCTION(BlueprintCallable, Category="SX|Enemy|Golem")
	void HandleLeapLaunchNotify();

	UFUNCTION(BlueprintCallable, Category="SX|Enemy|Golem")
	void FinishLeapAttack();

	UFUNCTION(BlueprintPure, Category="SX|Enemy|Golem")
	bool IsLeapAttacking() const { return LeapState != ESXGolemLeapState::None; }

	UFUNCTION(BlueprintPure, Category="SX|Enemy|Golem")
	ESXGolemLeapState GetLeapState() const { return LeapState; }

protected:
	bool CanStartLeapAttack(const APawn* TargetPawn) const;
	FVector ResolveLeapTargetLocation(const APawn* TargetPawn);
	void ApplySlamImpact();
	void SpawnLandingWarning();
	void ClearLandingWarning();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Leap", meta=(ClampMin="0.0", Units=cm))
	float LeapMinRange = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Leap", meta=(ClampMin="0.0", Units=cm))
	float LeapMaxRange = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Leap", meta=(ClampMin="0.1", Units=s))
	float LeapCooldown = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Leap", meta=(ClampMin="0.1", Units=s))
	float LeapFlightTime = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Leap", meta=(ClampMin="0.0", Units=s))
	float TargetPredictionTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Leap", meta=(ClampMin="0.0", Units=cm))
	float LandingOffset = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Leap", meta=(ClampMin="0.0", Units=s))
	float LeapLaunchFallbackDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Leap", meta=(ClampMin="0.0", Units=s))
	float RecoveryDuration = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Impact", meta=(ClampMin="0.0"))
	float SlamDamage = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Impact", meta=(ClampMin="0.0", Units=cm))
	float SlamRadius = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Impact", meta=(ClampMin="0.0", Units=cm))
	float SlamMaxHeightDifference = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Impact", meta=(ClampMin="0.0", Units="cm/s"))
	float KnockbackHorizontalSpeed = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Impact", meta=(ClampMin="0.0", Units="cm/s"))
	float KnockbackVerticalSpeed = 450.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Animation")
	TObjectPtr<UAnimMontage> LeapWindupMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Animation")
	TObjectPtr<UAnimMontage> LeapLandMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Feedback")
	TSubclassOf<AActor> LandingWarningActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Feedback")
	TObjectPtr<UNiagaraSystem> SlamEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Feedback")
	TObjectPtr<USoundBase> SlamSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Golem|Feedback")
	TSubclassOf<UCameraShakeBase> SlamCameraShake;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Golem")
	ESXGolemLeapState LeapState = ESXGolemLeapState::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Golem")
	FVector LeapTargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Golem")
	TObjectPtr<ASXCharacterBase> LeapTarget;

	UPROPERTY(Transient)
	TObjectPtr<AActor> LandingWarningActor;

	float LastLeapEndTime = -10000.0f;
	FVector LandingWarningLocation = FVector::ZeroVector;

	FTimerHandle LeapLaunchFallbackTimerHandle;
	FTimerHandle LeapRecoveryTimerHandle;
};
