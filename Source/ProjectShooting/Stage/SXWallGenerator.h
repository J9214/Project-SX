// SXWallGenerator.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXWallGenerator.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class ESXWallEntranceSide : uint8
{
	North,
	South,
	East,
	West
};

USTRUCT(BlueprintType)
struct FSXWallEntranceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Wall")
	ESXWallEntranceSide Side = ESXWallEntranceSide::South;

	// 0 means the center segment of the selected side.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Wall", meta=(ClampMin="0"))
	int32 SegmentIndex = 0;

	// How many wall segments this entrance occupies.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Wall", meta=(ClampMin="1"))
	int32 SegmentWidth = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Wall")
	bool bUseEntranceWallMesh = true;
};

USTRUCT(BlueprintType)
struct FSXWallInstanceTransformSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Wall")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Wall")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Wall")
	FVector Scale = FVector::OneVector;
};

USTRUCT(BlueprintType)
struct FSXWallCornerTransformSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Wall", meta=(Units=deg))
	float BaseYaw = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SX|Wall")
	FSXWallInstanceTransformSettings Transform;
};

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXWallGenerator : public AActor
{
	GENERATED_BODY()

public:
	ASXWallGenerator();

	UFUNCTION(BlueprintCallable, Category="SX|Wall Generator")
	void RebuildWalls();

	UFUNCTION(BlueprintCallable, Category="SX|Wall Generator")
	void ClearWalls();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	void BuildHorizontalSide(bool bNorth);
	void BuildVerticalSide(bool bEast);
	void AddWallInstance(UHierarchicalInstancedStaticMeshComponent* TargetComponent, const FSXWallInstanceTransformSettings& TransformSettings, const FVector& Location, float YawDegrees) const;
	void AddCornerWallInstance(const FSXWallCornerTransformSettings& CornerTransformSettings, const FVector& Location) const;
	bool IsEntranceSegment(ESXWallEntranceSide Side, int32 SegmentIndex, int32 SegmentCount, const FSXWallEntranceData*& OutEntranceData) const;
	int32 ResolveEntranceStartIndex(const FSXWallEntranceData& EntranceData, int32 SegmentCount) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> StraightWallHISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> CornerWallHISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> EntranceWallHISM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator", meta=(ClampMin="1"))
	int32 WidthSegments = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator", meta=(ClampMin="1"))
	int32 HeightSegments = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator", meta=(ClampMin="1.0", Units=cm))
	float SegmentLength = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator", meta=(Units=cm))
	float WallZOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator")
	bool bCenteredOnActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator")
	bool bGenerateOnConstruction = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator")
	bool bUseCornerWallMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator")
	bool bUseEntranceWallMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator|Transform")
	FSXWallInstanceTransformSettings StraightWallTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator|Transform")
	FSXWallInstanceTransformSettings CornerWallTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator|Transform")
	FSXWallInstanceTransformSettings EntranceWallTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator|Transform|Corner")
	FSXWallCornerTransformSettings SouthWestCornerTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator|Transform|Corner")
	FSXWallCornerTransformSettings SouthEastCornerTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator|Transform|Corner")
	FSXWallCornerTransformSettings NorthWestCornerTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator|Transform|Corner")
	FSXWallCornerTransformSettings NorthEastCornerTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Wall Generator")
	TArray<FSXWallEntranceData> Entrances;
};
