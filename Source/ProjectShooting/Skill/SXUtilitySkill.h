// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/SXSkillBase.h"
#include "SXUtilitySkill.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PROJECTSHOOTING_API USXUtilitySkill : public USXSkillBase
{
	GENERATED_BODY()

public:
	virtual bool Activate() override;

	virtual void EndSkill() override;

protected:
	float OriginalMaxWalkSpeed = 0.0f;

	float OriginalJumpZVelocity = 0.0f;
};
