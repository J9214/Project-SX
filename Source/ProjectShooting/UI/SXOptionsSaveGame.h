// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericWindow.h"
#include "GameFramework/SaveGame.h"
#include "SXOptionsSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FSXOptionsSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Options|Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Options|Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BGMVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Options|Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SFXVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Options|Gameplay", meta=(ClampMin="0.01"))
	float MouseSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Options|Gameplay")
	bool bInvertLookY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Options|Gameplay")
	bool bToggleAim = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Options|Video")
	FIntPoint ScreenResolution = FIntPoint(1280, 720);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Options|Video")
	TEnumAsByte<EWindowMode::Type> FullscreenMode = EWindowMode::WindowedFullscreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Options|Video")
	bool bVSyncEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Options|Video", meta=(ClampMin="0.0"))
	float FrameRateLimit = 0.0f;
};

UCLASS()
class PROJECTSHOOTING_API USXOptionsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SX|Options")
	FSXOptionsSnapshot Options;
};
