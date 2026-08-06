// SXWeapon.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXWeapon.generated.h"

class ASXPlayerCharacter;
class ASXAmmoProjectile;
class ASXWeapon;
class USXPickupComponent;
class USXInventoryComponent;
class UAnimMontage;
class UCameraShakeBase;
class UMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USphereComponent;
class USkeletalMeshComponent;
class USoundBase;
class UStaticMeshComponent;
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

UENUM(BlueprintType)
enum class ESXWeaponType : uint8
{
	Unarmed,
	Pistol,
	Rifle,
	Shotgun,
	Heavy
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo|FX")
	TObjectPtr<UNiagaraSystem> TracerEffect;
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

	UAnimMontage* GetReloadMontage() const { return ReloadMontage; }

	float GetMaxAttackRange() const { return MaxAttackRange; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Visual")
	UMeshComponent* GetWeaponVisualMesh() const;

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Visual")
	FName GetMuzzleSocketName() const { return MuzzleSocketName; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Visual")
	FName GetCharacterAttachSocketName() const { return CharacterAttachSocketName; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Visual")
	FName GetLegacyCharacterAttachSocketName() const { return LegacyCharacterAttachSocketName; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Visual")
	FTransform GetMuzzleTransform() const;

	UFUNCTION(BlueprintPure, Category="SX|Weapon")
	ESXWeaponType GetWeaponType() const { return WeaponType; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Fire Mode")
	bool CanUseFullAuto() const { return bAllowFullAuto; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Fire Mode")
	bool IsFullAutoEnabled() const { return bAllowFullAuto && bFullAutoEnabled; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Fire Mode")
	float GetFireInterval() const { return FireInterval; }

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Fire Mode")
	bool SetFullAutoEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Fire Mode")
	bool ToggleFullAuto();

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	ESXAmmoType GetCurrentAmmoType() const { return CurrentAmmoType; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	int32 GetAmmoInMagazine() const { return AmmoInMagazine; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	int32 GetReserveAmmo() const;

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	bool HasInfiniteReserveAmmo() const { return bInfiniteReserveAmmo; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	int32 GetMagazineSize() const;

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	float GetCurrentAmmoDamage() const;

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Ammo")
	void BroadcastAmmoChanged();

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Ammo", meta=(DeprecatedFunction, DeprecationMessage="Ammo type is now fixed per weapon. Set the weapon's CurrentAmmoType/Weapon Ammo Type in defaults instead."))
	void SetCurrentAmmoType(ESXAmmoType NewAmmoType);

	UFUNCTION(BlueprintCallable, Category="SX|Weapon")
	bool EquipToCharacter(ASXPlayerCharacter* InPickUpCharacter, bool bGrantInitialAmmo = true);

	void SetGrantInitialReserveAmmoOnPickup(bool bInGrantInitialReserveAmmoOnPickup) { bGrantInitialReserveAmmoOnPickup = bInGrantInitialReserveAmmoOnPickup; }

	void TryFire(ASXPlayerCharacter* InPickUpCharacter);

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Ammo")
	void Reload();

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Ammo")
	bool StartReload(ASXPlayerCharacter* InPickUpCharacter);

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Ammo")
	void CompleteReload();

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Ammo")
	void FinishReload();

	UFUNCTION(BlueprintCallable, Category="SX|Weapon|Ammo")
	void CancelReload();

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	bool CanReload() const;

	UFUNCTION(BlueprintPure, Category="SX|Weapon|Ammo")
	bool IsReloading() const { return bIsReloading; }

	UPROPERTY(BlueprintAssignable, Category="SX|Weapon|Ammo")
	FSXOnAmmoChangedSignature OnAmmoChanged;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOnPickUp(ASXPlayerCharacter* InPickUpCharacter);

	UFUNCTION()
	void HandleInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	const FSXAmmoData* GetCurrentAmmoData() const;
	FSXAmmoRuntimeState* GetCurrentAmmoState();
	const FSXAmmoRuntimeState* GetCurrentAmmoState() const;

	USXInventoryComponent* GetOwnerInventoryComponent() const;
	void GrantInitialReserveAmmoTo(ASXPlayerCharacter* InPickUpCharacter);
	bool ConsumeAmmo();
	float ApplyDamageToHit(const FHitResult& HitResult, ASXPlayerCharacter* InPickUpCharacter, float DamageAmount);
	void FireAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& FireDirection, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter);
	void FireLineTraceAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& FireDirection, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter);
	void FireProjectileAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& FireDirection, ASXPlayerCharacter* InPickUpCharacter);
	void FireNormalAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter);
	void FirePiercingAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter);
	void FireExplosiveAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter);
	void PlayFireFeedback(ASXPlayerCharacter* InPickUpCharacter, const FVector& MuzzleLocation, const FRotator& MuzzleRotation);
	void PlayReloadSound();
	void SpawnTracerEffect(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation) const;
	void SpawnImpactEffect(const FHitResult& HitResult) const;
	void HandleReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|Pickup")
	TObjectPtr<USXPickupComponent> PickupComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Weapon|Pickup")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Weapon|Pickup", meta=(ClampMin="0.0", Units=cm))
	float InteractionRadius = 140.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon")
	ESXWeaponType WeaponType = ESXWeaponType::Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Weapon|Fire Mode")
	bool bAllowFullAuto = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Weapon|Fire Mode", meta=(EditCondition="bAllowFullAuto"))
	bool bStartFullAutoEnabled = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Weapon|Fire Mode")
	bool bFullAutoEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Weapon|Fire Mode", meta=(ClampMin="0.01", Units=s))
	float FireInterval = 0.12f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Weapon|Visual")
	TObjectPtr<USkeletalMeshComponent> SkeletalWeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Weapon|Visual")
	TObjectPtr<UStaticMeshComponent> StaticWeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Weapon|Visual")
	FName CharacterAttachSocketName = TEXT("WeaponSocket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Weapon|Visual")
	FName LegacyCharacterAttachSocketName = TEXT("Weapon_Socket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Weapon|Visual")
	FName MuzzleSocketName = TEXT("MuzzleSocket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Weapon|Visual")
	FName LegacyMuzzleSocketName = TEXT("MuzzleFlash");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|Animation")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|FX")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|FX")
	TObjectPtr<UNiagaraSystem> DefaultTracerEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|FX")
	FName TracerStartParameterName = TEXT("User.BeamStart");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|FX")
	FName TracerEndParameterName = TEXT("User.BeamEnd");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|FX", meta=(ClampMin="0.0", Units=s))
	float TracerLifeTime = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|Sound")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|Sound")
	TObjectPtr<USoundBase> ReloadSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Weapon|Camera")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Units = cm))
	float MaxAttackRange = 25000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Ammo")
	TArray<FSXAmmoData> AmmoDataList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo", meta=(DisplayName="Weapon Ammo Type"))
	ESXAmmoType CurrentAmmoType = ESXAmmoType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo")
	bool bLockAmmoTypeToWeapon = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Ammo", meta=(DisplayName="Infinite Reserve Ammo", ToolTip="Reloads do not consume inventory ammo. Reserve ammo is reported as -1 so UI can display an infinity symbol."))
	bool bInfiniteReserveAmmo = false;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo")
	bool bGrantInitialReserveAmmoOnPickup = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo")
	bool bHasGrantedInitialReserveAmmo = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Weapon|Reload")
	bool bIsReloading = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Weapon|Reload")
	bool bReloadAmmoTransferred = false;

	UPROPERTY(Transient)
	TObjectPtr<ASXPlayerCharacter> ReloadingCharacter;
};
