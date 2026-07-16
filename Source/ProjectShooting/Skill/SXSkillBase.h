// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SXSkillBase.generated.h"

class ASXCharacterBase;
class USXSkillComponent;
class USXSkillData;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PROJECTSHOOTING_API USXSkillBase : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category="SX|Skill")
	virtual void InitializeSkill(USXSkillComponent* InOwningSkillComponent, USXSkillData* InSkillData);

	UFUNCTION(BlueprintCallable, Category="SX|Skill")
	virtual bool CanActivate() const;

	UFUNCTION(BlueprintCallable, Category="SX|Skill")
	virtual bool Activate();

	UFUNCTION(BlueprintCallable, Category="SX|Skill")
	virtual void EndSkill();

	UFUNCTION(BlueprintPure, Category="SX|Skill")
	USXSkillComponent* GetOwningSkillComponent() const { return OwningSkillComponent; }

	UFUNCTION(BlueprintPure, Category="SX|Skill")
	USXSkillData* GetSkillData() const { return SkillData; }

	UFUNCTION(BlueprintPure, Category="SX|Skill")
	AActor* GetOwnerActor() const;

	UFUNCTION(BlueprintPure, Category="SX|Skill")
	ASXCharacterBase* GetOwnerCharacter() const;

	UFUNCTION(BlueprintPure, Category="SX|Skill")
	bool IsActive() const { return bIsActive; }

	UFUNCTION(BlueprintNativeEvent, Category="SX|Skill")
	void BP_OnActivated();

	UFUNCTION(BlueprintNativeEvent, Category="SX|Skill")
	void BP_OnEnded();

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill")
	TObjectPtr<USXSkillComponent> OwningSkillComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill")
	TObjectPtr<USXSkillData> SkillData;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Skill")
	bool bIsActive = false;
};
