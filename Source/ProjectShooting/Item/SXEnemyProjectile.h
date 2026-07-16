// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXEnemyProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class PROJECTSHOOTING_API ASXEnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASXEnemyProjectile();

	UFUNCTION(BlueprintCallable, Category="SX|Enemy|Projectile")
	void InitializeProjectile(float InDamage, AActor* InDamageCauser);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Enemy|Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Enemy|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Projectile", meta=(ClampMin="0.0"))
	float Damage = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Enemy|Projectile", meta=(ClampMin="0.1", Units=s))
	float LifeTime = 5.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Enemy|Projectile")
	TObjectPtr<AActor> DamageCauser;
};
