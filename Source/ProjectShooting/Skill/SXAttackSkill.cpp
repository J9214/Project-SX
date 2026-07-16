// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXAttackSkill.h"

#include "Character/SXCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Skill/SXSkillAreaActor.h"
#include "Skill/SXSkillData.h"

bool USXAttackSkill::Activate()
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

	const FVector TargetLocation = GetSkillTargetLocation();

	if (SkillData->VFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, SkillData->VFX, TargetLocation);
	}

	if (SkillData->SFX)
	{
		UGameplayStatics::PlaySoundAtLocation(World, SkillData->SFX, TargetLocation);
	}

	if (SkillData->Duration > 0.0f)
	{
		TSubclassOf<ASXSkillAreaActor> AreaActorClass = SkillData->AreaActorClass;
		if (AreaActorClass == nullptr)
		{
			AreaActorClass = ASXSkillAreaActor::StaticClass();
		}

		ASXSkillAreaActor* SkillAreaActor = World->SpawnActor<ASXSkillAreaActor>(AreaActorClass, TargetLocation, FRotator::ZeroRotator);
		if (IsValid(SkillAreaActor) == true)
		{
			SkillAreaActor->InitializeSkillArea(OwnerCharacter, SkillData->Damage, SkillData->Radius, SkillData->AreaTickInterval, SkillData->Duration);
		}
	}
	else
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(OwnerCharacter);

		UGameplayStatics::ApplyRadialDamage(
			World,
			SkillData->Damage,
			TargetLocation,
			SkillData->Radius,
			nullptr,
			IgnoreActors,
			OwnerCharacter,
			OwnerCharacter->GetInstigatorController(),
			true
		);

		EndSkill();
	}

	return true;
}

FVector USXAttackSkill::GetSkillTargetLocation() const
{
	const ASXCharacterBase* OwnerCharacter = GetOwnerCharacter();
	if (IsValid(OwnerCharacter) == false || IsValid(SkillData.Get()) == false)
	{
		return FVector::ZeroVector;
	}

	const AController* Controller = OwnerCharacter->GetController();
	const FRotator ControlRotation = IsValid(Controller) == true ? Controller->GetControlRotation() : OwnerCharacter->GetActorRotation();
	const FVector ForwardDirection = FRotationMatrix(FRotator(0.0f, ControlRotation.Yaw, 0.0f)).GetUnitAxis(EAxis::X);

	return OwnerCharacter->GetActorLocation() + ForwardDirection * SkillData->TargetDistance;
}
