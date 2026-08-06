// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SXSkillTypes.generated.h"

UENUM(BlueprintType)
enum class ESXSkillType : uint8
{
	Attack,
	Defense,
	Utility,
	Movement,
	WorldDistortion
};

UENUM(BlueprintType)
enum class ESXSkillSlot : uint8
{
	Movement,
	Skill1,
	Skill2
};
