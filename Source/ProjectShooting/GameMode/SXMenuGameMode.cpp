// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode/SXMenuGameMode.h"

#include "Controller/SXMenuPlayerController.h"

ASXMenuGameMode::ASXMenuGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ASXMenuPlayerController::StaticClass();
}
