// SXFloorGenerator.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXFloorGenerator.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class ESXFloorShape : uint8
{
	Rectangle,
	Circle
};

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXFloorGenerator : public AActor
{
	GENERATED_BODY()

public:
	ASXFloorGenerator();

	UFUNCTION(BlueprintCallable, Category="SX|Floor Generator")
	void RebuildFloor();

	UFUNCTION(BlueprintCallable, Category="SX|Floor Generator")
	void ClearFloor();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	void AddFloorTile(int32 XIndex, int32 YIndex, const FVector& Location, FRandomStream& RandomStream);
	void AddTileInstance(UHierarchicalInstancedStaticMeshComponent* TargetComponent, const FVector& Location, float YawDegrees) const;
	bool ShouldCreateTile(const FVector& Location) const;
	bool IsCornerTile(int32 XIndex, int32 YIndex) const;
	bool IsEdgeTile(int32 XIndex, int32 YIndex) const;
	float GetRandomYaw(FRandomStream& RandomStream) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> BasicFloorHISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> DamagedFloorHISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> EdgeFloorHISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> CornerFloorHISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> HazardFloorHISM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator", meta=(ClampMin="1"))
	int32 Width = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator", meta=(ClampMin="1"))
	int32 Height = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator", meta=(ClampMin="1.0", Units=cm))
	float TileSize = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator", meta=(Units=cm))
	float FloorZOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator|Transform")
	FRotator TileRotation = FRotator(90.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator|Transform")
	FVector TileScale = FVector(3.5f, 3.5f, 3.65f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator")
	bool bCenteredOnActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator")
	bool bGenerateOnConstruction = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator")
	ESXFloorShape FloorShape = ESXFloorShape::Rectangle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator", meta=(ClampMin="0.0", Units=cm, EditCondition="FloorShape == ESXFloorShape::Circle", EditConditionHides))
	float CircleRadius = 4200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator|Random")
	int32 RandomSeed = 1001;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator|Random")
	bool bRandomRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator|Random", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DamagedTileChance = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator|Random", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HazardTileChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator|Edge")
	bool bUseEdgeTiles = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator|Edge")
	bool bUseCornerTiles = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Floor Generator|Edge")
	bool bEdgesOverrideHazard = true;
};
