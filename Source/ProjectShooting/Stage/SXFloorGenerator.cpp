// SXFloorGenerator.cpp

#include "Stage/SXFloorGenerator.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"

ASXFloorGenerator::ASXFloorGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BasicFloorHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("BasicFloorHISM"));
	BasicFloorHISM->SetupAttachment(SceneRoot);
	BasicFloorHISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BasicFloorHISM->SetCollisionObjectType(ECC_WorldStatic);
	BasicFloorHISM->SetCollisionResponseToAllChannels(ECR_Block);

	DamagedFloorHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("DamagedFloorHISM"));
	DamagedFloorHISM->SetupAttachment(SceneRoot);
	DamagedFloorHISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DamagedFloorHISM->SetCollisionObjectType(ECC_WorldStatic);
	DamagedFloorHISM->SetCollisionResponseToAllChannels(ECR_Block);

	EdgeFloorHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("EdgeFloorHISM"));
	EdgeFloorHISM->SetupAttachment(SceneRoot);
	EdgeFloorHISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	EdgeFloorHISM->SetCollisionObjectType(ECC_WorldStatic);
	EdgeFloorHISM->SetCollisionResponseToAllChannels(ECR_Block);

	CornerFloorHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("CornerFloorHISM"));
	CornerFloorHISM->SetupAttachment(SceneRoot);
	CornerFloorHISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CornerFloorHISM->SetCollisionObjectType(ECC_WorldStatic);
	CornerFloorHISM->SetCollisionResponseToAllChannels(ECR_Block);

	HazardFloorHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HazardFloorHISM"));
	HazardFloorHISM->SetupAttachment(SceneRoot);
	HazardFloorHISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HazardFloorHISM->SetCollisionObjectType(ECC_WorldStatic);
	HazardFloorHISM->SetCollisionResponseToAllChannels(ECR_Block);
}

void ASXFloorGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bGenerateOnConstruction)
	{
		RebuildFloor();
	}
}

void ASXFloorGenerator::RebuildFloor()
{
	ClearFloor();

	Width = FMath::Max(1, Width);
	Height = FMath::Max(1, Height);
	TileSize = FMath::Max(1.0f, TileSize);
	CircleRadius = FMath::Max(0.0f, CircleRadius);
	DamagedTileChance = FMath::Clamp(DamagedTileChance, 0.0f, 1.0f);
	HazardTileChance = FMath::Clamp(HazardTileChance, 0.0f, 1.0f);

	FRandomStream RandomStream(RandomSeed);

	const float HalfWidth = Width * TileSize * 0.5f;
	const float HalfHeight = Height * TileSize * 0.5f;

	for (int32 XIndex = 0; XIndex < Width; ++XIndex)
	{
		for (int32 YIndex = 0; YIndex < Height; ++YIndex)
		{
			const float X = bCenteredOnActor
				? (-HalfWidth + TileSize * 0.5f + XIndex * TileSize)
				: (TileSize * 0.5f + XIndex * TileSize);

			const float Y = bCenteredOnActor
				? (-HalfHeight + TileSize * 0.5f + YIndex * TileSize)
				: (TileSize * 0.5f + YIndex * TileSize);

			const FVector Location(X, Y, FloorZOffset);
			if (ShouldCreateTile(Location) == false)
			{
				continue;
			}

			AddFloorTile(XIndex, YIndex, Location, RandomStream);
		}
	}
}

void ASXFloorGenerator::ClearFloor()
{
	if (IsValid(BasicFloorHISM))
	{
		BasicFloorHISM->ClearInstances();
	}

	if (IsValid(DamagedFloorHISM))
	{
		DamagedFloorHISM->ClearInstances();
	}

	if (IsValid(EdgeFloorHISM))
	{
		EdgeFloorHISM->ClearInstances();
	}

	if (IsValid(CornerFloorHISM))
	{
		CornerFloorHISM->ClearInstances();
	}

	if (IsValid(HazardFloorHISM))
	{
		HazardFloorHISM->ClearInstances();
	}
}

void ASXFloorGenerator::AddFloorTile(int32 XIndex, int32 YIndex, const FVector& Location, FRandomStream& RandomStream)
{
	const float Yaw = GetRandomYaw(RandomStream);

	if (bUseCornerTiles && IsCornerTile(XIndex, YIndex) && IsValid(CornerFloorHISM))
	{
		AddTileInstance(CornerFloorHISM, Location, Yaw);
		return;
	}

	if (bUseEdgeTiles && IsEdgeTile(XIndex, YIndex) && IsValid(EdgeFloorHISM))
	{
		AddTileInstance(EdgeFloorHISM, Location, Yaw);
		return;
	}

	if (bEdgesOverrideHazard == false || IsEdgeTile(XIndex, YIndex) == false)
	{
		if (HazardTileChance > 0.0f && IsValid(HazardFloorHISM) && RandomStream.FRand() < HazardTileChance)
		{
			AddTileInstance(HazardFloorHISM, Location, Yaw);
			return;
		}
	}

	if (DamagedTileChance > 0.0f && IsValid(DamagedFloorHISM) && RandomStream.FRand() < DamagedTileChance)
	{
		AddTileInstance(DamagedFloorHISM, Location, Yaw);
		return;
	}

	AddTileInstance(BasicFloorHISM, Location, Yaw);
}

void ASXFloorGenerator::AddTileInstance(UHierarchicalInstancedStaticMeshComponent* TargetComponent, const FVector& Location, float YawDegrees) const
{
	if (IsValid(TargetComponent) == false)
	{
		return;
	}

	FRotator FinalRotation = TileRotation;
	FinalRotation.Yaw += YawDegrees;

	const FTransform InstanceTransform(FinalRotation, Location, TileScale);
	TargetComponent->AddInstance(InstanceTransform);
}

bool ASXFloorGenerator::ShouldCreateTile(const FVector& Location) const
{
	if (FloorShape != ESXFloorShape::Circle)
	{
		return true;
	}

	return FVector2D(Location.X, Location.Y).Size() <= CircleRadius;
}

bool ASXFloorGenerator::IsCornerTile(int32 XIndex, int32 YIndex) const
{
	const bool bMinX = XIndex == 0;
	const bool bMaxX = XIndex == Width - 1;
	const bool bMinY = YIndex == 0;
	const bool bMaxY = YIndex == Height - 1;

	return (bMinX || bMaxX) && (bMinY || bMaxY);
}

bool ASXFloorGenerator::IsEdgeTile(int32 XIndex, int32 YIndex) const
{
	return XIndex == 0 || XIndex == Width - 1 || YIndex == 0 || YIndex == Height - 1;
}

float ASXFloorGenerator::GetRandomYaw(FRandomStream& RandomStream) const
{
	if (bRandomRotation == false)
	{
		return 0.0f;
	}

	return static_cast<float>(RandomStream.RandRange(0, 3) * 90);
}
