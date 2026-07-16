// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/SXSkillBase.h"
#include "SXDefenseSkill.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PROJECTSHOOTING_API USXDefenseSkill : public USXSkillBase
{
	GENERATED_BODY()

public:
	virtual bool Activate() override;
};
