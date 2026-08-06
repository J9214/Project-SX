// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SXAnimNotify_GolemLeapLaunch.generated.h"

/**
 * Launches an SX golem toward the landing point captured when its leap attack
 * started. Place this notify on the takeoff frame of the windup montage.
 */
UCLASS(meta=(DisplayName="SX Golem Leap Launch"))
class PROJECTSHOOTING_API USXAnimNotify_GolemLeapLaunch : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
};
