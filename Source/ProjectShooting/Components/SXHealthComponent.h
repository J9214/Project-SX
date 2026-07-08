// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SXHealthComponent.generated.h"

class USXHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FSXOnHealthChangedSignature, USXHealthComponent*, HealthComponent, float, OldHealth, float, NewHealth, float, Delta, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSXOnDeathSignature, USXHealthComponent*, HealthComponent, AActor*, InstigatorActor);

UCLASS(ClassGroup=(SX), meta=(BlueprintSpawnableComponent))
class PROJECTSHOOTING_API USXHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USXHealthComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="SX|Health")
	float ApplyDamage(float DamageAmount, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="SX|Health")
	float Heal(float HealAmount, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="SX|Health")
	void ResetHealth();

	UFUNCTION(BlueprintPure, Category="SX|Health")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category="SX|Health")
	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category="SX|Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="SX|Health")
	float GetHealthRatio() const;

	UPROPERTY(BlueprintAssignable, Category="SX|Health")
	FSXOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|Health")
	FSXOnDeathSignature OnDeath;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Health", meta=(ClampMin="1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Health")
	float CurrentHealth = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Health")
	bool bIsDead = false;
};
