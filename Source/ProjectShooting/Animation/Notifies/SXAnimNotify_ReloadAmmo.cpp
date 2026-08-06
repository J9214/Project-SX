// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/Notifies/SXAnimNotify_ReloadAmmo.h"

#include "Character/SXPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Item/SXWeapon.h"

void USXAnimNotify_ReloadAmmo::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ASXPlayerCharacter* PlayerCharacter = IsValid(MeshComp)
		? Cast<ASXPlayerCharacter>(MeshComp->GetOwner())
		: nullptr;

	ASXWeapon* Weapon = IsValid(PlayerCharacter)
		? PlayerCharacter->GetCurrentWeapon()
		: nullptr;

	if (IsValid(Weapon))
	{
		Weapon->CompleteReload();
	}
}
