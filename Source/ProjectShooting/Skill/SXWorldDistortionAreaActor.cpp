// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SXWorldDistortionAreaActor.h"

#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"

ASXWorldDistortionAreaActor::ASXWorldDistortionAreaActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetSphereRadius(Radius);

	DistortionMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DistortionMeshComponent"));
	DistortionMeshComponent->SetupAttachment(CollisionComponent);
	DistortionMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	DistortionMeshComponent->SetGenerateOverlapEvents(false);
	DistortionMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, VisualZOffset));
	DistortionMeshComponent->SetHiddenInGame(true);
	DistortionMeshComponent->SetVisibility(false);

	RangeDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("RangeDecalComponent"));
	RangeDecalComponent->SetupAttachment(CollisionComponent);
	RangeDecalComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	RangeDecalComponent->DecalSize = FVector(DecalDepth, Radius, Radius);
	RangeDecalComponent->SetHiddenInGame(true);
	RangeDecalComponent->SetVisibility(false);
}

void ASXWorldDistortionAreaActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Duration <= 0.0f)
	{
		UpdateVisuals(1.0f);
		return;
	}

	ElapsedTime += DeltaSeconds;
	const float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
	UpdateVisuals(Alpha);
}

void ASXWorldDistortionAreaActor::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->SetSphereRadius(Radius);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleAreaBeginOverlap);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleAreaEndOverlap);

	InitializeVisuals();
	ApplyDistortionParameters();

	if (Duration > 0.0f)
	{
		SetLifeSpan(Duration);
	}

	TArray<AActor*> OverlappingActors;
	CollisionComponent->GetOverlappingActors(OverlappingActors);
	for (AActor* OverlappingActor : OverlappingActors)
	{
		ApplySlowToActor(OverlappingActor);
	}
}

void ASXWorldDistortionAreaActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAllSlows();
	ClearDistortionParameters();

	Super::EndPlay(EndPlayReason);
}

void ASXWorldDistortionAreaActor::InitializeWorldDistortionArea(
	AActor* InSourceActor,
	float InRadius,
	float InSlowMultiplier,
	float InDuration,
	UMaterialParameterCollection* InParameterCollection,
	FName InPositionParameterName
)
{
	SourceActor = InSourceActor;
	Radius = FMath::Max(0.0f, InRadius);
	SlowMultiplier = FMath::Clamp(InSlowMultiplier, 0.0f, 1.0f);
	Duration = InDuration;
	ParameterCollection = InParameterCollection;
	PositionParameterName = InPositionParameterName;

	if (IsValid(CollisionComponent.Get()) == true)
	{
		CollisionComponent->SetSphereRadius(Radius);
	}

	if (IsValid(RangeDecalComponent.Get()) == true)
	{
		RangeDecalComponent->DecalSize = FVector(DecalDepth, Radius, Radius);
	}
}

void ASXWorldDistortionAreaActor::HandleAreaBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	ApplySlowToActor(OtherActor);
}

void ASXWorldDistortionAreaActor::HandleAreaEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	RemoveSlowFromActor(OtherActor);
}

void ASXWorldDistortionAreaActor::ApplyDistortionParameters()
{
	if (IsValid(ParameterCollection.Get()) == false)
	{
		return;
	}

	if (PositionParameterName != NAME_None)
	{
		const FVector Location = GetActorLocation();
		UKismetMaterialLibrary::SetVectorParameterValue(this, ParameterCollection, PositionParameterName, FLinearColor(Location.X, Location.Y, Location.Z, 1.0f));
	}

}

void ASXWorldDistortionAreaActor::ClearDistortionParameters()
{
	if (bClearDistortionParameterOnEnd == false || IsValid(ParameterCollection.Get()) == false)
	{
		return;
	}

	if (PositionParameterName != NAME_None)
	{
		UKismetMaterialLibrary::SetVectorParameterValue(
			this,
			ParameterCollection,
			PositionParameterName,
			FLinearColor(ClearedDistortionPosition.X, ClearedDistortionPosition.Y, ClearedDistortionPosition.Z, 1.0f)
		);
	}

}

void ASXWorldDistortionAreaActor::ApplySlowToActor(AActor* TargetActor)
{
	if (IsValid(TargetActor) == false)
	{
		return;
	}

	if (bAffectSourceActor == false && TargetActor == SourceActor)
	{
		return;
	}

	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	if (IsValid(TargetCharacter) == false)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = TargetCharacter->GetCharacterMovement();
	if (IsValid(MovementComponent) == false || OriginalMaxWalkSpeeds.Contains(MovementComponent))
	{
		return;
	}

	OriginalMaxWalkSpeeds.Add(MovementComponent, MovementComponent->MaxWalkSpeed);
	MovementComponent->MaxWalkSpeed *= SlowMultiplier;
}

