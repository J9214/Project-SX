// SXWeapon.cpp


#include "Item/SXWeapon.h"

#include "Animation/AnimInstance.h"
#include "Character/SXCharacterBase.h"
#include "Components/SXPickupComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/SXPlayerCharacter.h"
#include "Controller/SXPlayerController.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/PlayerController.h"

ASXWeapon::ASXWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupComponent = CreateDefaultSubobject<USXPickupComponent>(TEXT("PickupComponent"));
	SetRootComponent(PickupComponent);

	FSXAmmoData NormalAmmoData;
	NormalAmmoData.AmmoType = ESXAmmoType::Normal;
	NormalAmmoData.Damage = 10.0f;
	NormalAmmoData.MagazineSize = 30;
	NormalAmmoData.InitialReserveAmmo = 90;

	FSXAmmoData ExplosiveAmmoData;
	ExplosiveAmmoData.AmmoType = ESXAmmoType::Explosive;
	ExplosiveAmmoData.Damage = 18.0f;
	ExplosiveAmmoData.MagazineSize = 8;
	ExplosiveAmmoData.InitialReserveAmmo = 16;

	FSXAmmoData PiercingAmmoData;
	PiercingAmmoData.AmmoType = ESXAmmoType::Piercing;
	PiercingAmmoData.Damage = 14.0f;
	PiercingAmmoData.MagazineSize = 12;
	PiercingAmmoData.InitialReserveAmmo = 24;

	AmmoDataList = { NormalAmmoData, ExplosiveAmmoData, PiercingAmmoData };
}

void ASXWeapon::BeginPlay()
{
	Super::BeginPlay();

	PickupComponent->OnPickUp.AddDynamic(this, &ThisClass::HandleOnPickUp);

	if (const FSXAmmoData* AmmoData = GetCurrentAmmoData())
	{
		AmmoInMagazine = AmmoData->MagazineSize;
		ReserveAmmo = AmmoData->InitialReserveAmmo;
		BroadcastAmmoChanged();
	}
}

void ASXWeapon::HandleOnPickUp(ASXPlayerCharacter* InPickUpCharacter)
{
	//UE_LOG(LogTemp, Error, TEXT("HandleOnPickUp(%s)"), *InPickUpCharacter->GetName());

	if (IsValid(InPickUpCharacter) == false)
	{
		return;
	}

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(InPickUpCharacter->GetMesh(), AttachmentRules, FName(TEXT("Weapon_Socket")));
	//PickupComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorEnableCollision(false);
	PickupComponent->SetSimulatePhysics(false);

	InPickUpCharacter->SetCurrentWeapon(this);
}

int32 ASXWeapon::GetMagazineSize() const
{
	if (const FSXAmmoData* AmmoData = GetCurrentAmmoData())
	{
		return AmmoData->MagazineSize;
	}

	return 0;
}

float ASXWeapon::GetCurrentAmmoDamage() const
{
	if (const FSXAmmoData* AmmoData = GetCurrentAmmoData())
	{
		return AmmoData->Damage;
	}

	return 0.0f;
}

void ASXWeapon::BroadcastAmmoChanged()
{
	OnAmmoChanged.Broadcast(this, CurrentAmmoType, AmmoInMagazine, ReserveAmmo, GetMagazineSize());
}

void ASXWeapon::Reload()
{
	const int32 MagazineSize = GetMagazineSize();
	if (MagazineSize <= 0 || AmmoInMagazine >= MagazineSize || ReserveAmmo <= 0)
	{
		return;
	}

	const int32 NeedAmmo = MagazineSize - AmmoInMagazine;
	const int32 ReloadAmmo = FMath::Min(NeedAmmo, ReserveAmmo);

	AmmoInMagazine += ReloadAmmo;
	ReserveAmmo -= ReloadAmmo;

	BroadcastAmmoChanged();
}

const FSXAmmoData* ASXWeapon::GetCurrentAmmoData() const
{
	return AmmoDataList.FindByPredicate([this](const FSXAmmoData& AmmoData)
	{
		return AmmoData.AmmoType == CurrentAmmoType;
	});
}

bool ASXWeapon::ConsumeAmmo()
{
	if (AmmoInMagazine <= 0)
	{
		return false;
	}

	--AmmoInMagazine;
	BroadcastAmmoChanged();
	return true;
}

