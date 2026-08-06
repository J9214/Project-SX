// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXDefenseSkill.h"

#include "Character/SXCharacterBase.h"
#include "Kismet/GameplayStatics.h"
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

	const FRotator SpawnRotation = FRotator::ZeroRotator;
	const FVector SpawnLocation = OwnerCharacter->GetActorLocation();
	TSubclassOf<ASXSkillBarrierActor> BarrierActorClass = SkillData->BarrierActorClass;
	if (BarrierActorClass == nullptr)
	{
		BarrierActorClass = ASXSkillBarrierActor::StaticClass();
	}

	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);
	ASXSkillBarrierActor* BarrierActor = World->SpawnActorDeferred<ASXSkillBarrierActor>(
		BarrierActorClass,
		SpawnTransform,
		OwnerCharacter,
		OwnerCharacter,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (IsValid(BarrierActor) == false)
	{
		EndSkill();
		return false;
	}

	BarrierActor->InitializeBarrier(
		OwnerCharacter,
		SkillData->Duration,
		SkillData->BarrierRadius,
		SkillData->BarrierExpansionDuration,
		SkillData->BarrierPushStrength,
		SkillData->BarrierPushTickInterval
	);

	UGameplayStatics::FinishSpawningActor(BarrierActor, SpawnTransform);
	UE_LOG(LogTemp, Log, TEXT("Barrier spawned at %s, radius %.1f, duration %.1f."), *SpawnLocation.ToString(), SkillData->BarrierRadius, SkillData->Duration);

	EndSkill();
	return true;
}
