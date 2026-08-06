#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SXStoveSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="STOVE PC SDK"))
class PROJECTSHOOTING_API USXStoveSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	UPROPERTY(Config, EditAnywhere, Category="STOVE", meta=(ToolTip="Enable STOVE PC SDK initialization when the game starts."))
	bool bEnableStoveSDK = false;

	UPROPERTY(Config, EditAnywhere, Category="STOVE")
	FString Environment = TEXT("LIVE");

	UPROPERTY(Config, EditAnywhere, Category="STOVE", meta=(DisplayName="Game ID"))
	FString GameID;

	UPROPERTY(Config, EditAnywhere, Category="STOVE", meta=(DisplayName="Application Key"))
	FString ApplicationKey;

	UPROPERTY(Config, EditAnywhere, Category="Launcher")
	bool bLaunchStoveLauncherIfNeeded = true;

	UPROPERTY(Config, EditAnywhere, Category="Launcher", meta=(ClampMin="1000", Units="ms"))
	int32 LauncherWaitTimeMilliseconds = 60000;
};
