// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SXPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class USXStatusComponent;

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

	UFUNCTION(BlueprintCallable, Category="SX|UI|Menu")
	void QuitGame();

protected:
	UFUNCTION()
	void HandleControlledPawnDeath(USXStatusComponent* StatusComponent, AActor* InstigatorActor);

	void ApplyDefaultInputMapping();
	void SetGameOnlyInputMode();
	void SetMenuInputMode(UUserWidget* WidgetToFocus);
	void CreateGameHUD();
	bool ShouldShowMainMenuOnBeginPlay() const;
	void BindControlledPawnDeath();
	bool ShouldCreateGameHUDOnBeginPlay() const;
	UUserWidget* CreateMenuWidget(TSubclassOf<UUserWidget> WidgetClass, TObjectPtr<UUserWidget>& WidgetInstance, int32 ZOrder);

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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	bool bPauseMenuOpen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|UI|Menu", Meta=(AllowPrivateAccess))
	bool bGameOverShown = false;
};
