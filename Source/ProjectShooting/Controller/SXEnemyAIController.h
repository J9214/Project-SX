// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SXEnemyAIController.generated.h"

class ASXEnemyCharacterBase;

UCLASS()
class PROJECTSHOOTING_API ASXEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASXEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	void UpdateAI();
	APawn* GetTargetPlayerPawn() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|AI", meta=(ClampMin="0.05", Units=s))
	float AIUpdateInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|AI", meta=(ClampMin="0.0", Units=cm))
	float AcceptanceRadiusPadding = 20.0f;

	FTimerHandle AIUpdateTimerHandle;
};
