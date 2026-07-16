// SXPickupComponent.cpp


#include "Components/SXPickupComponent.h"
#include "Character/SXPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

USXPickupComponent::USXPickupComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
}

void USXPickupComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleOnComponentBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleOnComponentEndOverlap);

	if (PickupMethod == ESXPickupMethod::Magnet)
	{
		SetComponentTickEnabled(true);
	}
}

void USXPickupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (PickupMethod != ESXPickupMethod::Magnet || bPickedUp || IsValid(Owner) == false)
	{
		SetComponentTickEnabled(false);
		return;
	}

	if (IsValid(MagnetTargetCharacter) == false)
	{
		UpdateMagnetTarget();
	}

	if (IsValid(MagnetTargetCharacter) == false)
	{
		return;
	}

	const FVector CurrentLocation = Owner->GetActorLocation();
	const FVector TargetLocation = MagnetTargetCharacter->GetActorLocation();
	const FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MagnetMoveSpeed);
	Owner->SetActorLocation(NewLocation);

	if (FVector::DistSquared(NewLocation, TargetLocation) <= FMath::Square(PickupDistance))
	{
		PickUp(MagnetTargetCharacter);
	}
}

bool USXPickupComponent::CanInteract_Implementation(ASXPlayerCharacter* InteractingCharacter) const
{
	return bPickedUp == false && IsValid(InteractingCharacter) == true && PickupMethod == ESXPickupMethod::Interaction;
}

void USXPickupComponent::Interact_Implementation(ASXPlayerCharacter* InteractingCharacter)
{
	PickUp(InteractingCharacter);
}

FText USXPickupComponent::GetInteractionText_Implementation() const
{
	return InteractionText;
}

void USXPickupComponent::PickUp(ASXPlayerCharacter* InPickUpCharacter)
{
	if (bPickedUp || IsValid(InPickUpCharacter) == false)
	{
		return;
	}

	if (OnPickUp.IsBound() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupComponent %s has no OnPickUp binding."), *GetNameSafe(this));
		return;
	}

	bPickedUp = true;
	SetComponentTickEnabled(false);
	OnPickUp.Broadcast(InPickUpCharacter);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InPickUpCharacter->ClearInteractionCandidate(this);
}

void USXPickupComponent::HandleOnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASXPlayerCharacter* OverlappedCharacter = Cast<ASXPlayerCharacter>(OtherActor);
	if (IsValid(OverlappedCharacter) == false || bPickedUp)
	{
		return;
	}

	switch (PickupMethod)
	{
	case ESXPickupMethod::Magnet:
		StartMagnetPickup(OverlappedCharacter);
		break;
	case ESXPickupMethod::Interaction:
	default:
		OverlappedCharacter->SetInteractionCandidate(this);
		break;
	}
}

void USXPickupComponent::HandleOnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASXPlayerCharacter* OverlappedCharacter = Cast<ASXPlayerCharacter>(OtherActor);
	if (IsValid(OverlappedCharacter) == false || PickupMethod != ESXPickupMethod::Interaction)
	{
		return;
	}

	OverlappedCharacter->ClearInteractionCandidate(this);
}

void USXPickupComponent::StartMagnetPickup(ASXPlayerCharacter* InPickUpCharacter)
{
	if (IsValid(InPickUpCharacter) == false)
	{
		return;
	}

	MagnetTargetCharacter = InPickUpCharacter;
	SetComponentTickEnabled(true);
}

void USXPickupComponent::UpdateMagnetTarget()
{
	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false)
	{
		return;
	}

	ASXPlayerCharacter* PlayerCharacter = Cast<ASXPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (IsValid(PlayerCharacter) == false)
	{
		return;
	}

	const float DistanceSquared = FVector::DistSquared(Owner->GetActorLocation(), PlayerCharacter->GetActorLocation());
	if (DistanceSquared <= FMath::Square(MagnetAttractRadius))
	{
		StartMagnetPickup(PlayerCharacter);
	}
}
