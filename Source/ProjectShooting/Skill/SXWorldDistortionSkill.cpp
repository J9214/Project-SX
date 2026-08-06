// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXWorldDistortionSkill.h"

#include "Character/SXCharacterBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Skill/SXSkillData.h"
#include "Skill/SXWorldDistortionAreaActor.h"

bool USXWorldDistortionSkill::Activate()
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

	FVector TargetLocation = GetSkillTargetLocation();
	FRotator TargetRotation = FRotator::ZeroRotator;

	if (SkillData->bProjectWorldDistortionToGround)
	{
		const FVector TraceStart = TargetLocation + FVector(0.0f, 0.0f, SkillData->WorldDistortionGroundTraceUpDistance);
		const FVector TraceEnd = TargetLocation - FVector(0.0f, 0.0f, SkillData->WorldDistortionGroundTraceDownDistance);

		FHitResult GroundHit;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WorldDistortionGroundTrace), false, OwnerCharacter);
		if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			TargetLocation = GroundHit.ImpactPoint + GroundHit.ImpactNormal * 2.0f;
			TargetRotation = FRotationMatrix::MakeFromZ(GroundHit.ImpactNormal).Rotator();
		}
	}

	if (SkillData->VFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, SkillData->VFX, TargetLocation);
	}

	if (SkillData->SFX)
	{
		UGameplayStatics::PlaySoundAtLocation(World, SkillData->SFX, TargetLocation);
	}

	TSubclassOf<ASXWorldDistortionAreaActor> AreaActorClass = SkillData->WorldDistortionAreaActorClass;
	if (AreaActorClass == nullptr)
	{
		AreaActorClass = ASXWorldDistortionAreaActor::StaticClass();
	}

	const FTransform SpawnTransform(TargetRotation, TargetLocation);
	ASXWorldDistortionAreaActor* AreaActor = World->SpawnActorDeferred<ASXWorldDistortionAreaActor>(
		AreaActorClass,
		SpawnTransform,
		OwnerCharacter,
		OwnerCharacter,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (IsValid(AreaActor) == false)
	{
		EndSkill();
		return false;
	}

	AreaActor->InitializeWorldDistortionArea(
		OwnerCharacter,
		SkillData->Radius,
		SkillData->SlowMultiplier,
		SkillData->Duration,
		SkillData->WorldDistortionParameterCollection,
		SkillData->WorldDistortionPositionParameterName
	);

	UGameplayStatics::FinishSpawningActor(AreaActor, SpawnTransform);
	UE_LOG(LogTemp, Log, TEXT("WorldDistortion area spawned at %s, radius %.1f, duration %.1f."), *TargetLocation.ToString(), SkillData->Radius, SkillData->Duration);

	return true;
}

FVector USXWorldDistortionSkill::GetSkillTargetLocation() const
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
