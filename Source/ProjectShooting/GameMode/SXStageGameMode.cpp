// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode/SXStageGameMode.h"

#include "Character/SXPlayerCharacter.h"
#include "Controller/SXGamePlayerController.h"

ASXStageGameMode::ASXStageGameMode()
{
	DefaultPawnClass = ASXPlayerCharacter::StaticClass();
	PlayerControllerClass = ASXGamePlayerController::StaticClass();
}
