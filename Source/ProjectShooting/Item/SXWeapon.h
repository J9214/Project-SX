// SXWeapon.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXWeapon.generated.h"

class ASXPlayerCharacter;
class ASXAmmoProjectile;
class ASXWeapon;
class USXPickupComponent;
class UAnimMontage;
struct FCollisionQueryParams;
struct FHitResult;

UENUM(BlueprintType)
enum class ESXAmmoType : uint8
{
	Normal,
	Explosive,
	Piercing
};

UENUM(BlueprintType)
enum class ESXAmmoFireMethod : uint8
{
	LineTrace,
	Projectile
};

USTRUCT(BlueprintType)
struct FSXAmmoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo")
	ESXAmmoType AmmoType = ESXAmmoType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo")
	ESXAmmoFireMethod FireMethod = ESXAmmoFireMethod::LineTrace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo", meta=(ClampMin="0.0"))
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo", meta=(ClampMin="1"))
	int32 MagazineSize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo", meta=(ClampMin="0"))
	int32 InitialReserveAmmo = 90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo|Piercing", meta=(ClampMin="1"))
	int32 MaxPiercingHitCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo|Explosive", meta=(ClampMin="0.0", Units=cm))
	float ExplosiveRadius = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo|Projectile", meta=(EditCondition="FireMethod == ESXAmmoFireMethod::Projectile", EditConditionHides))
	TSubclassOf<ASXAmmoProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo|Projectile", meta=(ClampMin="0.0", EditCondition="FireMethod == ESXAmmoFireMethod::Projectile", EditConditionHides, Units="cm/s"))
	float ProjectileSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo|Projectile", meta=(ClampMin="0.1", EditCondition="FireMethod == ESXAmmoFireMethod::Projectile", EditConditionHides, Units=s))
	float ProjectileLifeTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo|Multi Shot", meta=(ClampMin="1"))
	int32 ShotCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo|Multi Shot", meta=(ClampMin="0.0", Units=deg))
	float SpreadAngle = 0.0f;
};

USTRUCT(BlueprintType)
struct FSXAmmoRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo")
	ESXAmmoType AmmoType = ESXAmmoType::Normal;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo")
	int32 AmmoInMagazine = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo")
	int32 ReserveAmmo = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FSXOnAmmoChangedSignature, ASXWeapon*, Weapon, ESXAmmoType, AmmoType, int32, AmmoInMagazine, int32, ReserveAmmo, int32, MagazineSize);

UCLASS()
class PROJECTSHOOTING_API ASXWeapon : public AActor
{
	GENERATED_BODY()

public:
	ASXWeapon();

	USXPickupComponent* GetPickupComponent() const { return PickupComponent; }

	UAnimMontage* GetAttackMontage() const { return AttackMontage; }

	float GetMaxAttackRange() const { return MaxAttackRange; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	ESXAmmoType GetCurrentAmmoType() const { return CurrentAmmoType; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	int32 GetAmmoInMagazine() const { return AmmoInMagazine; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	int32 GetReserveAmmo() const { return ReserveAmmo; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	int32 GetMagazineSize() const;

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	float GetCurrentAmmoDamage() const;

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Ammo")
	void BroadcastAmmoChanged();

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Ammo")
	void SetCurrentAmmoType(ESXAmmoType NewAmmoType);

	void TryFire(ASXPlayerCharacter* InPickUpCharacter);

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Ammo")
	void Reload();

	UPROPERTY(BlueprintAssignable, Category="SX|Weapon|Ammo")
	FSXOnAmmoChangedSignature OnAmmoChanged;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOnPickUp(ASXPlayerCharacter* InPickUpCharacter);

	const FSXAmmoData* GetCurrentAmmoData() const;
	FSXAmmoRuntimeState* GetCurrentAmmoState();
	const FSXAmmoRuntimeState* GetCurrentAmmoState() const;

	bool ConsumeAmmo();
	void ApplyDamageToHit(const FHitResult& HitResult, ASXPlayerCharacter* InPickUpCharacter, float DamageAmount);
	void FireAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& FireDirection, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter);
	void FireLineTraceAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& FireDirection, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter);
	void FireProjectileAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& FireDirection, ASXPlayerCharacter* InPickUpCharacter);
	void FireNormalAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter);
	void FirePiercingAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter);
	void FireExplosiveAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USXPickupComponent> PickupComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Units = cm))
	float MaxAttackRange = 25000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Ammo")
	TArray<FSXAmmoData> AmmoDataList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo")
	ESXAmmoType CurrentAmmoType = ESXAmmoType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo|Piercing", meta=(ClampMin="1"))
	int32 MaxPiercingHitCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo|Explosive", meta=(ClampMin="0.0", Units=cm))
	float ExplosiveRadius = 350.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo")
	int32 AmmoInMagazine = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo")
	int32 ReserveAmmo = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo")
	TArray<FSXAmmoRuntimeState> AmmoRuntimeStates;
};
