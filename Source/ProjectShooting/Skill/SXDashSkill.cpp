// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXDashSkill.h"

#include "Character/SXCharacterBase.h"
#include "GameFramework/Controller.h"
#include "Skill/SXSkillData.h"

bool USXDashSkill::Activate()
{
	if (Super::Activate() == false)
	{
		return false;
	}

	ASXCharacterBase* OwnerCharacter = GetOwnerCharacter();
	if (IsValid(OwnerCharacter) == false)
	{
		EndSkill();
		return false;
	}

	FVector DashDirection = OwnerCharacter->GetLastMovementInputVector();
	DashDirection.Z = 0.0f;

	if (DashDirection.IsNearlyZero() == true)
	{
		const FRotator ControlRotation = IsValid(OwnerCharacter->GetController()) == true ? OwnerCharacter->GetController()->GetControlRotation() : OwnerCharacter->GetActorRotation();
		DashDirection = FRotationMatrix(FRotator(0.0f, ControlRotation.Yaw, 0.0f)).GetUnitAxis(EAxis::X);
	}

	DashDirection.Normalize();

	const float DashStrength = IsValid(SkillData.Get()) == true ? SkillData->DashStrength : 1500.0f;

	OwnerCharacter->LaunchCharacter(DashDirection * DashStrength, true, false);
	EndSkill();

	return true;
}
