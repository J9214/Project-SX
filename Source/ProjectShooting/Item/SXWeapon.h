// SXWeapon.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SXWeapon.generated.h"

class ASXPlayerCharacter;
class ASXWeapon;
class USXPickupComponent;
class UAnimMontage;

UENUM(BlueprintType)
enum class ESXAmmoType : uint8
{
	Normal,
	Explosive,
	Piercing
};

USTRUCT(BlueprintType)
struct FSXAmmoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo")
	ESXAmmoType AmmoType = ESXAmmoType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo", meta=(ClampMin="0.0"))
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo", meta=(ClampMin="1"))
	int32 MagazineSize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Ammo", meta=(ClampMin="0"))
	int32 InitialReserveAmmo = 90;
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

	bool ConsumeAmmo();

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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo")
	int32 AmmoInMagazine = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Ammo")
	int32 ReserveAmmo = 0;
};
