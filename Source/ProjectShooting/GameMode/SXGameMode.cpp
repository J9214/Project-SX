// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/SXGameMode.h"

#include "Character/SXPlayerCharacter.h"
#include "Controller/SXPlayerController.h"

ASXGameMode::ASXGameMode()
{
	DefaultPawnClass = ASXPlayerCharacter::StaticClass();
	PlayerControllerClass = ASXPlayerController::StaticClass();
}
