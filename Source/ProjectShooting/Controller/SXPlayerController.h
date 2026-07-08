// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SXPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

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

	UFUNCTION(BlueprintCallable, Category="SX|Input")
	void AddInputMappingContext(UInputMappingContext* MappingContext, int32 Priority);

	UFUNCTION(BlueprintCallable, Category="SX|Input")
	void RemoveInputMappingContext(UInputMappingContext* MappingContext);

	UFUNCTION(BlueprintCallable, Category="SX|Input")
	void SetGameplayInputEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="SX|UI")
	UUserWidget* GetGameHUDWidget() const { return GameHUDWidgetInstance; }

protected:
	void ApplyDefaultInputMapping();
	void SetGameOnlyInputMode();
	void CreateGameHUD();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Input")
	TArray<FSXInputMappingContextEntry> DefaultGameplayMappingContexts;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI", Meta=(AllowPrivateAccess))
	TSubclassOf<UUserWidget> GameHUDWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|UI", Meta=(AllowPrivateAccess))
	TObjectPtr<UUserWidget> GameHUDWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|UI", Meta=(AllowPrivateAccess))
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|UI", Meta=(AllowPrivateAccess))
	TObjectPtr<UUserWidget> CrosshairWidgetInstance;
};
