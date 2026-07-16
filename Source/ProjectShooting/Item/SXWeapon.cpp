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
#include "Engine/OverlapResult.h"
#include "GameFramework/PlayerController.h"
#include "Item/SXAmmoProjectile.h"

ASXWeapon::ASXWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupComponent = CreateDefaultSubobject<USXPickupComponent>(TEXT("PickupComponent"));
	SetRootComponent(PickupComponent);
	PickupComponent->SetPickupMethod(ESXPickupMethod::Interaction);

	FSXAmmoData NormalAmmoData;
	NormalAmmoData.AmmoType = ESXAmmoType::Normal;
	NormalAmmoData.FireMethod = ESXAmmoFireMethod::LineTrace;
	NormalAmmoData.Damage = 10.0f;
	NormalAmmoData.MagazineSize = 30;
	NormalAmmoData.InitialReserveAmmo = 90;

	FSXAmmoData ExplosiveAmmoData;
	ExplosiveAmmoData.AmmoType = ESXAmmoType::Explosive;
	ExplosiveAmmoData.FireMethod = ESXAmmoFireMethod::LineTrace;
	ExplosiveAmmoData.Damage = 18.0f;
	ExplosiveAmmoData.MagazineSize = 8;
	ExplosiveAmmoData.InitialReserveAmmo = 16;
	ExplosiveAmmoData.ExplosiveRadius = ExplosiveRadius;

	FSXAmmoData PiercingAmmoData;
	PiercingAmmoData.AmmoType = ESXAmmoType::Piercing;
	PiercingAmmoData.FireMethod = ESXAmmoFireMethod::LineTrace;
	PiercingAmmoData.Damage = 14.0f;
	PiercingAmmoData.MagazineSize = 12;
	PiercingAmmoData.InitialReserveAmmo = 24;
	PiercingAmmoData.MaxPiercingHitCount = MaxPiercingHitCount;

	AmmoDataList = { NormalAmmoData, ExplosiveAmmoData, PiercingAmmoData };
}

void ASXWeapon::BeginPlay()
{
	Super::BeginPlay();

	PickupComponent->OnPickUp.AddDynamic(this, &ThisClass::HandleOnPickUp);

	if (const FSXAmmoData* AmmoData = GetCurrentAmmoData())
	{
		AmmoRuntimeStates.Reset();
		for (const FSXAmmoData& RuntimeAmmoData : AmmoDataList)
		{
			FSXAmmoRuntimeState RuntimeState;
			RuntimeState.AmmoType = RuntimeAmmoData.AmmoType;
			RuntimeState.AmmoInMagazine = RuntimeAmmoData.MagazineSize;
			RuntimeState.ReserveAmmo = RuntimeAmmoData.InitialReserveAmmo;
			AmmoRuntimeStates.Add(RuntimeState);
		}

		AmmoInMagazine = AmmoData->MagazineSize;
		ReserveAmmo = AmmoData->InitialReserveAmmo;
		BroadcastAmmoChanged();
	}
}

void ASXWeapon::HandleOnPickUp(ASXPlayerCharacter* InPickUpCharacter)
{
	if (IsValid(InPickUpCharacter) == false)
	{
		return;
	}

	USkeletalMeshComponent* TargetMesh = InPickUpCharacter->GetMesh();
	if (IsValid(TargetMesh) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon %s pickup failed: player mesh is invalid."), *GetNameSafe(this));
		return;
	}

	PickupComponent->SetSimulatePhysics(false);
	PickupComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorEnableCollision(false);
	SetOwner(InPickUpCharacter);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	const bool bAttached = AttachToComponent(TargetMesh, AttachmentRules, FName(TEXT("Weapon_Socket")));
	UE_LOG(LogTemp, Log, TEXT("Weapon %s pickup by %s. Attached: %s, SocketExists: %s"),
		*GetNameSafe(this),
		*GetNameSafe(InPickUpCharacter),
		bAttached ? TEXT("true") : TEXT("false"),
		TargetMesh->DoesSocketExist(FName(TEXT("Weapon_Socket"))) ? TEXT("true") : TEXT("false"));

	InPickUpCharacter->ClearPickupCandidate(PickupComponent);

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
	if (const FSXAmmoRuntimeState* AmmoState = GetCurrentAmmoState())
	{
		AmmoInMagazine = AmmoState->AmmoInMagazine;
		ReserveAmmo = AmmoState->ReserveAmmo;
	}

	OnAmmoChanged.Broadcast(this, CurrentAmmoType, AmmoInMagazine, ReserveAmmo, GetMagazineSize());
}

