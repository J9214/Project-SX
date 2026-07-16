// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controller/SXPlayerController.h"
#include "SXGamePlayerController.generated.h"

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXGamePlayerController : public ASXPlayerController
{
	GENERATED_BODY()

public:
	ASXGamePlayerController();
};
