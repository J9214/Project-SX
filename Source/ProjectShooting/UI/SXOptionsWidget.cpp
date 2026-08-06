// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/SXOptionsWidget.h"

#include "Character/SXPlayerCharacter.h"
#include "Controller/SXPlayerController.h"
#include "Components/CheckBox.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundSubmix.h"

void USXOptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindGameplayOptionWidgets();
	LoadOptions();
}

void USXOptionsWidget::LoadOptions()
{
	FSXOptionsSnapshot LoadedOptions = BuildDefaultOptions();

	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex);
		if (const USXOptionsSaveGame* OptionsSaveGame = Cast<USXOptionsSaveGame>(SaveGame))
		{
			LoadedOptions = OptionsSaveGame->Options;
		}
	}

	LoadedOptions = ReadCurrentVideoOptions(LoadedOptions);
	PendingOptions = LoadedOptions;
	AppliedOptions = LoadedOptions;

	ApplyAudioOptions(AppliedOptions);
	ApplyGameplayOptionsToOwningPlayer(AppliedOptions);
	SyncGameplayOptionWidgets(AppliedOptions);
	OnOptionsLoaded.Broadcast(AppliedOptions);
	BroadcastPendingOptionsChanged();
}

void USXOptionsWidget::ApplyOptions(bool bSaveOptions)
{
	ApplyAudioOptions(PendingOptions);
	ApplyGameplayOptionsToOwningPlayer(PendingOptions);
	ApplyVideoOptions(PendingOptions);

	AppliedOptions = PendingOptions;
	if (bSaveOptions)
	{
		SaveOptions();
	}

	OnOptionsApplied.Broadcast(AppliedOptions);
}

void USXOptionsWidget::RevertPendingOptions()
{
	PendingOptions = AppliedOptions;
	ApplyAudioOptions(PendingOptions);
	ApplyGameplayOptionsToOwningPlayer(PendingOptions);
	BroadcastPendingOptionsChanged();
	SyncGameplayOptionWidgets(PendingOptions);
}

void USXOptionsWidget::ResetToDefaults(bool bApplyImmediately)
{
	PendingOptions = BuildDefaultOptions();
	BroadcastPendingOptionsChanged();
	SyncGameplayOptionWidgets(PendingOptions);

	if (bApplyImmediately)
	{
		ApplyOptions(true);
	}
}

void USXOptionsWidget::CloseOptions(bool bApplyPendingOptions)
{
	if (bApplyPendingOptions)
	{
		ApplyOptions(true);
	}
	else
	{
		RevertPendingOptions();
	}

	if (ASXPlayerController* SXPlayerController = Cast<ASXPlayerController>(GetOwningPlayer()))
	{
		SXPlayerController->HideOptionsMenu();
		return;
	}

	RemoveFromParent();
}

void USXOptionsWidget::SetMasterVolume(float Volume, bool bPreview)
{
	PendingOptions.MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (bPreview)
	{
		ApplyAudioOptions(PendingOptions);
	}
	BroadcastPendingOptionsChanged();
}

void USXOptionsWidget::SetBGMVolume(float Volume, bool bPreview)
{
	PendingOptions.BGMVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (bPreview)
	{
		ApplyAudioOptions(PendingOptions);
	}
	BroadcastPendingOptionsChanged();
}

void USXOptionsWidget::SetSFXVolume(float Volume, bool bPreview)
{
	PendingOptions.SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (bPreview)
	{
		ApplyAudioOptions(PendingOptions);
	}
	BroadcastPendingOptionsChanged();
}

void USXOptionsWidget::SetMouseSensitivity(float Sensitivity)
{
	PendingOptions.MouseSensitivity = FMath::Max(0.01f, Sensitivity);
	ApplyGameplayOptionsToOwningPlayer(PendingOptions);
	BroadcastPendingOptionsChanged();
}

void USXOptionsWidget::SetInvertLookY(bool bInvert)
{
	if (PendingOptions.bInvertLookY == bInvert)
	{
		return;
	}

	PendingOptions.bInvertLookY = bInvert;
	ApplyGameplayOptionsToOwningPlayer(PendingOptions);
	BroadcastPendingOptionsChanged();
}