void ASXWeapon::SetCurrentAmmoType(ESXAmmoType NewAmmoType)
{
	if (CurrentAmmoType == NewAmmoType)
	{
		return;
	}

	if (AmmoDataList.ContainsByPredicate([NewAmmoType](const FSXAmmoData& AmmoData)
	{
		return AmmoData.AmmoType == NewAmmoType;
	}) == false)
	{
		return;
	}

	CurrentAmmoType = NewAmmoType;
	BroadcastAmmoChanged();
}

void ASXWeapon::Reload()
{
	FSXAmmoRuntimeState* AmmoState = GetCurrentAmmoState();
	if (AmmoState == nullptr)
	{
		return;
	}

	const int32 MagazineSize = GetMagazineSize();
	if (MagazineSize <= 0 || AmmoState->AmmoInMagazine >= MagazineSize || AmmoState->ReserveAmmo <= 0)
	{
		return;
	}

	const int32 NeedAmmo = MagazineSize - AmmoState->AmmoInMagazine;
	const int32 ReloadAmmo = FMath::Min(NeedAmmo, AmmoState->ReserveAmmo);

	AmmoState->AmmoInMagazine += ReloadAmmo;
	AmmoState->ReserveAmmo -= ReloadAmmo;

	BroadcastAmmoChanged();
}

const FSXAmmoData* ASXWeapon::GetCurrentAmmoData() const
{
	return AmmoDataList.FindByPredicate([this](const FSXAmmoData& AmmoData)
	{
		return AmmoData.AmmoType == CurrentAmmoType;
	});
}

FSXAmmoRuntimeState* ASXWeapon::GetCurrentAmmoState()
{
	return AmmoRuntimeStates.FindByPredicate([this](const FSXAmmoRuntimeState& AmmoState)
	{
		return AmmoState.AmmoType == CurrentAmmoType;
	});
}

const FSXAmmoRuntimeState* ASXWeapon::GetCurrentAmmoState() const
{
	return AmmoRuntimeStates.FindByPredicate([this](const FSXAmmoRuntimeState& AmmoState)
	{
		return AmmoState.AmmoType == CurrentAmmoType;
	});
}

bool ASXWeapon::ConsumeAmmo()
{
	FSXAmmoRuntimeState* AmmoState = GetCurrentAmmoState();
	if (AmmoState == nullptr || AmmoState->AmmoInMagazine <= 0)
	{
		return false;
	}

	--AmmoState->AmmoInMagazine;
	BroadcastAmmoChanged();
	return true;
}

void ASXWeapon::ApplyDamageToHit(const FHitResult& HitResult, ASXPlayerCharacter* InPickUpCharacter, float DamageAmount)
{
	ASXCharacterBase* HittedCharacter = Cast<ASXCharacterBase>(HitResult.GetActor());
	if (IsValid(HittedCharacter) == true)
	{
		FDamageEvent DamageEvent;
		UE_LOG(LogTemp, Log, TEXT("Apply %s ammo damage %.1f to %s"),
			*UEnum::GetValueAsString(CurrentAmmoType),
			DamageAmount,
			*GetNameSafe(HittedCharacter));
		HittedCharacter->TakeDamage(DamageAmount, DamageEvent, InPickUpCharacter->GetController(), this);
	}
	else if (IsValid(HitResult.GetActor()) == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit actor is not SXCharacterBase: %s"), *GetNameSafe(HitResult.GetActor()));
	}
}

