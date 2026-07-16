// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/SXSkillBase.h"
#include "SXAttackSkill.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PROJECTSHOOTING_API USXAttackSkill : public USXSkillBase
{
	GENERATED_BODY()

public:
	virtual bool Activate() override;

protected:
	FVector GetSkillTargetLocation() const;
};
