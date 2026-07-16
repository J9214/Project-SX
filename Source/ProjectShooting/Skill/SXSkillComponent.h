// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Skill/SXSkillTypes.h"
#include "SXSkillComponent.generated.h"

class USXSkillBase;
class USXSkillData;

UCLASS(ClassGroup=(SX), meta=(BlueprintSpawnableComponent))
class PROJECTSHOOTING_API USXSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USXSkillComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="SX|Skill")
	bool ActivateSkill(ESXSkillSlot SkillSlot);

	UFUNCTION(BlueprintCallable, Category="SX|Skill")
	void EndSkill(ESXSkillSlot SkillSlot);

	UFUNCTION(BlueprintPure, Category="SX|Skill")
	bool CanActivateSkill(ESXSkillSlot SkillSlot) const;

	UFUNCTION(BlueprintPure, Category="SX|Skill")
	bool IsSkillOnCooldown(ESXSkillSlot SkillSlot) const;

	UFUNCTION(BlueprintPure, Category="SX|Skill")
	float GetCooldownRemaining(ESXSkillSlot SkillSlot) const;

	UFUNCTION(BlueprintPure, Category="SX|Skill")
	USXSkillBase* GetSkillInstance(ESXSkillSlot SkillSlot) const;

protected:
	void InitializeSkills();

	void InitializeSkill(ESXSkillSlot SkillSlot, USXSkillData* SkillData);

	USXSkillData* GetSkillData(ESXSkillSlot SkillSlot) const;

	void StartCooldown(ESXSkillSlot SkillSlot, float Cooldown);

	void StartDurationTimer(ESXSkillSlot SkillSlot, float Duration);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Equipped Skills", meta=(DisplayName="Movement Skill"))
	TObjectPtr<USXSkillData> MovementSkillData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Equipped Skills", meta=(DisplayName="Skill 1"))
	TObjectPtr<USXSkillData> Skill1Data;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Skill|Equipped Skills", meta=(DisplayName="Skill 2"))
	TObjectPtr<USXSkillData> Skill2Data;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill")
	TMap<ESXSkillSlot, TObjectPtr<USXSkillBase>> SkillInstances;

	TMap<ESXSkillSlot, FTimerHandle> CooldownTimerHandles;

	TMap<ESXSkillSlot, FTimerHandle> DurationTimerHandles;
};
