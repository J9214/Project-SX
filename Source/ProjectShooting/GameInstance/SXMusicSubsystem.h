// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SXMusicSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

/**
 * Owns the currently playing BGM independently from any level.
 * Music created here can survive OpenLevel and is deduplicated by asset.
 */
UCLASS()
class PROJECTSHOOTING_API USXMusicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	/** Plays Music, crossfading from the existing track when necessary. */
	UFUNCTION(BlueprintCallable, Category="SX|Audio|Music", meta=(DisplayName="Play Music"))
	void PlayMusic(USoundBase* Music, float FadeTime = 1.0f);

	/** Stops the current track using an optional fade out. */
	UFUNCTION(BlueprintCallable, Category="SX|Audio|Music", meta=(DisplayName="Stop Music"))
	void StopMusic(float FadeTime = 1.0f);

	UFUNCTION(BlueprintPure, Category="SX|Audio|Music")
	USoundBase* GetCurrentMusic() const { return CurrentMusic; }

	UFUNCTION(BlueprintPure, Category="SX|Audio|Music")
	bool IsMusicPlaying() const;

private:
	void StopFadingOutComponent();

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CurrentMusic;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> MusicAudioComponent;

	/** Keeps the previous component alive until its FadeOut has finished. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> FadingOutAudioComponent;
};

