// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXDefenseSkill.h"

#include "Character/SXCharacterBase.h"
#include "Skill/SXSkillBarrierActor.h"
#include "Skill/SXSkillData.h"

bool USXDefenseSkill::Activate()
{
	if (Super::Activate() == false)
	{
		return false;
	}

	ASXCharacterBase* OwnerCharacter = GetOwnerCharacter();
	UWorld* World = GetWorld();
	if (IsValid(OwnerCharacter) == false || IsValid(World) == false || IsValid(SkillData.Get()) == false)
	{
		EndSkill();
		return false;
	}

	const AController* Controller = OwnerCharacter->GetController();
	const FRotator ControlRotation = IsValid(Controller) == true ? Controller->GetControlRotation() : OwnerCharacter->GetActorRotation();
	const FRotator SpawnRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector SpawnDirection = FRotationMatrix(SpawnRotation).GetUnitAxis(EAxis::X);
	const FVector SpawnLocation = OwnerCharacter->GetActorLocation() + SpawnDirection * SkillData->TargetDistance;
	TSubclassOf<ASXSkillBarrierActor> BarrierActorClass = SkillData->BarrierActorClass;
	if (BarrierActorClass == nullptr)
	{
		BarrierActorClass = ASXSkillBarrierActor::StaticClass();
	}

	ASXSkillBarrierActor* BarrierActor = World->SpawnActor<ASXSkillBarrierActor>(BarrierActorClass, SpawnLocation, SpawnRotation);
	if (IsValid(BarrierActor) == true)
	{
		BarrierActor->InitializeBarrier(OwnerCharacter, SkillData->Duration);
	}

	EndSkill();
	return IsValid(BarrierActor) == true;
}
