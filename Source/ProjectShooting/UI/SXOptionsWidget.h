// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/SXOptionsSaveGame.h"
#include "SXOptionsWidget.generated.h"

class USoundClass;
class USoundMix;
class USoundSubmix;
class UCheckBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXOnOptionsChangedSignature, FSXOptionsSnapshot, Options);

UCLASS(Abstract, Blueprintable)
class PROJECTSHOOTING_API USXOptionsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="SX|Options")
	void LoadOptions();

	UFUNCTION(BlueprintCallable, Category="SX|Options")
	void ApplyOptions(bool bSaveOptions = true);

	UFUNCTION(BlueprintCallable, Category="SX|Options")
	void RevertPendingOptions();

	UFUNCTION(BlueprintCallable, Category="SX|Options")
	void ResetToDefaults(bool bApplyImmediately = false);

	UFUNCTION(BlueprintCallable, Category="SX|Options")
	void CloseOptions(bool bApplyPendingOptions = false);

	UFUNCTION(BlueprintPure, Category="SX|Options")
	FSXOptionsSnapshot GetPendingOptions() const { return PendingOptions; }

	UFUNCTION(BlueprintPure, Category="SX|Options")
	FSXOptionsSnapshot GetAppliedOptions() const { return AppliedOptions; }

	UFUNCTION(BlueprintCallable, Category="SX|Options|Audio")
	void SetMasterVolume(float Volume, bool bPreview = true);

	UFUNCTION(BlueprintCallable, Category="SX|Options|Audio")
	void SetBGMVolume(float Volume, bool bPreview = true);

	UFUNCTION(BlueprintCallable, Category="SX|Options|Audio")
	void SetSFXVolume(float Volume, bool bPreview = true);

	UFUNCTION(BlueprintCallable, Category="SX|Options|Gameplay")
	void SetMouseSensitivity(float Sensitivity);

	UFUNCTION(BlueprintCallable, Category="SX|Options|Gameplay")
	void SetInvertLookY(bool bInvert);

	UFUNCTION(BlueprintCallable, Category="SX|Options|Gameplay")
	void SetToggleAim(bool bToggle);

	UFUNCTION(BlueprintCallable, Category="SX|Options|Video")
	void SetScreenResolution(FIntPoint Resolution);

	UFUNCTION(BlueprintCallable, Category="SX|Options|Video")
	void SetFullscreenMode(TEnumAsByte<EWindowMode::Type> Mode);

	UFUNCTION(BlueprintCallable, Category="SX|Options|Video")
	void SetVSyncEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="SX|Options|Video")
	void SetFrameRateLimit(float FrameRateLimit);

	UFUNCTION(BlueprintPure, Category="SX|Options|Video")
	TArray<FIntPoint> GetSupportedScreenResolutions() const;

	UPROPERTY(BlueprintAssignable, Category="SX|Options")
	FSXOnOptionsChangedSignature OnOptionsLoaded;

	UPROPERTY(BlueprintAssignable, Category="SX|Options")
	FSXOnOptionsChangedSignature OnPendingOptionsChanged;

	UPROPERTY(BlueprintAssignable, Category="SX|Options")
	FSXOnOptionsChangedSignature OnOptionsApplied;

protected:
	UFUNCTION()
	void HandleInvertLookYCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void HandleToggleAimCheckStateChanged(bool bIsChecked);

	FSXOptionsSnapshot BuildDefaultOptions() const;
	FSXOptionsSnapshot ReadCurrentVideoOptions(FSXOptionsSnapshot BaseOptions) const;
	void BindGameplayOptionWidgets();
	void SyncGameplayOptionWidgets(const FSXOptionsSnapshot& Options);
	void ApplyAudioOptions(const FSXOptionsSnapshot& Options);
	void ApplyGameplayOptionsToOwningPlayer(const FSXOptionsSnapshot& Options);
	void ApplyVideoOptions(const FSXOptionsSnapshot& Options);
	void SaveOptions() const;
	void BroadcastPendingOptionsChanged();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Options|Save")
	FString SaveSlotName = TEXT("SXOptions");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Options|Save")
	int32 SaveUserIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Options|Defaults")
	FSXOptionsSnapshot DefaultOptions;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="SX|Options|Gameplay")
	TObjectPtr<UCheckBox> InvertLookYCheckBox;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="SX|Options|Gameplay")
	TObjectPtr<UCheckBox> ToggleAimCheckBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Options|Audio")
	TObjectPtr<USoundSubmix> MasterSoundSubmix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Options|Audio")
	TObjectPtr<USoundSubmix> BGMSoundSubmix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Options|Audio")
	TObjectPtr<USoundSubmix> SFXSoundSubmix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Options|Audio|Legacy")
	TObjectPtr<USoundMix> SoundMixModifier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Options|Audio|Legacy")
	TObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Options|Audio|Legacy")
	TObjectPtr<USoundClass> BGMSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Options|Audio|Legacy")
	TObjectPtr<USoundClass> SFXSoundClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Options")
	FSXOptionsSnapshot PendingOptions;

	UPROPERTY(Transient, BlueprintReadOnly, Category="SX|Options")
	FSXOptionsSnapshot AppliedOptions;
};
