// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXSkillBase.h"

#include "Character/SXCharacterBase.h"
#include "Skill/SXSkillComponent.h"
#include "Skill/SXSkillData.h"

UWorld* USXSkillBase::GetWorld() const
{
	if (IsValid(OwningSkillComponent.Get()) == true)
	{
		return OwningSkillComponent->GetWorld();
	}

	return nullptr;
}

void USXSkillBase::InitializeSkill(USXSkillComponent* InOwningSkillComponent, USXSkillData* InSkillData)
{
	OwningSkillComponent = InOwningSkillComponent;
	SkillData = InSkillData;
	bIsActive = false;
}

bool USXSkillBase::CanActivate() const
{
	return IsValid(OwningSkillComponent.Get()) == true && IsValid(SkillData.Get()) == true && IsValid(GetOwnerActor()) == true;
}

bool USXSkillBase::Activate()
{
	if (CanActivate() == false)
	{
		return false;
	}

	bIsActive = true;
	BP_OnActivated();

	return true;
}

void USXSkillBase::EndSkill()
{
	if (bIsActive == false)
	{
		return;
	}

	bIsActive = false;
	BP_OnEnded();
}

AActor* USXSkillBase::GetOwnerActor() const
{
	return IsValid(OwningSkillComponent.Get()) == true ? OwningSkillComponent->GetOwner() : nullptr;
}

ASXCharacterBase* USXSkillBase::GetOwnerCharacter() const
{
	return Cast<ASXCharacterBase>(GetOwnerActor());
}

void USXSkillBase::BP_OnActivated_Implementation()
{
}

void USXSkillBase::BP_OnEnded_Implementation()
{
}
