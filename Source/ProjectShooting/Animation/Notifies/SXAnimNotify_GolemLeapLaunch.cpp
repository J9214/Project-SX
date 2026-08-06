// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/Notifies/SXAnimNotify_GolemLeapLaunch.h"

#include "Character/SXEnemyGolem.h"
#include "Components/SkeletalMeshComponent.h"

void USXAnimNotify_GolemLeapLaunch::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ASXEnemyGolem* Golem = IsValid(MeshComp) ? Cast<ASXEnemyGolem>(MeshComp->GetOwner()) : nullptr;
	if (IsValid(Golem))
	{
		Golem->HandleLeapLaunchNotify();
	}
}
