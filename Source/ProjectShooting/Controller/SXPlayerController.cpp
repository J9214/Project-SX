// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/SXPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"

ASXPlayerController::ASXPlayerController()
{
	bShowMouseCursor = false;
}

void ASXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ApplyDefaultInputMapping();
	SetGameOnlyInputMode();
	CreateGameHUD();
}

void ASXPlayerController::CreateGameHUD()
{
	if (IsValid(GameHUDWidgetClass) == true && IsValid(GameHUDWidgetInstance) == false)
	{
		GameHUDWidgetInstance = CreateWidget<UUserWidget>(this, GameHUDWidgetClass);
		if (IsValid(GameHUDWidgetInstance) == true)
		{
			GameHUDWidgetInstance->AddToViewport(0);
			GameHUDWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		}
	}

	if (IsValid(CrosshairWidgetClass) == true && IsValid(CrosshairWidgetInstance) == false)
	{
		CrosshairWidgetInstance = CreateWidget<UUserWidget>(this, CrosshairWidgetClass);
		if (IsValid(CrosshairWidgetInstance) == true)
		{
			CrosshairWidgetInstance->AddToViewport(1);
			CrosshairWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void ASXPlayerController::AddInputMappingContext(UInputMappingContext* MappingContext, int32 Priority)
{
	if (!MappingContext)
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			InputSubsystem->AddMappingContext(MappingContext, Priority);
		}
	}
}

void ASXPlayerController::RemoveInputMappingContext(UInputMappingContext* MappingContext)
{
	if (!MappingContext)
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			InputSubsystem->RemoveMappingContext(MappingContext);
		}
	}
}

void ASXPlayerController::SetGameplayInputEnabled(bool bEnabled)
{
	SetIgnoreMoveInput(!bEnabled);
	SetIgnoreLookInput(!bEnabled);

	if (bEnabled)
	{
		SetGameOnlyInputMode();
	}
}

void ASXPlayerController::ApplyDefaultInputMapping()
{
	for (const FSXInputMappingContextEntry& MappingEntry : DefaultGameplayMappingContexts)
	{
		AddInputMappingContext(MappingEntry.MappingContext, MappingEntry.Priority);
	}
}

void ASXPlayerController::SetGameOnlyInputMode()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}
