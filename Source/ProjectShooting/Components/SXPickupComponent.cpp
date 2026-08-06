// SXPickupComponent.cpp


#include "Components/SXPickupComponent.h"
#include "Character/SXPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

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

	if (bUseDropPhysics)
	{
		StartDropPhysics();
	}
	else if (bFloatAfterDrop)
	{
		StartFloating();
	}

	RefreshTickState();
}

void USXPickupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (bPickedUp || IsValid(Owner) == false)
	{
		SetComponentTickEnabled(false);
		return;
	}

	if (PickupMethod == ESXPickupMethod::Magnet && IsValid(MagnetTargetCharacter) == false)
	{
		UpdateMagnetTarget();
	}

	if (PickupMethod == ESXPickupMethod::Magnet && IsValid(MagnetTargetCharacter))
	{
		if (bDropPhysicsActive || bKinematicDropActive)
		{
			StopDropPhysics();
		}

		bFloatingActive = false;

		const FVector CurrentLocation = Owner->GetActorLocation();
		const FVector TargetLocation = MagnetTargetCharacter->GetActorLocation();
		const FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MagnetMoveSpeed);
		Owner->SetActorLocation(NewLocation);

		if (FVector::DistSquared(NewLocation, TargetLocation) <= FMath::Square(PickupDistance))
		{
			PickUp(MagnetTargetCharacter);
		}
		return;
	}

	if (bKinematicDropActive)
	{
		UpdateKinematicDrop(DeltaTime);
		return;
	}

	if (bFloatingActive)
	{
		UpdateFloating(DeltaTime);
		return;
	}

	if (PickupMethod != ESXPickupMethod::Magnet)
	{
		RefreshTickState();
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
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DropPhysicsTimerHandle);
	}
	bDropPhysicsActive = false;
	bKinematicDropActive = false;
	bFloatingActive = false;
	SetSimulatePhysics(false);
	SetEnableGravity(false);
	SetComponentTickEnabled(false);
	OnPickUp.Broadcast(InPickUpCharacter);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InPickUpCharacter->ClearInteractionCandidate(this);
}

void USXPickupComponent::DisablePickupBehavior()
{
	bPickedUp = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DropPhysicsTimerHandle);
	}

	MagnetTargetCharacter = nullptr;
	bDropPhysicsActive = false;
	bKinematicDropActive = false;
	bFloatingActive = false;
	KinematicDropVelocity = FVector::ZeroVector;
	KinematicDropElapsedTime = 0.0f;
	FloatingElapsedTime = 0.0f;

	SetPhysicsLinearVelocity(FVector::ZeroVector);
	SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	SetSimulatePhysics(false);
	SetEnableGravity(false);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetComponentTickEnabled(false);
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
	RefreshTickState();
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

void USXPickupComponent::StartDropPhysics()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (IsValid(Owner) == false || IsValid(World) == false)
	{
		return;
	}

	bDropPhysicsActive = true;
	bKinematicDropActive = false;
	bFloatingActive = false;

	const FVector RandomDirection = FMath::VRand();
	const FVector HorizontalDirection = FVector(RandomDirection.X, RandomDirection.Y, 0.0f).GetSafeNormal();
	const FVector DropImpulse = HorizontalDirection * DropImpulseXY + FVector::UpVector * DropImpulseZ;

	if (IsValid(GetStaticMesh()) == false)
	{
		bDropPhysicsActive = false;
		bKinematicDropActive = true;
		KinematicDropElapsedTime = 0.0f;
		KinematicDropVelocity = DropImpulse;
		RefreshTickState();
		return;
	}

	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGenerateOverlapEvents(true);

	SetSimulatePhysics(true);
	SetEnableGravity(true);

	AddImpulse(DropImpulse, NAME_None, true);

	const FVector AngularImpulse = FMath::VRand() * DropAngularImpulse;
	AddAngularImpulseInDegrees(AngularImpulse, NAME_None, true);

	World->GetTimerManager().SetTimer(
		DropPhysicsTimerHandle,
		this,
		&ThisClass::StopDropPhysics,
		DropPhysicsTime,
		false
	);
}

void USXPickupComponent::StopDropPhysics()
{
	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DropPhysicsTimerHandle);
	}

	bDropPhysicsActive = false;
	bKinematicDropActive = false;

	SetPhysicsLinearVelocity(FVector::ZeroVector);
	SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	SetSimulatePhysics(false);
	SetEnableGravity(false);

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGenerateOverlapEvents(true);

	if (bFloatAfterDrop)
	{
		StartFloating();
	}
	else
	{
		RefreshTickState();
	}
}

void USXPickupComponent::UpdateKinematicDrop(float DeltaTime)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (IsValid(Owner) == false || IsValid(World) == false)
	{
		return;
	}

	KinematicDropElapsedTime += DeltaTime;

	const FVector CurrentLocation = Owner->GetActorLocation();
	KinematicDropVelocity.Z += World->GetGravityZ() * DeltaTime;
	const FVector NextLocation = CurrentLocation + KinematicDropVelocity * DeltaTime;

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(SXPickupKinematicDropTrace), false, Owner);
	FHitResult HitResult;
	const FVector TraceStart = CurrentLocation + FVector::UpVector * 10.0f;
	const FVector TraceEnd = NextLocation - FVector::UpVector * 15.0f;
	const bool bHitGround = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	if (bHitGround || KinematicDropElapsedTime >= DropPhysicsTime)
	{
		if (bHitGround)
		{
			Owner->SetActorLocation(HitResult.ImpactPoint);
		}
		else
		{
			Owner->SetActorLocation(NextLocation);
		}
		StopDropPhysics();
		return;
	}

	Owner->SetActorLocation(NextLocation);
}

void USXPickupComponent::StartFloating()
{
	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false)
	{
		return;
	}

	FloatingElapsedTime = FMath::FRandRange(0.0f, 6.28318530718f);
	FloatingBaseLocation = Owner->GetActorLocation();

	if (bSnapToGroundBeforeFloating)
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			const FVector ActorLocation = Owner->GetActorLocation();
			const FVector TraceStart = ActorLocation + FVector::UpVector * 50.0f;
			const FVector TraceEnd = ActorLocation - FVector::UpVector * GroundTraceDistance;

			FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(SXPickupGroundTrace), false, Owner);
			FHitResult HitResult;
			if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams))
			{
				FloatingBaseLocation = HitResult.ImpactPoint + FVector::UpVector * FloatingHeight;
				Owner->SetActorLocation(FloatingBaseLocation);
			}
			else
			{
				FloatingBaseLocation += FVector::UpVector * FloatingHeight;
				Owner->SetActorLocation(FloatingBaseLocation);
			}
		}
	}

	bFloatingActive = true;
	RefreshTickState();
}

void USXPickupComponent::UpdateFloating(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false)
	{
		return;
	}

	FloatingElapsedTime += DeltaTime;

	const float ZOffset = FMath::Sin(FloatingElapsedTime * FloatingSpeed) * FloatingAmplitude;
	const FVector NewLocation = FloatingBaseLocation + FVector::UpVector * ZOffset;
	Owner->SetActorLocation(NewLocation);

	if (!FMath::IsNearlyZero(FloatingRotationSpeed))
	{
		Owner->AddActorWorldRotation(FRotator(0.0f, FloatingRotationSpeed * DeltaTime, 0.0f));
	}
}

void USXPickupComponent::RefreshTickState()
{
	SetComponentTickEnabled(bPickedUp == false && (PickupMethod == ESXPickupMethod::Magnet || bFloatingActive || bKinematicDropActive));
}
