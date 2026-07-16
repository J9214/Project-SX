// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SXStatusComponent.generated.h"

class USXStatusComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FSXOnStatusHealthChangedSignature, USXStatusComponent*, StatusComponent, float, OldHealth, float, NewHealth, float, Delta, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSXOnStatusDeathSignature, USXStatusComponent*, StatusComponent, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSXOnStatusGoldChangedSignature, USXStatusComponent*, StatusComponent, int32, OldGold, int32, NewGold, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSXOnStatusExperienceChangedSignature, USXStatusComponent*, StatusComponent, int32, OldExperience, int32, NewExperience, int32, Delta);

UCLASS(ClassGroup=(SX), meta=(BlueprintSpawnableComponent))
class PROJECTSHOOTING_API USXStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USXStatusComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="SX|Status|Health")
	float ApplyDamage(float DamageAmount, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="SX|Status|Health")
	float Heal(float HealAmount, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="SX|Status|Health")
	void ResetHealth();

	UFUNCTION(BlueprintCallable, Category="SX|Status|Health")
	void SetMaxHealth(float NewMaxHealth, bool bResetCurrentHealth = true);

	UFUNCTION(BlueprintPure, Category="SX|Status|Health")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category="SX|Status|Health")
	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category="SX|Status|Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="SX|Status|Health")
	float GetHealthRatio() const;

	UFUNCTION(BlueprintPure, Category="SX|Status|Movement")
	float GetWalkSpeed() const { return WalkSpeed; }

	UFUNCTION(BlueprintPure, Category="SX|Status|Movement")
	float GetSprintSpeed() const { return SprintSpeed; }

	UFUNCTION(BlueprintPure, Category="SX|Status|Weapon")
	float GetFirePerMinute() const { return FirePerMinute; }

	UFUNCTION(BlueprintPure, Category="SX|Status|Weapon")
	float GetTimeBetweenFire() const;

	UFUNCTION(BlueprintCallable, Category="SX|Status|Reward")
	int32 AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category="SX|Status|Reward")
	bool SpendGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category="SX|Status|Reward")
	int32 AddExperience(int32 Amount);

	UFUNCTION(BlueprintPure, Category="SX|Status|Reward")
	int32 GetGold() const { return Gold; }

	UFUNCTION(BlueprintPure, Category="SX|Status|Reward")
	int32 GetExperience() const { return Experience; }

	UPROPERTY(BlueprintAssignable, Category="SX|Status|Health")
	FSXOnStatusHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|Status|Health")
	FSXOnStatusDeathSignature OnDeath;

	UPROPERTY(BlueprintAssignable, Category="SX|Status|Reward")
	FSXOnStatusGoldChangedSignature OnGoldChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|Status|Reward")
	FSXOnStatusExperienceChangedSignature OnExperienceChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Status|Health", meta=(ClampMin="1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Status|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Status|Health")
	bool bIsDead = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Status|Movement")
	float WalkSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Status|Movement")
	float SprintSpeed = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Status|Weapon", meta=(ClampMin="1.0"))
	float FirePerMinute = 600.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Status|Reward")
	int32 Gold = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Status|Reward")
	int32 Experience = 0;
};
