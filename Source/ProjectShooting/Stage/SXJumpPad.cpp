// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/SXJumpPad.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

ASXJumpPad::ASXJumpPad()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(120.0f, 120.0f, 45.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);

	PadMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMeshComponent"));
	PadMeshComponent->SetupAttachment(TriggerBox);
	PadMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PadMeshComponent->SetGenerateOverlapEvents(false);
}

void ASXJumpPad::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleTriggerBeginOverlap);
}

void ASXJumpPad::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	LaunchActor(OtherActor, OtherComp);
}

bool ASXJumpPad::LaunchActor(AActor* TargetActor, UPrimitiveComponent* TargetComponent)
{
	if (CanLaunchActor(TargetActor) == false)
	{
		return false;
	}

	const FVector LaunchVelocity = BuildLaunchVelocity();
	bool bLaunched = false;

	if (bAffectCharacters)
	{
		if (ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor))
		{
			TargetCharacter->LaunchCharacter(LaunchVelocity, bOverrideXYVelocity, bOverrideZVelocity);
			bLaunched = true;
		}
	}

	if (bLaunched == false && bAffectPhysicsObjects && IsValid(TargetComponent) && TargetComponent->IsSimulatingPhysics())
	{
		TargetComponent->SetPhysicsLinearVelocity(LaunchVelocity, false);
		bLaunched = true;
	}

	if (bLaunched == false)
	{
		return false;
	}

	MarkActorLaunched(TargetActor);
	PlayLaunchFeedback();
	OnJumpPadLaunched.Broadcast(TargetActor);
	return true;
}

FVector ASXJumpPad::BuildLaunchVelocity() const
{
	FVector ForwardDirection = bUsePadForwardDirection ? GetActorForwardVector() : FVector::ZeroVector;
	ForwardDirection.Z = 0.0f;
	ForwardDirection = ForwardDirection.GetSafeNormal();

	FVector LaunchVelocity = ForwardDirection * ForwardVelocity;
	LaunchVelocity.Z = UpVelocity;
	return LaunchVelocity;
}

bool ASXJumpPad::CanLaunchActor(AActor* TargetActor) const
{
	if (IsValid(TargetActor) == false || TargetActor == this)
	{
		return false;
	}

	if (bPlayerOnly)
	{
		const APawn* TargetPawn = Cast<APawn>(TargetActor);
		if (IsValid(TargetPawn) == false || TargetPawn->IsPlayerControlled() == false)
		{
			return false;
		}
	}

	if (LaunchCooldown <= 0.0f)
	{
		return true;
	}

	const double* LastLaunchTime = LastLaunchTimes.Find(TargetActor);
	if (LastLaunchTime == nullptr)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return true;
	}

	return World->GetTimeSeconds() - *LastLaunchTime >= LaunchCooldown;
}

void ASXJumpPad::MarkActorLaunched(AActor* TargetActor)
{
	if (IsValid(TargetActor) == false)
	{
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		LastLaunchTimes.FindOrAdd(TargetActor) = World->GetTimeSeconds();
	}
}

void ASXJumpPad::PlayLaunchFeedback() const
{
	const FVector FeedbackLocation = GetActorLocation() + FeedbackOffset;

	if (IsValid(LaunchVFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			LaunchVFX,
			FeedbackLocation,
			GetActorRotation()
		);
	}

	if (IsValid(LaunchSFX))
	{
		UGameplayStatics::PlaySoundAtLocation(this, LaunchSFX, FeedbackLocation);
	}
}
