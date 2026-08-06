#include "GameInstance/SXStoveSubsystem.h"

#include "BaseSDK.h"
#include "Containers/Ticker.h"
#include "GameInstance/SXStoveSettings.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "ProjectShooting.h"

using namespace Stove::PCSDK;
using namespace Stove::PCSDK::Base;

namespace
{
	TWeakObjectPtr<USXStoveSubsystem> ActiveStoveSubsystem;
	StovePCInitializeParamEx2* InitializeParameters = nullptr;
	void* BaseSDKDLLHandle = nullptr;

	FString GetStoveErrorMessage(const CallbackResult& Callback)
	{
		const wchar_t* ErrorMessage = Callback.GetErrorMessage();
		return ErrorMessage ? FString(ErrorMessage) : FString();
	}

	void __cdecl OnLauncherCheckFinished(CallbackResult Callback, bool bRestartRequired)
	{
		if (USXStoveSubsystem* Subsystem = ActiveStoveSubsystem.Get())
		{
			const Result CallbackValue = Callback.GetResult();
			Subsystem->HandleLauncherCheckResult(
				CallbackValue.IsSuccessful(),
				CallbackValue.GetResultCode(),
				GetStoveErrorMessage(Callback),
				bRestartRequired
			);
		}
	}

	void __cdecl OnBaseInitializeFinished(CallbackResult Callback)
	{
		if (USXStoveSubsystem* Subsystem = ActiveStoveSubsystem.Get())
		{
			const Result CallbackValue = Callback.GetResult();
			Subsystem->HandleInitializeResult(
				CallbackValue.IsSuccessful(),
				CallbackValue.GetResultCode(),
				GetStoveErrorMessage(Callback)
			);
		}
	}
}

void USXStoveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if !PLATFORM_WINDOWS
	return;
#else
	const USXStoveSettings* Settings = GetDefault<USXStoveSettings>();
	if (IsValid(Settings) == false || Settings->bEnableStoveSDK == false)
	{
		UE_LOG(LogProjectShooting, Log, TEXT("STOVE PC SDK is disabled in Project Settings."));
		return;
	}

	if (Settings->GameID.IsEmpty() || Settings->ApplicationKey.IsEmpty())
	{
		UE_LOG(LogProjectShooting, Error, TEXT("STOVE PC SDK is enabled, but Game ID or Application Key is empty."));
		return;
	}

	if (LoadBaseSDK() == false)
	{
		return;
	}

	ActiveStoveSubsystem = this;
	CallbackTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ThisClass::TickStove)
	);

	delete InitializeParameters;
	InitializeParameters = new StovePCInitializeParamEx2();
	InitializeParameters->SetEnvironment(*Settings->Environment);
	InitializeParameters->SetGameID(*Settings->GameID);
	InitializeParameters->SetApplicationKey(*Settings->ApplicationKey);
	InitializeParameters->SetWaitTimeMillisec(static_cast<uint32>(FMath::Max(1000, Settings->LauncherWaitTimeMilliseconds)));
	InitializeParameters->SetLaunchLauncher(Settings->bLaunchStoveLauncherIfNeeded);

	bInitializationRequested = true;
	Base_RestartAppIfNecessaryAsyncEx2(InitializeParameters, &OnLauncherCheckFinished);
	UE_LOG(LogProjectShooting, Log, TEXT("STOVE launcher check requested for Game ID '%s'."), *Settings->GameID);
#endif
}

void USXStoveSubsystem::Deinitialize()
{
#if PLATFORM_WINDOWS
	if (CallbackTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(CallbackTickerHandle);
		CallbackTickerHandle.Reset();
	}

	if (bStoveInitialized)
	{
		const Result UnInitializeResult = Base_UnInitialize();
		if (UnInitializeResult.IsSuccessful())
		{
			UE_LOG(LogProjectShooting, Log, TEXT("STOVE Base SDK shutdown result: %u"), UnInitializeResult.GetResultCode());
		}
		else
		{
			UE_LOG(LogProjectShooting, Warning, TEXT("STOVE Base SDK shutdown failed: %u"), UnInitializeResult.GetResultCode());
		}
	}

	bInitializationRequested = false;
	bStoveInitialized = false;
	ActiveStoveSubsystem.Reset();

	delete InitializeParameters;
	InitializeParameters = nullptr;
#endif

	Super::Deinitialize();
}

bool USXStoveSubsystem::TickStove(float DeltaTime)
{
#if PLATFORM_WINDOWS
	if (bInitializationRequested || bStoveInitialized)
	{
		Base_RunCallbackWithTimeout(2);
	}
#endif
	return true;
}

bool USXStoveSubsystem::LoadBaseSDK()
{
#if !PLATFORM_WINDOWS
	return false;
#else
	if (BaseSDKDLLHandle != nullptr)
	{
		return true;
	}

	const FString DLLPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries/Win64/BaseSDK.dll"));
	BaseSDKDLLHandle = FPlatformProcess::GetDllHandle(*DLLPath);
	if (BaseSDKDLLHandle == nullptr)
	{
		UE_LOG(LogProjectShooting, Error, TEXT("Failed to load STOVE Base SDK DLL: %s"), *DLLPath);
		return false;
	}

	return true;
#endif
}

void USXStoveSubsystem::HandleLauncherCheckResult(
	bool bSuccess,
	uint32 ResultCode,
	const FString& ErrorMessage,
	bool bRestartRequired
)
{
	if (bSuccess == false)
	{
		bInitializationRequested = false;
		UE_LOG(LogProjectShooting, Error, TEXT("STOVE launcher check failed (%u): %s"), ResultCode, *ErrorMessage);
		return;
	}

	if (bRestartRequired)
	{
		bInitializationRequested = false;
		UE_LOG(LogProjectShooting, Warning, TEXT("The game must be restarted through the STOVE launcher."));

#if !WITH_EDITOR
		FPlatformMisc::RequestExit(false);
#endif
		return;
	}

	Base_InitializeEx(&OnBaseInitializeFinished);
}

void USXStoveSubsystem::HandleInitializeResult(bool bSuccess, uint32 ResultCode, const FString& ErrorMessage)
{
	bInitializationRequested = false;
	bStoveInitialized = bSuccess;

	if (bSuccess)
	{
		UE_LOG(LogProjectShooting, Log, TEXT("STOVE Base SDK initialized successfully."));
	}
	else
	{
		UE_LOG(LogProjectShooting, Error, TEXT("STOVE Base SDK initialization failed (%u): %s"), ResultCode, *ErrorMessage);
	}
}
