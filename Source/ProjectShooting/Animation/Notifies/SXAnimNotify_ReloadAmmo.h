// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SXAnimNotify_ReloadAmmo.generated.h"

/**
 * Transfers ammunition for the currently equipped weapon at the exact frame
 * selected in a reload montage.
 */
UCLASS(meta=(DisplayName="SX Reload Ammo"))
class PROJECTSHOOTING_API USXAnimNotify_ReloadAmmo : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
};
