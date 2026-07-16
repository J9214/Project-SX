// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXUtilitySkill.h"

#include "Character/SXCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Skill/SXSkillData.h"

bool USXUtilitySkill::Activate()
{
	if (Super::Activate() == false)
	{
		return false;
	}

	ASXCharacterBase* OwnerCharacter = GetOwnerCharacter();
	if (IsValid(OwnerCharacter) == false || IsValid(SkillData.Get()) == false)
	{
		EndSkill();
		return false;
	}

	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (IsValid(MovementComponent) == false)
	{
		EndSkill();
		return false;
	}

	OriginalMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
	OriginalJumpZVelocity = MovementComponent->JumpZVelocity;

	MovementComponent->MaxWalkSpeed = OriginalMaxWalkSpeed * SkillData->SpeedMultiplier;
	MovementComponent->JumpZVelocity = OriginalJumpZVelocity * SkillData->JumpMultiplier;

	return true;
}

void USXUtilitySkill::EndSkill()
{
	ASXCharacterBase* OwnerCharacter = GetOwnerCharacter();
	if (IsValid(OwnerCharacter) == true)
	{
		UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
		if (IsValid(MovementComponent) == true)
		{
			if (OriginalMaxWalkSpeed > 0.0f)
			{
				MovementComponent->MaxWalkSpeed = OriginalMaxWalkSpeed;
			}

			if (OriginalJumpZVelocity > 0.0f)
			{
				MovementComponent->JumpZVelocity = OriginalJumpZVelocity;
			}
		}
	}

	Super::EndSkill();
}
