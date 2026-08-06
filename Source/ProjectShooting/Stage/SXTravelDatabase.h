// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SXTravelDatabase.generated.h"

USTRUCT(BlueprintType)
struct FSXTravelPointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	FName PointId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	FTransform Transform = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct FSXTravelRouteData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	FName SourcePointId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	FName DestinationPointId = NAME_None;
};

UCLASS(BlueprintType)
class PROJECTSHOOTING_API USXTravelDatabase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="SX|Travel")
	bool FindTravelPoint(FName PointId, FSXTravelPointData& OutPoint) const;

	UFUNCTION(BlueprintPure, Category="SX|Travel")
	bool GetTravelPointTransform(FName PointId, FTransform& OutTransform) const;

	UFUNCTION(BlueprintPure, Category="SX|Travel")
	bool ResolveDestinationPointId(FName SourcePointId, FName ExplicitDestinationPointId, FName& OutDestinationPointId) const;

	UFUNCTION(BlueprintPure, Category="SX|Travel")
	TArray<FName> GetTravelPointIds() const;

	UFUNCTION(BlueprintPure, Category="SX|Travel")
	const TArray<FSXTravelPointData>& GetTravelPoints() const { return TravelPoints; }

	UFUNCTION(BlueprintPure, Category="SX|Travel")
	const TArray<FSXTravelRouteData>& GetTravelRoutes() const { return TravelRoutes; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	TArray<FSXTravelPointData> TravelPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Travel")
	TArray<FSXTravelRouteData> TravelRoutes;
};
