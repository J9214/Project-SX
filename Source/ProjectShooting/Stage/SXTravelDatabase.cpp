// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/SXTravelDatabase.h"

bool USXTravelDatabase::FindTravelPoint(FName PointId, FSXTravelPointData& OutPoint) const
{
	if (PointId.IsNone())
	{
		return false;
	}

	for (const FSXTravelPointData& TravelPoint : TravelPoints)
	{
		if (TravelPoint.PointId == PointId)
		{
			OutPoint = TravelPoint;
			return true;
		}
	}

	return false;
}

bool USXTravelDatabase::GetTravelPointTransform(FName PointId, FTransform& OutTransform) const
{
	FSXTravelPointData TravelPoint;
	if (FindTravelPoint(PointId, TravelPoint) == false)
	{
		return false;
	}

	OutTransform = TravelPoint.Transform;
	return true;
}

bool USXTravelDatabase::ResolveDestinationPointId(FName SourcePointId, FName ExplicitDestinationPointId, FName& OutDestinationPointId) const
{
	if (ExplicitDestinationPointId.IsNone() == false)
	{
		OutDestinationPointId = ExplicitDestinationPointId;
		return true;
	}

	if (SourcePointId.IsNone())
	{
		return false;
	}

	for (const FSXTravelRouteData& TravelRoute : TravelRoutes)
	{
		if (TravelRoute.SourcePointId == SourcePointId && TravelRoute.DestinationPointId.IsNone() == false)
		{
			OutDestinationPointId = TravelRoute.DestinationPointId;
			return true;
		}
	}

	return false;
}

TArray<FName> USXTravelDatabase::GetTravelPointIds() const
{
	TArray<FName> PointIds;
	PointIds.Reserve(TravelPoints.Num());

	for (const FSXTravelPointData& TravelPoint : TravelPoints)
	{
		if (TravelPoint.PointId.IsNone() == false)
		{
			PointIds.AddUnique(TravelPoint.PointId);
		}
	}

	return PointIds;
}