void ASXWeapon::TryFire(ASXPlayerCharacter* InPickUpCharacter)
{
	if (IsValid(InPickUpCharacter) == false)
	{
		return;
	}

	APlayerController* PlayerController = InPickUpCharacter->GetController<APlayerController>();
	if (IsValid(PlayerController) == true)
	{
		if (ConsumeAmmo() == false)
		{
			return;
		}

#pragma region CaculateTargetTransform
		float FocalDistance = 400.f;
		FVector FocalLocation;
		FVector CameraLocation;
		FRotator CameraRotation;

		PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

		FVector AimDirectionFromCamera = CameraRotation.Vector().GetSafeNormal();
		FocalLocation = CameraLocation + (AimDirectionFromCamera * FocalDistance);

		FVector WeaponMuzzleLocation = GetPickupComponent()->GetSocketLocation(TEXT("MuzzleFlash"));
		FVector FinalFocalLocation = FocalLocation + (((WeaponMuzzleLocation - FocalLocation) | AimDirectionFromCamera) * AimDirectionFromCamera);

		FTransform TargetTransform = FTransform(CameraRotation, FinalFocalLocation);

		if (1 == ASXCharacterBase::ShowAttackRangedDebug)
		{
			DrawDebugSphere(GetWorld(), WeaponMuzzleLocation, 2.f, 16, FColor::Red, false, 60.f);

			DrawDebugSphere(GetWorld(), CameraLocation, 2.f, 16, FColor::Yellow, false, 60.f);

			DrawDebugSphere(GetWorld(), FinalFocalLocation, 2.f, 16, FColor::Magenta, false, 60.f);

			// (WeaponLoc - FocalLoc)
			DrawDebugLine(GetWorld(), FocalLocation, WeaponMuzzleLocation, FColor::Yellow, false, 60.f, 0, 2.f);

			// AimDir
			DrawDebugLine(GetWorld(), CameraLocation, FinalFocalLocation, FColor::Blue, false, 60.f, 0, 2.f);

			// Project Direction Line
			DrawDebugLine(GetWorld(), WeaponMuzzleLocation, FinalFocalLocation, FColor::Red, false, 60.f, 0, 2.f);
		}

#pragma endregion

#pragma region PerformLineTracing

		FVector BulletDirection = TargetTransform.GetUnitAxis(EAxis::X);
		FVector StartLocation = WeaponMuzzleLocation;
		FVector EndLocation = TargetTransform.GetLocation() + BulletDirection * GetMaxAttackRange();

		FHitResult HitResult;
		FCollisionQueryParams TraceParams(NAME_None, false, this);
		TraceParams.AddIgnoredActor(this);
		TraceParams.AddIgnoredActor(InPickUpCharacter);

		bool IsCollided = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, TraceParams);
		if (IsCollided == false)
		{
			HitResult.TraceStart = StartLocation;
			HitResult.TraceEnd = EndLocation;
		}

		if (2 == ASXCharacterBase::ShowAttackRangedDebug)
		{
			if (IsCollided == true)
			{
				DrawDebugSphere(GetWorld(), StartLocation, 2.f, 16, FColor::Red, false, 60.f);

				DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 2.f, 16, FColor::Green, false, 60.f);

				DrawDebugLine(GetWorld(), StartLocation, HitResult.ImpactPoint, FColor::Blue, false, 60.f, 0, 2.f);
			}
			else
			{
				DrawDebugSphere(GetWorld(), StartLocation, 2.f, 16, FColor::Red, false, 60.f);

				DrawDebugSphere(GetWorld(), EndLocation, 2.f, 16, FColor::Green, false, 60.f);

				DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, 60.f, 0, 2.f);
			}
		}

#pragma endregion

		if (IsCollided == true)
		{
			ASXCharacterBase* HittedCharacter = Cast<ASXCharacterBase>(HitResult.GetActor());
			if (IsValid(HittedCharacter) == true)
			{
				FDamageEvent DamageEvent;
				HittedCharacter->TakeDamage(GetCurrentAmmoDamage(), DamageEvent, InPickUpCharacter->GetController(), this);
			}
		}

		UAnimInstance* AnimInstance = InPickUpCharacter->GetMesh()->GetAnimInstance();
		UAnimMontage* CurrentAttackMontage = GetAttackMontage();
		if (IsValid(AnimInstance) == true && IsValid(CurrentAttackMontage) == true)
		{
			if (AnimInstance->Montage_IsPlaying(CurrentAttackMontage) == false)
			{
				AnimInstance->Montage_Play(CurrentAttackMontage);
			}
		}

		if (IsValid(InPickUpCharacter->AttackFireCameraShake) == true)
		{
			PlayerController->ClientStartCameraShake(InPickUpCharacter->AttackFireCameraShake);
		}
	}
}
