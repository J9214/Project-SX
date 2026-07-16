// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SXEnemyCharacterBase.h"
#include "SXEnemyCharger.generated.h"

class ASXEnemyAIController;

UCLASS()
class PROJECTSHOOTING_API ASXEnemyCharger : public ASXEnemyCharacterBase
{
	GENERATED_BODY()

public:
	ASXEnemyCharger();

	virtual void Tick(float DeltaTime) override;
	virtual void UpdateAIBehavior(ASXEnemyAIController* AIController, APawn* TargetPawn) override;

protected:
	void StartCharge(APawn* TargetPawn);
	void StopCharge();
	void TryApplyChargeDamage(APawn* TargetPawn);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Charge", meta=(ClampMin="0.0", Units=cm))
	float ChargeTriggerRange = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Charge", meta=(ClampMin="0.0", Units="cm/s"))
	float ChargeSpeed = 1400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Charge", meta=(ClampMin="0.01", Units=s))
	float ChargeDuration = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Charge", meta=(ClampMin="0.0", Units=s))
	float ChargeCooldown = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Charge", meta=(ClampMin="0.0"))
	float ChargeDamage = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Charge", meta=(ClampMin="0.0", Units=cm))
	float ChargeHitRadius = 120.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Charge")
	bool bIsCharging = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Charge")
	bool bChargeDamageApplied = false;

	FVector ChargeDirection = FVector::ZeroVector;
	float ChargeStartTime = -10000.0f;
	float LastChargeEndTime = -10000.0f;
	float DefaultWalkSpeed = 450.0f;

	TWeakObjectPtr<APawn> ChargeTarget;
};
