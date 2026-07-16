// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXSkillComponent.h"

#include "Skill/SXSkillBase.h"
#include "Skill/SXSkillData.h"

USXSkillComponent::USXSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USXSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeSkills();
}

bool USXSkillComponent::ActivateSkill(ESXSkillSlot SkillSlot)
{
	if (CanActivateSkill(SkillSlot) == false)
	{
		return false;
	}

	USXSkillBase* SkillInstance = GetSkillInstance(SkillSlot);
	if (IsValid(SkillInstance) == false || SkillInstance->Activate() == false)
	{
		return false;
	}

	if (USXSkillData* SkillData = GetSkillData(SkillSlot))
	{
		StartCooldown(SkillSlot, SkillData->Cooldown);
		StartDurationTimer(SkillSlot, SkillData->Duration);
	}

	return true;
}

void USXSkillComponent::EndSkill(ESXSkillSlot SkillSlot)
{
	if (USXSkillBase* SkillInstance = GetSkillInstance(SkillSlot))
	{
		SkillInstance->EndSkill();
	}
}

bool USXSkillComponent::CanActivateSkill(ESXSkillSlot SkillSlot) const
{
	const USXSkillBase* SkillInstance = GetSkillInstance(SkillSlot);
	return IsValid(SkillInstance) == true && SkillInstance->CanActivate() == true && IsSkillOnCooldown(SkillSlot) == false && SkillInstance->IsActive() == false;
}

bool USXSkillComponent::IsSkillOnCooldown(ESXSkillSlot SkillSlot) const
{
	const FTimerHandle* CooldownTimerHandle = CooldownTimerHandles.Find(SkillSlot);
	return CooldownTimerHandle != nullptr && GetWorld()->GetTimerManager().IsTimerActive(*CooldownTimerHandle);
}

float USXSkillComponent::GetCooldownRemaining(ESXSkillSlot SkillSlot) const
{
	const FTimerHandle* CooldownTimerHandle = CooldownTimerHandles.Find(SkillSlot);
	if (CooldownTimerHandle == nullptr)
	{
		return 0.0f;
	}

	return GetWorld()->GetTimerManager().GetTimerRemaining(*CooldownTimerHandle);
}

USXSkillBase* USXSkillComponent::GetSkillInstance(ESXSkillSlot SkillSlot) const
{
	const TObjectPtr<USXSkillBase>* SkillInstance = SkillInstances.Find(SkillSlot);
	return SkillInstance != nullptr ? SkillInstance->Get() : nullptr;
}

void USXSkillComponent::InitializeSkills()
{
	SkillInstances.Empty();

	InitializeSkill(ESXSkillSlot::Movement, MovementSkillData);
	InitializeSkill(ESXSkillSlot::Skill1, Skill1Data);
	InitializeSkill(ESXSkillSlot::Skill2, Skill2Data);
}

void USXSkillComponent::InitializeSkill(ESXSkillSlot SkillSlot, USXSkillData* SkillData)
{
	if (IsValid(SkillData) == false || SkillData->SkillClass == nullptr)
	{
		return;
	}

	USXSkillBase* SkillInstance = NewObject<USXSkillBase>(this, SkillData->SkillClass);
	if (IsValid(SkillInstance) == false)
	{
		return;
	}

	SkillInstance->InitializeSkill(this, SkillData);
	SkillInstances.Add(SkillSlot, SkillInstance);
}

USXSkillData* USXSkillComponent::GetSkillData(ESXSkillSlot SkillSlot) const
{
	switch (SkillSlot)
	{
	case ESXSkillSlot::Movement:
		return MovementSkillData;
	case ESXSkillSlot::Skill1:
		return Skill1Data;
	case ESXSkillSlot::Skill2:
		return Skill2Data;
	default:
		return nullptr;
	}
}

void USXSkillComponent::StartCooldown(ESXSkillSlot SkillSlot, float Cooldown)
{
	if (Cooldown <= 0.0f)
	{
		return;
	}

	FTimerHandle& CooldownTimerHandle = CooldownTimerHandles.FindOrAdd(SkillSlot);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, Cooldown, false);
}

void USXSkillComponent::StartDurationTimer(ESXSkillSlot SkillSlot, float Duration)
{
	if (Duration <= 0.0f)
	{
		return;
	}

	FTimerDelegate DurationTimerDelegate;
	DurationTimerDelegate.BindUObject(this, &ThisClass::EndSkill, SkillSlot);

	FTimerHandle& DurationTimerHandle = DurationTimerHandles.FindOrAdd(SkillSlot);
	GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, DurationTimerDelegate, Duration, false);
}
