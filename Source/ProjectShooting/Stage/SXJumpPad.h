// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXJumpPad.generated.h"

class ACharacter;
class UNiagaraSystem;
class UBoxComponent;
class UPrimitiveComponent;
class USoundBase;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXOnJumpPadLaunchedSignature, AActor*, LaunchedActor);

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXJumpPad : public AActor
{
	GENERATED_BODY()

public:
	ASXJumpPad();

	UFUNCTION(BlueprintCallable, Category="SX|Jump Pad")
	bool LaunchActor(AActor* TargetActor, UPrimitiveComponent* TargetComponent = nullptr);

	UPROPERTY(BlueprintAssignable, Category="SX|Jump Pad")
	FSXOnJumpPadLaunchedSignature OnJumpPadLaunched;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	FVector BuildLaunchVelocity() const;
	bool CanLaunchActor(AActor* TargetActor) const;
	void MarkActorLaunched(AActor* TargetActor);
	void PlayLaunchFeedback() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UStaticMeshComponent> PadMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad")
	bool bPlayerOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad")
	bool bAffectCharacters = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad")
	bool bAffectPhysicsObjects = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad")
	bool bUsePadForwardDirection = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad", meta=(ClampMin="0.0", Units="cm/s"))
	float UpVelocity = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad", meta=(ClampMin="0.0", Units="cm/s"))
	float ForwardVelocity = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad")
	bool bOverrideXYVelocity = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad")
	bool bOverrideZVelocity = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad", meta=(ClampMin="0.0", Units=s))
	float LaunchCooldown = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad|Feedback")
	TObjectPtr<UNiagaraSystem> LaunchVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad|Feedback")
	TObjectPtr<USoundBase> LaunchSFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Jump Pad|Feedback")
	FVector FeedbackOffset = FVector(0.0f, 0.0f, 40.0f);

	TMap<AActor*, double> LastLaunchTimes;
};