void ASXWorldDistortionAreaActor::RemoveSlowFromActor(AActor* TargetActor)
{
	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	if (IsValid(TargetCharacter) == false)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = TargetCharacter->GetCharacterMovement();
	if (IsValid(MovementComponent) == false)
	{
		return;
	}

	if (float* OriginalMaxWalkSpeed = OriginalMaxWalkSpeeds.Find(MovementComponent))
	{
		MovementComponent->MaxWalkSpeed = *OriginalMaxWalkSpeed;
		OriginalMaxWalkSpeeds.Remove(MovementComponent);
	}
}

void ASXWorldDistortionAreaActor::RemoveAllSlows()
{
	for (const TPair<UCharacterMovementComponent*, float>& OriginalSpeedPair : OriginalMaxWalkSpeeds)
	{
		if (IsValid(OriginalSpeedPair.Key) == true)
		{
			OriginalSpeedPair.Key->MaxWalkSpeed = OriginalSpeedPair.Value;
		}
	}

	OriginalMaxWalkSpeeds.Empty();
}

void ASXWorldDistortionAreaActor::InitializeVisuals()
{
	if (IsValid(DistortionMaterial.Get()) == true && IsValid(DistortionMeshComponent.Get()) == true)
	{
		DistortionMaterialInstance = DistortionMeshComponent->CreateDynamicMaterialInstance(0, DistortionMaterial);
		DistortionMeshComponent->SetHiddenInGame(false);
		DistortionMeshComponent->SetVisibility(true);
	}
	else if (IsValid(DistortionMeshComponent.Get()) == true)
	{
		DistortionMaterialInstance = nullptr;
		DistortionMeshComponent->SetHiddenInGame(true);
		DistortionMeshComponent->SetVisibility(false);
	}

	if (IsValid(RangeDecalMaterial.Get()) == true && IsValid(RangeDecalComponent.Get()) == true)
	{
		RangeDecalMaterialInstance = UMaterialInstanceDynamic::Create(RangeDecalMaterial, this);
		RangeDecalComponent->SetDecalMaterial(RangeDecalMaterialInstance);
		RangeDecalComponent->SetHiddenInGame(false);
		RangeDecalComponent->SetVisibility(true);
	}
	else if (IsValid(RangeDecalComponent.Get()) == true)
	{
		RangeDecalComponent->SetDecalMaterial(nullptr);
		RangeDecalComponent->SetHiddenInGame(true);
		RangeDecalComponent->SetVisibility(false);
	}

	if (bAutoScaleVisualsToRadius)
	{
		const float Diameter = Radius * 2.0f;
		const float SafeVisualMeshBaseSize = FMath::Max(1.0f, VisualMeshBaseSize);
		const float MeshScaleXY = Diameter / SafeVisualMeshBaseSize;

		if (IsValid(DistortionMeshComponent.Get()) == true)
		{
			DistortionMeshComponent->SetRelativeScale3D(FVector(MeshScaleXY, MeshScaleXY, VisualMeshZScale));
			DistortionMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, VisualZOffset));
		}

		if (IsValid(RangeDecalComponent.Get()) == true)
		{
			RangeDecalComponent->DecalSize = FVector(DecalDepth, Radius, Radius);
		}
	}

	UpdateVisuals(0.0f);
}

void ASXWorldDistortionAreaActor::UpdateVisuals(float Alpha)
{
	const float Opacity = 1.0f - Alpha;
	const float DistortionStrength = FMath::Sin(Alpha * PI);

	if (IsValid(DistortionMaterialInstance.Get()) == true)
	{
		if (ProgressParameterName != NAME_None)
		{
			DistortionMaterialInstance->SetScalarParameterValue(ProgressParameterName, Alpha);
		}

		if (OpacityParameterName != NAME_None)
		{
			DistortionMaterialInstance->SetScalarParameterValue(OpacityParameterName, Opacity);
		}

		if (DistortionStrengthParameterName != NAME_None)
		{
			DistortionMaterialInstance->SetScalarParameterValue(DistortionStrengthParameterName, DistortionStrength);
		}

		if (MaterialRadiusParameterName != NAME_None)
		{
			DistortionMaterialInstance->SetScalarParameterValue(MaterialRadiusParameterName, Radius);
		}
	}

	if (IsValid(RangeDecalMaterialInstance.Get()) == true)
	{
		if (ProgressParameterName != NAME_None)
		{
			RangeDecalMaterialInstance->SetScalarParameterValue(ProgressParameterName, Alpha);
		}

		if (OpacityParameterName != NAME_None)
		{
			RangeDecalMaterialInstance->SetScalarParameterValue(OpacityParameterName, Opacity);
		}

		if (MaterialRadiusParameterName != NAME_None)
		{
			RangeDecalMaterialInstance->SetScalarParameterValue(MaterialRadiusParameterName, Radius);
		}
	}
}
