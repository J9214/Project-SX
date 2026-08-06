// Fill out your copyright notice in the Description page of Project Settings.

#include "GameInstance/SXMusicSubsystem.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectShooting.h"
#include "Sound/SoundBase.h"

void USXMusicSubsystem::Deinitialize()
{
	StopMusic(0.0f);
	StopFadingOutComponent();

	Super::Deinitialize();
}

void USXMusicSubsystem::PlayMusic(USoundBase* Music, float FadeTime)
{
	if (IsValid(Music) == false)
	{
		StopMusic(FadeTime);
		return;
	}

	// A level may request its BGM again on BeginPlay after OpenLevel. Keep the
	// persistent component playing instead of restarting the same track.
	if (CurrentMusic == Music && IsValid(MusicAudioComponent) && MusicAudioComponent->IsPlaying())
	{
		return;
	}

	FadeTime = FMath::Max(0.0f, FadeTime);
	StopFadingOutComponent();

	if (IsValid(MusicAudioComponent))
	{
		if (FadeTime > KINDA_SMALL_NUMBER && MusicAudioComponent->IsPlaying())
		{
			FadingOutAudioComponent = MusicAudioComponent;
			FadingOutAudioComponent->FadeOut(FadeTime, 0.0f);
		}
		else
		{
			MusicAudioComponent->Stop();
		}
	}

	MusicAudioComponent = nullptr;
	CurrentMusic = Music;

	// The component is owned by the GameInstance and persists while OpenLevel
	// destroys and replaces the current UWorld.
	MusicAudioComponent = UGameplayStatics::CreateSound2D(
		GetGameInstance(),
		Music,
		1.0f,
		1.0f,
		0.0f,
		nullptr,
		true,
		false);

	if (IsValid(MusicAudioComponent) == false)
	{
		UE_LOG(LogProjectShooting, Warning, TEXT("SXMusicSubsystem failed to create music component for '%s'."), *GetNameSafe(Music));
		CurrentMusic = nullptr;
		return;
	}

	if (FadeTime > KINDA_SMALL_NUMBER)
	{
		MusicAudioComponent->FadeIn(FadeTime, 1.0f, 0.0f);
	}
	else
	{
		MusicAudioComponent->Play();
	}
}

void USXMusicSubsystem::StopMusic(float FadeTime)
{
	FadeTime = FMath::Max(0.0f, FadeTime);
	StopFadingOutComponent();

	if (IsValid(MusicAudioComponent))
	{
		if (FadeTime > KINDA_SMALL_NUMBER && MusicAudioComponent->IsPlaying())
		{
			FadingOutAudioComponent = MusicAudioComponent;
			FadingOutAudioComponent->FadeOut(FadeTime, 0.0f);
		}
		else
		{
			MusicAudioComponent->Stop();
		}
	}

	MusicAudioComponent = nullptr;
	CurrentMusic = nullptr;
}

bool USXMusicSubsystem::IsMusicPlaying() const
{
	return IsValid(MusicAudioComponent) && MusicAudioComponent->IsPlaying();
}

void USXMusicSubsystem::StopFadingOutComponent()
{
	if (IsValid(FadingOutAudioComponent))
	{
		FadingOutAudioComponent->Stop();
	}

	FadingOutAudioComponent = nullptr;
}