void USXOptionsWidget::SetToggleAim(bool bToggle)
{
	if (PendingOptions.bToggleAim == bToggle)
	{
		return;
	}

	PendingOptions.bToggleAim = bToggle;
	ApplyGameplayOptionsToOwningPlayer(PendingOptions);
	BroadcastPendingOptionsChanged();
}

void USXOptionsWidget::HandleInvertLookYCheckStateChanged(bool bIsChecked)
{
	SetInvertLookY(bIsChecked);
}

void USXOptionsWidget::HandleToggleAimCheckStateChanged(bool bIsChecked)
{
	SetToggleAim(bIsChecked);
}

void USXOptionsWidget::SetScreenResolution(FIntPoint Resolution)
{
	if (Resolution.X <= 0 || Resolution.Y <= 0)
	{
		return;
	}

	PendingOptions.ScreenResolution = Resolution;
	BroadcastPendingOptionsChanged();
}

void USXOptionsWidget::SetFullscreenMode(TEnumAsByte<EWindowMode::Type> Mode)
{
	PendingOptions.FullscreenMode = Mode;
	BroadcastPendingOptionsChanged();
}

void USXOptionsWidget::SetVSyncEnabled(bool bEnabled)
{
	PendingOptions.bVSyncEnabled = bEnabled;
	BroadcastPendingOptionsChanged();
}

void USXOptionsWidget::SetFrameRateLimit(float FrameRateLimit)
{
	PendingOptions.FrameRateLimit = FMath::Max(0.0f, FrameRateLimit);
	BroadcastPendingOptionsChanged();
}

TArray<FIntPoint> USXOptionsWidget::GetSupportedScreenResolutions() const
{
	TArray<FIntPoint> Resolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	return Resolutions;
}

FSXOptionsSnapshot USXOptionsWidget::BuildDefaultOptions() const
{
	FSXOptionsSnapshot Options = DefaultOptions;
	if (Options.ScreenResolution.X <= 0 || Options.ScreenResolution.Y <= 0)
	{
		if (GEngine != nullptr && GEngine->GetGameUserSettings() != nullptr)
		{
			Options.ScreenResolution = GEngine->GetGameUserSettings()->GetScreenResolution();
		}
		else
		{
			Options.ScreenResolution = FIntPoint(1280, 720);
		}
	}

	Options.MasterVolume = FMath::Clamp(Options.MasterVolume, 0.0f, 1.0f);
	Options.BGMVolume = FMath::Clamp(Options.BGMVolume, 0.0f, 1.0f);
	Options.SFXVolume = FMath::Clamp(Options.SFXVolume, 0.0f, 1.0f);
	Options.MouseSensitivity = FMath::Max(0.01f, Options.MouseSensitivity);
	Options.FrameRateLimit = FMath::Max(0.0f, Options.FrameRateLimit);
	return Options;
}

FSXOptionsSnapshot USXOptionsWidget::ReadCurrentVideoOptions(FSXOptionsSnapshot BaseOptions) const
{
	if (GEngine == nullptr || GEngine->GetGameUserSettings() == nullptr)
	{
		return BaseOptions;
	}

	const UGameUserSettings* GameUserSettings = GEngine->GetGameUserSettings();
	BaseOptions.ScreenResolution = GameUserSettings->GetScreenResolution();
	BaseOptions.FullscreenMode = GameUserSettings->GetFullscreenMode();
	BaseOptions.bVSyncEnabled = GameUserSettings->IsVSyncEnabled();
	BaseOptions.FrameRateLimit = GameUserSettings->GetFrameRateLimit();
	return BaseOptions;
}

void USXOptionsWidget::BindGameplayOptionWidgets()
{
	if (IsValid(InvertLookYCheckBox))
	{
		InvertLookYCheckBox->OnCheckStateChanged.RemoveDynamic(this, &ThisClass::HandleInvertLookYCheckStateChanged);
		InvertLookYCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::HandleInvertLookYCheckStateChanged);
	}

	if (IsValid(ToggleAimCheckBox))
	{
		ToggleAimCheckBox->OnCheckStateChanged.RemoveDynamic(this, &ThisClass::HandleToggleAimCheckStateChanged);
		ToggleAimCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::HandleToggleAimCheckStateChanged);
	}
}

