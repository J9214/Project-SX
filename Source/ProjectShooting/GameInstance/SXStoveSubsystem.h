#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SXStoveSubsystem.generated.h"

UCLASS()
class PROJECTSHOOTING_API USXStoveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category="SX|STOVE")
	bool IsStoveInitialized() const { return bStoveInitialized; }

	void HandleLauncherCheckResult(bool bSuccess, uint32 ResultCode, const FString& ErrorMessage, bool bRestartRequired);
	void HandleInitializeResult(bool bSuccess, uint32 ResultCode, const FString& ErrorMessage);

private:
	bool TickStove(float DeltaTime);
	bool LoadBaseSDK();

	bool bInitializationRequested = false;
	bool bStoveInitialized = false;
	FTSTicker::FDelegateHandle CallbackTickerHandle;
};