void ASXWeapon::FireAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& FireDirection, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter)
{
	switch (AmmoData.FireMethod)
	{
	case ESXAmmoFireMethod::Projectile:
		FireProjectileAmmo(AmmoData, StartLocation, FireDirection, InPickUpCharacter);
		break;
	case ESXAmmoFireMethod::LineTrace:
	default:
		FireLineTraceAmmo(AmmoData, StartLocation, FireDirection, TraceParams, InPickUpCharacter);
		break;
	}
}

void ASXWeapon::FireLineTraceAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& FireDirection, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter)
{
	const FVector EndLocation = StartLocation + FireDirection * GetMaxAttackRange();

	switch (AmmoData.AmmoType)
	{
	case ESXAmmoType::Explosive:
		FireExplosiveAmmo(AmmoData, StartLocation, EndLocation, TraceParams, InPickUpCharacter);
		break;
	case ESXAmmoType::Piercing:
		FirePiercingAmmo(AmmoData, StartLocation, EndLocation, TraceParams, InPickUpCharacter);
		break;
	case ESXAmmoType::Normal:
	default:
		FireNormalAmmo(AmmoData, StartLocation, EndLocation, TraceParams, InPickUpCharacter);
		break;
	}
}

void ASXWeapon::FireProjectileAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& FireDirection, ASXPlayerCharacter* InPickUpCharacter)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	TSubclassOf<ASXAmmoProjectile> ProjectileClass = AmmoData.ProjectileClass;
	if (ProjectileClass == nullptr)
	{
		ProjectileClass = ASXAmmoProjectile::StaticClass();
	}

	const FRotator SpawnRotation = FireDirection.Rotation();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = InPickUpCharacter;

	ASXAmmoProjectile* Projectile = World->SpawnActor<ASXAmmoProjectile>(ProjectileClass, StartLocation, SpawnRotation, SpawnParameters);
	if (IsValid(Projectile) == false)
	{
		return;
	}

	Projectile->InitializeProjectile(AmmoData.AmmoType, AmmoData.Damage, AmmoData.ExplosiveRadius, AmmoData.MaxPiercingHitCount, AmmoData.ProjectileLifeTime, this, InPickUpCharacter);
	Projectile->SetProjectileSpeed(AmmoData.ProjectileSpeed);

	if (2 == ASXCharacterBase::ShowAttackRangedDebug)
	{
		DrawDebugLine(GetWorld(), StartLocation, StartLocation + FireDirection * 500.0f, FColor::Purple, false, 5.0f, 0, 2.0f);
	}
}

void ASXWeapon::FireNormalAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter)
{
	FHitResult HitResult;
	const bool bIsCollided = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, TraceParams);
	if (bIsCollided == false)
	{
		HitResult.TraceStart = StartLocation;
		HitResult.TraceEnd = EndLocation;
	}

	if (2 == ASXCharacterBase::ShowAttackRangedDebug)
	{
		const FVector DebugEndLocation = bIsCollided ? HitResult.ImpactPoint : EndLocation;
		DrawDebugSphere(GetWorld(), StartLocation, 2.f, 16, FColor::Red, false, 60.f);
		DrawDebugSphere(GetWorld(), DebugEndLocation, 2.f, 16, FColor::Green, false, 60.f);
		DrawDebugLine(GetWorld(), StartLocation, DebugEndLocation, FColor::Blue, false, 60.f, 0, 2.f);
	}

	if (bIsCollided == true)
	{
		UE_LOG(LogTemp, Log, TEXT("Weapon hit actor: %s, component: %s"),
			*GetNameSafe(HitResult.GetActor()),
			*GetNameSafe(HitResult.GetComponent()));

		ApplyDamageToHit(HitResult, InPickUpCharacter, AmmoData.Damage);
	}
}