void USXOptionsWidget::SyncGameplayOptionWidgets(const FSXOptionsSnapshot& Options)
{
	if (IsValid(InvertLookYCheckBox))
	{
		InvertLookYCheckBox->SetIsChecked(Options.bInvertLookY);
	}

	if (IsValid(ToggleAimCheckBox))
	{
		ToggleAimCheckBox->SetIsChecked(Options.bToggleAim);
	}
}

void USXOptionsWidget::ApplyAudioOptions(const FSXOptionsSnapshot& Options)
{
	const float MasterVolume = FMath::Clamp(Options.MasterVolume, 0.0f, 1.0f);
	const float BGMVolume = FMath::Clamp(Options.BGMVolume, 0.0f, 1.0f);
	const float SFXVolume = FMath::Clamp(Options.SFXVolume, 0.0f, 1.0f);

	bool bAppliedSubmixVolume = false;

	if (IsValid(MasterSoundSubmix))
	{
		MasterSoundSubmix->SetSubmixOutputVolume(this, MasterVolume);
		bAppliedSubmixVolume = true;
	}

	if (IsValid(BGMSoundSubmix))
	{
		BGMSoundSubmix->SetSubmixOutputVolume(this, BGMVolume);
		bAppliedSubmixVolume = true;
	}

	if (IsValid(SFXSoundSubmix))
	{
		SFXSoundSubmix->SetSubmixOutputVolume(this, SFXVolume);
		bAppliedSubmixVolume = true;
	}

	if (bAppliedSubmixVolume || IsValid(SoundMixModifier) == false)
	{
		return;
	}

	UGameplayStatics::PushSoundMixModifier(this, SoundMixModifier);

	if (IsValid(MasterSoundClass))
	{
		UGameplayStatics::SetSoundMixClassOverride(this, SoundMixModifier, MasterSoundClass, MasterVolume, 1.0f, 0.0f, true);
	}

	if (IsValid(BGMSoundClass))
	{
		UGameplayStatics::SetSoundMixClassOverride(this, SoundMixModifier, BGMSoundClass, BGMVolume, 1.0f, 0.0f, true);
	}

	if (IsValid(SFXSoundClass))
	{
		UGameplayStatics::SetSoundMixClassOverride(this, SoundMixModifier, SFXSoundClass, SFXVolume, 1.0f, 0.0f, true);
	}
}

void USXOptionsWidget::ApplyGameplayOptionsToOwningPlayer(const FSXOptionsSnapshot& Options)
{
	if (ASXPlayerCharacter* PlayerCharacter = Cast<ASXPlayerCharacter>(GetOwningPlayerPawn()))
	{
		PlayerCharacter->ApplyGameplayOptions(Options);
	}
}

void USXOptionsWidget::ApplyVideoOptions(const FSXOptionsSnapshot& Options)
{
	if (GEngine == nullptr || GEngine->GetGameUserSettings() == nullptr)
	{
		return;
	}

	UGameUserSettings* GameUserSettings = GEngine->GetGameUserSettings();
	GameUserSettings->SetScreenResolution(Options.ScreenResolution);
	GameUserSettings->SetFullscreenMode(Options.FullscreenMode.GetValue());
	GameUserSettings->SetVSyncEnabled(Options.bVSyncEnabled);
	GameUserSettings->SetFrameRateLimit(Options.FrameRateLimit);
	GameUserSettings->ApplySettings(false);
	GameUserSettings->SaveSettings();
}

void USXOptionsWidget::SaveOptions() const
{
	USXOptionsSaveGame* OptionsSaveGame = Cast<USXOptionsSaveGame>(UGameplayStatics::CreateSaveGameObject(USXOptionsSaveGame::StaticClass()));
	if (IsValid(OptionsSaveGame) == false)
	{
		return;
	}

	OptionsSaveGame->Options = AppliedOptions;
	UGameplayStatics::SaveGameToSlot(OptionsSaveGame, SaveSlotName, SaveUserIndex);
}

void USXOptionsWidget::BroadcastPendingOptionsChanged()
{
	OnPendingOptionsChanged.Broadcast(PendingOptions);
}
