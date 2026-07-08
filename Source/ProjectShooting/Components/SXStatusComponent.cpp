// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SXStatusComponent.h"

#include "Math/UnrealMathUtility.h"

USXStatusComponent::USXStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USXStatusComponent::BeginPlay()
{
	Super::BeginPlay();

	ResetHealth();
}

float USXStatusComponent::ApplyDamage(float DamageAmount, AActor* InstigatorActor)
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

float USXStatusComponent::Heal(float HealAmount, AActor* InstigatorActor)
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

void USXStatusComponent::ResetHealth()
{
	bIsDead = false;
	CurrentHealth = MaxHealth;
}

bool USXStatusComponent::IsAlive() const
{
	return !bIsDead && CurrentHealth > 0.0f;
}

float USXStatusComponent::GetHealthRatio() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

float USXStatusComponent::GetTimeBetweenFire() const
{
	return FirePerMinute > 0.0f ? 60.0f / FirePerMinute : 0.0f;
}

