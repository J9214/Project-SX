// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SXHealthComponent.h"

#include "Math/UnrealMathUtility.h"

USXHealthComponent::USXHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USXHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	ResetHealth();
}

float USXHealthComponent::ApplyDamage(float DamageAmount, AActor* InstigatorActor)
{
	if (DamageAmount <= 0.0f || bIsDead)
	{
		return 0.0f;
	}

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	const float AppliedDamage = OldHealth - CurrentHealth;

	if (AppliedDamage > 0.0f)
	{
		OnHealthChanged.Broadcast(this, OldHealth, CurrentHealth, -AppliedDamage, InstigatorActor);
	}

	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		OnDeath.Broadcast(this, InstigatorActor);
	}

	return AppliedDamage;
}

float USXHealthComponent::Heal(float HealAmount, AActor* InstigatorActor)
{
	if (HealAmount <= 0.0f || bIsDead)
	{
		return 0.0f;
	}

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
	const float AppliedHeal = CurrentHealth - OldHealth;

	if (AppliedHeal > 0.0f)
	{
		OnHealthChanged.Broadcast(this, OldHealth, CurrentHealth, AppliedHeal, InstigatorActor);
	}

	return AppliedHeal;
}

void USXHealthComponent::ResetHealth()
{
	bIsDead = false;
	CurrentHealth = MaxHealth;
}

bool USXHealthComponent::IsAlive() const
{
	return !bIsDead && CurrentHealth > 0.0f;
}

float USXHealthComponent::GetHealthRatio() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}
