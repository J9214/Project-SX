// SXPickupComponent.cpp


#include "Components/SXPickupComponent.h"
#include "Character/SXPlayerCharacter.h"

USXPickupComponent::USXPickupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USXPickupComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleOnComponentBeginOverlap);
}

void USXPickupComponent::HandleOnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASXPlayerCharacter* OverlappedCharacter = Cast<ASXPlayerCharacter>(OtherActor);
	if (IsValid(OverlappedCharacter) == true)
	{
		OnPickUp.Broadcast(OverlappedCharacter);

		OnComponentBeginOverlap.RemoveAll(this);
	}
}
