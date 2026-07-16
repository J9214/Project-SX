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

void USXStatusComponent::SetMaxHealth(float NewMaxHealth, bool bResetCurrentHealth)
{
	MaxHealth = FMath::Max(1.0f, NewMaxHealth);

	if (bResetCurrentHealth)
	{
		ResetHealth();
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
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

int32 USXStatusComponent::AddGold(int32 Amount)
{
	if (Amount <= 0)
	{
		return 0;
	}

	const int32 OldGold = Gold;
	Gold += Amount;
	const int32 AddedGold = Gold - OldGold;

	OnGoldChanged.Broadcast(this, OldGold, Gold, AddedGold);
	return AddedGold;
}

bool USXStatusComponent::SpendGold(int32 Amount)
{
	if (Amount <= 0 || Gold < Amount)
	{
		return false;
	}

	const int32 OldGold = Gold;
	Gold -= Amount;

	OnGoldChanged.Broadcast(this, OldGold, Gold, -Amount);
	return true;
}

int32 USXStatusComponent::AddExperience(int32 Amount)
{
	if (Amount <= 0)
	{
		return 0;
	}

	const int32 OldExperience = Experience;
	Experience += Amount;
	const int32 AddedExperience = Experience - OldExperience;

	OnExperienceChanged.Broadcast(this, OldExperience, Experience, AddedExperience);
	return AddedExperience;
}

