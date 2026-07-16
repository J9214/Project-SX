// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/SXPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Character/SXCharacterBase.h"
#include "Components/SXStatusComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Stage/SXWaveSpawner.h"

ASXPlayerController::ASXPlayerController()
{
	bShowMouseCursor = false;
}

void ASXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ApplyDefaultInputMapping();

	if (ShouldShowMainMenuOnBeginPlay())
	{
		ShowMainMenu();
	}
	else
	{
		SetGameOnlyInputMode();
	}

	if (ShouldCreateGameHUDOnBeginPlay())
	{
		CreateGameHUD();
	}

	BindControlledPawnDeath();
}

void ASXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	BindControlledPawnDeath();
}

void ASXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (IsValid(EnhancedInputComponent) == true && IsValid(PauseMenuAction) == true)
	{
		PauseMenuAction->bTriggerWhenPaused = true;

		FEnhancedInputActionEventBinding& PauseMenuBinding = EnhancedInputComponent->BindAction(
			PauseMenuAction,
			ETriggerEvent::Started,
			this,
			&ThisClass::TogglePauseMenu
		);
		PauseMenuBinding.SetShouldConsume(true);
	}
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

void ASXPlayerController::BindControlledPawnDeath()
{
	ASXCharacterBase* ControlledCharacter = Cast<ASXCharacterBase>(GetPawn());
	if (IsValid(ControlledCharacter) == false)
	{
		return;
	}

	USXStatusComponent* StatusComponent = ControlledCharacter->GetStatusComponent();
	if (IsValid(StatusComponent) == false)
	{
		return;
	}

	StatusComponent->OnDeath.RemoveDynamic(this, &ThisClass::HandleControlledPawnDeath);
	StatusComponent->OnDeath.AddDynamic(this, &ThisClass::HandleControlledPawnDeath);
}

bool ASXPlayerController::ShouldCreateGameHUDOnBeginPlay() const
{
	if (!bCreateGameHUDOnBeginPlay || ShouldShowMainMenuOnBeginPlay())
	{
		return false;
	}

	const FString CurrentMapName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (!MainMenuLevelName.IsNone() && CurrentMapName == MainMenuLevelName.ToString())
	{
		return false;
	}

	if (bRequireWaveSpawnerForGameHUD && IsValid(UGameplayStatics::GetActorOfClass(this, ASXWaveSpawner::StaticClass())) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("Skip GameHUD creation in %s: no SXWaveSpawner found."), *CurrentMapName);
		return false;
	}

	return true;
}

