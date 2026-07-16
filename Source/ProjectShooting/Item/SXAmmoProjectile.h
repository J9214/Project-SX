// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/SXWeapon.h"
#include "SXAmmoProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS(Blueprintable)
class PROJECTSHOOTING_API ASXAmmoProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASXAmmoProjectile();

	UFUNCTION(BlueprintCallable, Category="SX|Ammo|Projectile")
	void InitializeProjectile(ESXAmmoType InAmmoType, float InDamage, float InExplosiveRadius, int32 InMaxPiercingHitCount, float InLifeTime, AActor* InDamageCauser, AActor* InSourceActor);

	UFUNCTION(BlueprintCallable, Category="SX|Ammo|Projectile")
	void SetProjectileSpeed(float InProjectileSpeed);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ApplyExplosionDamage(const FVector& ExplosionLocation);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Ammo|Projectile")
	ESXAmmoType AmmoType = ESXAmmoType::Normal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Ammo|Projectile", meta=(ClampMin="0.0"))
	float Damage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Ammo|Projectile", meta=(ClampMin="0.0", Units=cm))
	float ExplosiveRadius = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Ammo|Projectile", meta=(ClampMin="1"))
	int32 MaxPiercingHitCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Ammo|Projectile", meta=(ClampMin="0.1", Units=s))
	float LifeTime = 5.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo|Projectile")
	int32 CurrentPiercingHitCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo|Projectile")
	TObjectPtr<AActor> DamageCauser;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo|Projectile")
	TObjectPtr<AActor> SourceActor;
};