void ASXWeapon::FirePiercingAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter)
{
	if (2 == ASXCharacterBase::ShowAttackRangedDebug)
	{
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Cyan, false, 60.f, 0, 2.f);
	}

	FCollisionQueryParams PiercingTraceParams = TraceParams;
	TSet<AActor*> DamagedActors;
	int32 DamagedCount = 0;

	while (DamagedCount < AmmoData.MaxPiercingHitCount)
	{
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, PiercingTraceParams) == false)
		{
			break;
		}

		AActor* HitActor = HitResult.GetActor();
		ASXCharacterBase* HittedCharacter = Cast<ASXCharacterBase>(HitActor);
		if (IsValid(HittedCharacter) == false)
		{
			break;
		}

		if (DamagedActors.Contains(HitActor))
		{
			PiercingTraceParams.AddIgnoredActor(HitActor);
			continue;
		}

		ApplyDamageToHit(HitResult, InPickUpCharacter, AmmoData.Damage);
		DamagedActors.Add(HitActor);
		PiercingTraceParams.AddIgnoredActor(HitActor);
		++DamagedCount;

		if (2 == ASXCharacterBase::ShowAttackRangedDebug)
		{
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 8.f, 16, FColor::Cyan, false, 60.f);
		}

	}
}

void ASXWeapon::FireExplosiveAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter)
{
	FHitResult HitResult;
	const bool bIsCollided = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, TraceParams);
	if (bIsCollided == false)
	{
		if (2 == ASXCharacterBase::ShowAttackRangedDebug)
		{
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Orange, false, 60.f, 0, 2.f);
		}
		return;
	}

	const FVector ExplosionLocation = HitResult.ImpactPoint;
	if (2 == ASXCharacterBase::ShowAttackRangedDebug)
	{
		DrawDebugLine(GetWorld(), StartLocation, ExplosionLocation, FColor::Orange, false, 60.f, 0, 2.f);
		DrawDebugSphere(GetWorld(), ExplosionLocation, AmmoData.ExplosiveRadius, 32, FColor::Orange, false, 60.f);
	}

	FCollisionShape ExplosionShape = FCollisionShape::MakeSphere(AmmoData.ExplosiveRadius);
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams OverlapParams(NAME_None, false, this);
	OverlapParams.AddIgnoredActor(this);
	OverlapParams.AddIgnoredActor(InPickUpCharacter);

	const bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ExplosionLocation,
		FQuat::Identity,
		ECC_Pawn,
		ExplosionShape,
		OverlapParams
	);

	if (bHasOverlaps == false)
	{
		return;
	}

	TSet<AActor*> DamagedActors;
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		ASXCharacterBase* HittedCharacter = Cast<ASXCharacterBase>(OverlapResult.GetActor());
		if (IsValid(HittedCharacter) == false || DamagedActors.Contains(HittedCharacter))
		{
			continue;
		}

		FDamageEvent DamageEvent;
		const float DamageAmount = AmmoData.Damage;
		UE_LOG(LogTemp, Log, TEXT("Apply explosive damage %.1f to %s"), DamageAmount, *GetNameSafe(HittedCharacter));
		HittedCharacter->TakeDamage(DamageAmount, DamageEvent, InPickUpCharacter->GetController(), this);
		DamagedActors.Add(HittedCharacter);
	}
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
		const FSXAmmoData* AmmoData = GetCurrentAmmoData();
		if (AmmoData == nullptr)
		{
			return;
		}

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

		FVector BulletDirection = TargetTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
		FVector StartLocation = WeaponMuzzleLocation;

		FCollisionQueryParams TraceParams(NAME_None, false, this);
		TraceParams.AddIgnoredActor(this);
		TraceParams.AddIgnoredActor(InPickUpCharacter);

		FireAmmo(*AmmoData, StartLocation, BulletDirection, TraceParams, InPickUpCharacter);

#pragma endregion

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
