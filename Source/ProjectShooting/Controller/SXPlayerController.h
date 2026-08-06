// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SXPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class USXStatusComponent;
class ASXEnemyCharacterBase;
class ASXStageFlowManager;

USTRUCT(BlueprintType)
struct FSXStageResultStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="SX|UI|Result")
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly, Category="SX|UI|Result")
	int32 Gold = 0;

	UPROPERTY(BlueprintReadOnly, Category="SX|UI|Result")
	float TimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="SX|UI|Result")
	FText TimeText;
};

USTRUCT(BlueprintType)
struct FSXInputMappingContextEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Input")
	TObjectPtr<UInputMappingContext> MappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Input")
	int32 Priority = 0;
};

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASXPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable, Category="SX|Input")
	void AddInputMappingContext(UInputMappingContext* MappingContext, int32 Priority);

	UFUNCTION(BlueprintCallable, Category="SX|Input")
	void RemoveInputMappingContext(UInputMappingContext* MappingContext);

	UFUNCTION(BlueprintCallable, Category="SX|Input")
	void SetGameplayInputEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="SX|UI")
	UUserWidget* GetGameHUDWidget() const { return GameHUDWidgetInstance; }

	UFUNCTION(BlueprintPure, Category="SX|UI")
	UUserWidget* GetCrosshairWidget() const { return CrosshairWidgetInstance; }

	UFUNCTION(BlueprintCallable, Category="SX|UI|Combat")
	void ShowHitMarker(bool bKilled);

	UFUNCTION(BlueprintImplementableEvent, Category="SX|UI|Combat", meta=(DisplayName="On Hit Marker"))
	void BP_OnHitMarker(bool bKilled);

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void ShowMainMenu();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void HideMainMenu();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void HidePauseMenu();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void ShowOptionsMenu();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void HideOptionsMenu();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void RestartCurrentLevel();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void ShowGameOver();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Result")
	void ShowStageClearResult();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Result")
	void HideStageClearResult();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Result")
	void ResetStageResultStats();

	UFUNCTION(BlueprintCallable, Category="SX|UI|Result")
	void AddStageResultKill(int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category="SX|Stage")
	void SetActiveStageFlowManager(ASXStageFlowManager* NewStageFlowManager);

	UFUNCTION(BlueprintPure, Category="SX|UI|Result")
	FSXStageResultStats GetStageResultStats() const;

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void QuitGame();

protected:
	UFUNCTION()
	void HandleControlledPawnDeath(USXStatusComponent* StatusComponent, AActor* InstigatorActor);

	UFUNCTION()
	void HandleControlledPawnGoldChanged(USXStatusComponent* StatusComponent, int32 OldGold, int32 NewGold, int32 Delta);

	UFUNCTION()
	void HandleStageStarted();

	UFUNCTION()
	void HandleStageCleared();

	void ShowGameOverAfterDeathDelay();
	void ApplyDefaultInputMapping();
	void SetGameOnlyInputMode();
	void SetMenuInputMode(UUserWidget* WidgetToFocus);
	void CreateGameHUD();
	bool ShouldShowMainMenuOnBeginPlay() const;
	void BindControlledPawnDeath();
	void BindControlledPawnResultStats();
	void BindStageFlowManager();
	void BindStageFlowManager(ASXStageFlowManager* StageFlowManager);
	void HandleEnemyKilled(ASXEnemyCharacterBase* DeadEnemy, AActor* DamageCauser);
	void RefreshWaveStatusWidgets();
	bool ShouldCreateGameHUDOnBeginPlay() const;
	UUserWidget* CreateMenuWidget(TSubclassOf<UUserWidget> WidgetClass, TObjectPtr<UUserWidget>& WidgetInstance, int32 ZOrder);
	void ApplyResultStatsToWidget(UUserWidget* ResultWidget);
	FText FormatResultTime(float TimeSeconds) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Input")
	TArray<FSXInputMappingContextEntry> DefaultGameplayMappingContexts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Input")
	TObjectPtr<UInputAction> PauseMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Level")
	FName GameplayLevelName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Level")
	FName MainMenuLevelName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI")
	bool bCreateGameHUDOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI")
	bool bShowMainMenuOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI")
	bool bRequireWaveSpawnerForGameHUD = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI")
	bool bPauseOnGameOver = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(ClampMin="0.0", Units="s"))
	float GameOverDelayAfterDeath = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI|Result")
	bool bResetResultStatsOnStageStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI|Result")
	bool bPauseOnStageClearResult = false;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI", Meta=(AllowPrivateAccess))
	TSubclassOf<UUserWidget> GameHUDWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|UI", Meta=(AllowPrivateAccess))
	TObjectPtr<UUserWidget> GameHUDWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI", Meta=(AllowPrivateAccess))
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|UI", Meta=(AllowPrivateAccess))
	TObjectPtr<UUserWidget> CrosshairWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	TObjectPtr<UUserWidget> MainMenuWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	TObjectPtr<UUserWidget> PauseMenuWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	TSubclassOf<UUserWidget> OptionsMenuWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	TObjectPtr<UUserWidget> OptionsMenuWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	TObjectPtr<UUserWidget> GameOverWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI|Result", Meta=(AllowPrivateAccess))
	TSubclassOf<UUserWidget> StageClearWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|UI|Result", Meta=(AllowPrivateAccess))
	TObjectPtr<UUserWidget> StageClearWidgetInstance;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	bool bPauseMenuOpen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	bool bGameOverShown = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|UI|Result", Meta=(AllowPrivateAccess))
	int32 StageResultKills = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|UI|Result", Meta=(AllowPrivateAccess))
	int32 StageResultGold = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|UI|Result", Meta=(AllowPrivateAccess))
	float StageResultStartTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|UI|Result", Meta=(AllowPrivateAccess))
	float StageResultFrozenTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|UI|Result", Meta=(AllowPrivateAccess))
	bool bStageResultTimeFrozen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|UI|Result", Meta=(AllowPrivateAccess))
	bool bStageClearResultShown = false;

	UPROPERTY(Transient)
	TObjectPtr<ASXStageFlowManager> BoundStageFlowManager;

	FTimerHandle GameOverDelayTimerHandle;
	FDelegateHandle EnemyKilledDelegateHandle;
};
