// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Skill/SXSkillTypes.h"
#include "SXSkillData.generated.h"

class ASXSkillAreaActor;
class ASXSkillBarrierActor;
class UAnimMontage;
class UParticleSystem;
class USoundBase;
class USXSkillBase;

UCLASS(BlueprintType)
class PROJECTSHOOTING_API USXSkillData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill")
	FName SkillName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill")
	ESXSkillType SkillType = ESXSkillType::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill")
	TSubclassOf<USXSkillBase> SkillClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill", meta=(ClampMin="0.0"))
	float Cooldown = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill", meta=(ClampMin="0.0"))
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Anim")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Effect")
	TObjectPtr<UParticleSystem> VFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Effect")
	TObjectPtr<USoundBase> SFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Attack", meta=(ClampMin="0.0"))
	float Damage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Attack", meta=(ClampMin="0.0"))
	float Radius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Attack", meta=(ClampMin="0.0"))
	float TargetDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Attack")
	TSubclassOf<ASXSkillAreaActor> AreaActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Attack", meta=(ClampMin="0.01"))
	float AreaTickInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Defense")
	TSubclassOf<ASXSkillBarrierActor> BarrierActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Defense", meta=(ClampMin="0.0"))
	float ShieldAmount = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Movement", meta=(ClampMin="0.0"))
	float DashStrength = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Utility", meta=(ClampMin="0.0"))
	float SpeedMultiplier = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Utility", meta=(ClampMin="0.0"))
	float JumpMultiplier = 1.0f;
};
