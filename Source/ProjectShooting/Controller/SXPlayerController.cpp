// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/SXPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Character/SXCharacterBase.h"
#include "Character/SXEnemyCharacterBase.h"
#include "Components/SXStatusComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Stage/SXStageFlowManager.h"
#include "Stage/SXWaveSpawner.h"
#include "TimerManager.h"
#include "UI/SXWaveStatusWidget.h"

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

	ResetStageResultStats();
	BindControlledPawnResultStats();
	BindStageFlowManager();
	EnemyKilledDelegateHandle = ASXEnemyCharacterBase::OnEnemyKilledNative.AddUObject(this, &ThisClass::HandleEnemyKilled);
}

void ASXPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(GameOverDelayTimerHandle);

	if (EnemyKilledDelegateHandle.IsValid())
	{
		ASXEnemyCharacterBase::OnEnemyKilledNative.Remove(EnemyKilledDelegateHandle);
		EnemyKilledDelegateHandle.Reset();
	}

	if (IsValid(BoundStageFlowManager))
	{
		BoundStageFlowManager->OnStageStarted.RemoveDynamic(this, &ThisClass::HandleStageStarted);
		BoundStageFlowManager->OnStageCleared.RemoveDynamic(this, &ThisClass::HandleStageCleared);
	}

	if (ASXCharacterBase* ControlledCharacter = Cast<ASXCharacterBase>(GetPawn()))
	{
		if (USXStatusComponent* StatusComponent = ControlledCharacter->GetStatusComponent())
		{
			StatusComponent->OnDeath.RemoveDynamic(this, &ThisClass::HandleControlledPawnDeath);
			StatusComponent->OnGoldChanged.RemoveDynamic(this, &ThisClass::HandleControlledPawnGoldChanged);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ASXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	BindControlledPawnResultStats();
}

void ASXPlayerController::ResetStageResultStats()
{
	StageResultKills = 0;
	StageResultGold = 0;
	StageResultFrozenTime = 0.0f;
	bStageResultTimeFrozen = false;
	bGameOverShown = false;
	bStageClearResultShown = false;

	UWorld* World = GetWorld();
	StageResultStartTime = IsValid(World) ? World->GetTimeSeconds() : 0.0f;
}

void ASXPlayerController::AddStageResultKill(int32 Count)
{
	StageResultKills = FMath::Max(0, StageResultKills + Count);
}

FSXStageResultStats ASXPlayerController::GetStageResultStats() const
{
	FSXStageResultStats ResultStats;
	ResultStats.Kills = StageResultKills;
	ResultStats.Gold = StageResultGold;

	if (bStageResultTimeFrozen)
	{
		ResultStats.TimeSeconds = StageResultFrozenTime;
	}
	else if (const UWorld* World = GetWorld())
	{
		ResultStats.TimeSeconds = FMath::Max(0.0f, World->GetTimeSeconds() - StageResultStartTime);
	}

	ResultStats.TimeText = FormatResultTime(ResultStats.TimeSeconds);
	return ResultStats;
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

void ASXPlayerController::ShowHitMarker(bool bKilled)
{
	BP_OnHitMarker(bKilled);

	if (IsValid(CrosshairWidgetInstance) == false)
	{
		return;
	}

	UFunction* ShowHitMarkerFunction = CrosshairWidgetInstance->FindFunction(TEXT("ShowHitMarker"));
	if (ShowHitMarkerFunction == nullptr)
	{
		return;
	}

	struct FShowHitMarkerParams
	{
		bool bKilled = false;
	};

	FShowHitMarkerParams Params;
	Params.bKilled = bKilled;
	CrosshairWidgetInstance->ProcessEvent(ShowHitMarkerFunction, &Params);
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

void ASXPlayerController::BindControlledPawnResultStats()
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

	StatusComponent->OnGoldChanged.RemoveDynamic(this, &ThisClass::HandleControlledPawnGoldChanged);
	StatusComponent->OnGoldChanged.AddDynamic(this, &ThisClass::HandleControlledPawnGoldChanged);
}

void ASXPlayerController::HandleControlledPawnGoldChanged(USXStatusComponent* StatusComponent, int32 OldGold, int32 NewGold, int32 Delta)
{
	if (Delta <= 0 || bGameOverShown || bStageClearResultShown)
	{
		return;
	}

	StageResultGold += Delta;
}

void ASXPlayerController::BindStageFlowManager()
{
	ASXStageFlowManager* StageFlowManager = Cast<ASXStageFlowManager>(UGameplayStatics::GetActorOfClass(this, ASXStageFlowManager::StaticClass()));
	if (IsValid(StageFlowManager) == false)
	{
		return;
	}

	BindStageFlowManager(StageFlowManager);
}

void ASXPlayerController::BindStageFlowManager(ASXStageFlowManager* StageFlowManager)
{
	if (IsValid(StageFlowManager) == false)
	{
		return;
	}

	if (BoundStageFlowManager == StageFlowManager)
	{
		RefreshWaveStatusWidgets();
		return;
	}

	if (IsValid(BoundStageFlowManager))
	{
		BoundStageFlowManager->OnStageStarted.RemoveDynamic(this, &ThisClass::HandleStageStarted);
		BoundStageFlowManager->OnStageCleared.RemoveDynamic(this, &ThisClass::HandleStageCleared);
	}

	BoundStageFlowManager = StageFlowManager;
	BoundStageFlowManager->OnStageStarted.AddUniqueDynamic(this, &ThisClass::HandleStageStarted);
	BoundStageFlowManager->OnStageCleared.AddUniqueDynamic(this, &ThisClass::HandleStageCleared);

	RefreshWaveStatusWidgets();
	BoundStageFlowManager->BroadcastStageState();
}

void ASXPlayerController::SetActiveStageFlowManager(ASXStageFlowManager* NewStageFlowManager)
{
	BindStageFlowManager(NewStageFlowManager);
}

void ASXPlayerController::RefreshWaveStatusWidgets()
{
	if (IsValid(GameHUDWidgetInstance) == false)
	{
		return;
	}

	TArray<UWidget*> ChildWidgets;
	if (IsValid(GameHUDWidgetInstance->WidgetTree))
	{
		GameHUDWidgetInstance->WidgetTree->GetAllWidgets(ChildWidgets);
	}

	if (USXWaveStatusWidget* RootWaveStatusWidget = Cast<USXWaveStatusWidget>(GameHUDWidgetInstance.Get()))
	{
		RootWaveStatusWidget->InitWaveStatus(BoundStageFlowManager.Get(), IsValid(BoundStageFlowManager) ? BoundStageFlowManager->GetWaveSpawner() : nullptr);
	}

	for (UWidget* ChildWidget : ChildWidgets)
	{
		if (USXWaveStatusWidget* WaveStatusWidget = Cast<USXWaveStatusWidget>(ChildWidget))
		{
			WaveStatusWidget->InitWaveStatus(BoundStageFlowManager.Get(), IsValid(BoundStageFlowManager) ? BoundStageFlowManager->GetWaveSpawner() : nullptr);
		}
	}
}

void ASXPlayerController::HandleEnemyKilled(ASXEnemyCharacterBase* DeadEnemy, AActor* DamageCauser)
{
	if (IsValid(DeadEnemy) == false || bGameOverShown || bStageClearResultShown)
	{
		return;
	}

	AddStageResultKill(1);
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

	GetWorldTimerManager().ClearTimer(GameOverDelayTimerHandle);
	SetPause(false);
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

void ASXPlayerController::RestartCurrentLevel()
{
	GetWorldTimerManager().ClearTimer(GameOverDelayTimerHandle);
	SetPause(false);

	const FName CurrentLevelName(*UGameplayStatics::GetCurrentLevelName(this, true));
	UGameplayStatics::OpenLevel(this, CurrentLevelName);
}

void ASXPlayerController::ShowGameOver()
{
	GetWorldTimerManager().ClearTimer(GameOverDelayTimerHandle);

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
	ApplyResultStatsToWidget(GameOverWidget);
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

void ASXPlayerController::ShowStageClearResult()
{
	if (bStageClearResultShown)
	{
		return;
	}

	UUserWidget* StageClearWidget = CreateMenuWidget(StageClearWidgetClass, StageClearWidgetInstance, 30);
	if (IsValid(StageClearWidget) == false)
	{
		return;
	}

	bStageClearResultShown = true;
	ApplyResultStatsToWidget(StageClearWidget);
	StageClearWidget->SetVisibility(ESlateVisibility::Visible);
	SetMenuInputMode(StageClearWidget);
	SetGameplayInputEnabled(false);

	if (bPauseOnStageClearResult)
	{
		SetPause(true);
	}
}

void ASXPlayerController::HideStageClearResult()
{
	if (IsValid(StageClearWidgetInstance))
	{
		StageClearWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	bStageClearResultShown = false;
	SetPause(false);
	SetGameplayInputEnabled(true);
}

void ASXPlayerController::HandleControlledPawnDeath(USXStatusComponent* StatusComponent, AActor* InstigatorActor)
{
	GetWorldTimerManager().ClearTimer(GameOverDelayTimerHandle);

	if (bStageResultTimeFrozen == false)
	{
		const UWorld* World = GetWorld();
		StageResultFrozenTime = IsValid(World) ? FMath::Max(0.0f, World->GetTimeSeconds() - StageResultStartTime) : 0.0f;
		bStageResultTimeFrozen = true;
	}

	if (GameOverDelayAfterDeath <= 0.0f)
	{
		ShowGameOver();
		return;
	}

	GetWorldTimerManager().SetTimer(
		GameOverDelayTimerHandle,
		this,
		&ThisClass::ShowGameOverAfterDeathDelay,
		GameOverDelayAfterDeath,
		false
	);
}

void ASXPlayerController::ShowGameOverAfterDeathDelay()
{
	ShowGameOver();
}

void ASXPlayerController::HandleStageStarted()
{
	if (bResetResultStatsOnStageStart)
	{
		ResetStageResultStats();
	}
}

void ASXPlayerController::HandleStageCleared()
{
	if (bStageResultTimeFrozen == false)
	{
		const UWorld* World = GetWorld();
		StageResultFrozenTime = IsValid(World) ? FMath::Max(0.0f, World->GetTimeSeconds() - StageResultStartTime) : 0.0f;
		bStageResultTimeFrozen = true;
	}

	ShowStageClearResult();
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

void ASXPlayerController::ApplyResultStatsToWidget(UUserWidget* ResultWidget)
{
	if (IsValid(ResultWidget) == false)
	{
		return;
	}

	const FSXStageResultStats ResultStats = GetStageResultStats();

	if (UFunction* SetResultStatsFunction = ResultWidget->FindFunction(TEXT("SetResultStats")))
	{
		struct FSetResultStatsParams
		{
			FSXStageResultStats ResultStats;
		};

		FSetResultStatsParams Params;
		Params.ResultStats = ResultStats;
		ResultWidget->ProcessEvent(SetResultStatsFunction, &Params);
		return;
	}

	if (UFunction* UpdateResultStatsFunction = ResultWidget->FindFunction(TEXT("UpdateResultStats")))
	{
		struct FUpdateResultStatsParams
		{
			int32 Kills = 0;
			int32 Gold = 0;
			float TimeSeconds = 0.0f;
			FText TimeText;
		};

		FUpdateResultStatsParams Params;
		Params.Kills = ResultStats.Kills;
		Params.Gold = ResultStats.Gold;
		Params.TimeSeconds = ResultStats.TimeSeconds;
		Params.TimeText = ResultStats.TimeText;
		ResultWidget->ProcessEvent(UpdateResultStatsFunction, &Params);
	}
}

FText ASXPlayerController::FormatResultTime(float TimeSeconds) const
{
	const int32 TotalSeconds = FMath::Max(0, FMath::FloorToInt(TimeSeconds));
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;

	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
}