bool ASXPlayerController::ShouldShowMainMenuOnBeginPlay() const
{
	if (bShowMainMenuOnBeginPlay)
	{
		return true;
	}

	const FString CurrentMapName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (!MainMenuLevelName.IsNone() && CurrentMapName == MainMenuLevelName.ToString())
	{
		return true;
	}

	return CurrentMapName.Contains(TEXT("MainMenu"));
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

void ASXPlayerController::ShowMainMenu()
{
	if (IsValid(MainMenuWidgetClass) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SXPlayerController %s cannot show main menu: MainMenuWidgetClass is not set."), *GetNameSafe(this));
		return;
	}

	UUserWidget* MainMenuWidget = CreateMenuWidget(MainMenuWidgetClass, MainMenuWidgetInstance, 10);
	if (IsValid(MainMenuWidget) == false)
	{
		return;
	}

	MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
	SetPause(false);
	SetMenuInputMode(MainMenuWidget);
	SetGameplayInputEnabled(false);
}

void ASXPlayerController::HideMainMenu()
{
	if (IsValid(MainMenuWidgetInstance) == true)
	{
		MainMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ASXPlayerController::StartGame()
{
	if (GameplayLevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("SXPlayerController %s has no GameplayLevelName."), *GetNameSafe(this));
		return;
	}

	UGameplayStatics::OpenLevel(this, GameplayLevelName);
}

void ASXPlayerController::ShowPauseMenu()
{
	if (bPauseMenuOpen)
	{
		return;
	}

	UUserWidget* PauseMenuWidget = CreateMenuWidget(PauseMenuWidgetClass, PauseMenuWidgetInstance, 10);
	if (IsValid(PauseMenuWidget) == false)
	{
		return;
	}

	bPauseMenuOpen = true;
	PauseMenuWidget->SetVisibility(ESlateVisibility::Visible);
	SetPause(true);
	SetMenuInputMode(PauseMenuWidget);
	SetGameplayInputEnabled(false);
}

void ASXPlayerController::HidePauseMenu()
{
	if (IsValid(OptionsMenuWidgetInstance) == true)
	{
		OptionsMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (IsValid(PauseMenuWidgetInstance) == true)
	{
		PauseMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	bPauseMenuOpen = false;
	SetPause(false);
	SetGameplayInputEnabled(true);
}

void ASXPlayerController::TogglePauseMenu()
{
	if (bShowMainMenuOnBeginPlay)
	{
		return;
	}

	if (bPauseMenuOpen)
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}
}

void ASXPlayerController::ShowOptionsMenu()
{
	UUserWidget* OptionsMenuWidget = CreateMenuWidget(OptionsMenuWidgetClass, OptionsMenuWidgetInstance, 20);
	if (IsValid(OptionsMenuWidget) == false)
	{
		return;
	}

	OptionsMenuWidget->SetVisibility(ESlateVisibility::Visible);
	SetMenuInputMode(OptionsMenuWidget);
}

void ASXPlayerController::HideOptionsMenu()
{
	if (IsValid(OptionsMenuWidgetInstance) == true)
	{
		OptionsMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (bPauseMenuOpen && IsValid(PauseMenuWidgetInstance) == true)
	{
		SetMenuInputMode(PauseMenuWidgetInstance);
	}
	else if (IsValid(MainMenuWidgetInstance) == true && MainMenuWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		SetMenuInputMode(MainMenuWidgetInstance);
	}
	else
	{
		SetGameplayInputEnabled(true);
	}
}

void ASXPlayerController::ReturnToMainMenu()
{
	if (MainMenuLevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("SXPlayerController %s has no MainMenuLevelName."), *GetNameSafe(this));
		return;
	}

	SetPause(false);
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

void ASXPlayerController::RestartCurrentLevel()
{
	SetPause(false);

	const FName CurrentLevelName(*UGameplayStatics::GetCurrentLevelName(this, true));
	UGameplayStatics::OpenLevel(this, CurrentLevelName);
}

void ASXPlayerController::ShowGameOver()
{
	if (bGameOverShown)
	{
		return;
	}

	UUserWidget* GameOverWidget = CreateMenuWidget(GameOverWidgetClass, GameOverWidgetInstance, 30);
	if (IsValid(GameOverWidget) == false)
	{
		return;
	}

	bGameOverShown = true;
	GameOverWidget->SetVisibility(ESlateVisibility::Visible);
	SetMenuInputMode(GameOverWidget);
	SetGameplayInputEnabled(false);

	if (bPauseOnGameOver)
	{
		SetPause(true);
	}
	else
	{
		SetPause(false);
	}
}

void ASXPlayerController::HandleControlledPawnDeath(USXStatusComponent* StatusComponent, AActor* InstigatorActor)
{
	ShowGameOver();
}

void ASXPlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
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

void ASXPlayerController::SetMenuInputMode(UUserWidget* WidgetToFocus)
{
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	if (IsValid(WidgetToFocus) == true)
	{
		InputMode.SetWidgetToFocus(WidgetToFocus->TakeWidget());
	}

	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

UUserWidget* ASXPlayerController::CreateMenuWidget(TSubclassOf<UUserWidget> WidgetClass, TObjectPtr<UUserWidget>& WidgetInstance, int32 ZOrder)
{
	if (IsValid(WidgetClass) == false)
	{
		return nullptr;
	}

	if (IsValid(WidgetInstance) == false)
	{
		WidgetInstance = CreateWidget<UUserWidget>(this, WidgetClass);
		if (IsValid(WidgetInstance) == true)
		{
			WidgetInstance->AddToViewport(ZOrder);
			WidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	return WidgetInstance;
}
