// SXWeapon.cpp


#include "Item/SXWeapon.h"

#include "Animation/AnimInstance.h"
#include "Character/SXCharacterBase.h"
#include "Components/SXPickupComponent.h"
#include "Components/SXInventoryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/SXPlayerCharacter.h"
#include "Controller/SXPlayerController.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/PlayerController.h"
#include "Item/SXAmmoProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

ASXWeapon::ASXWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupComponent = CreateDefaultSubobject<USXPickupComponent>(TEXT("PickupComponent"));
	SetRootComponent(PickupComponent);
	PickupComponent->SetPickupMethod(ESXPickupMethod::Interaction);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(PickupComponent);
	InteractionSphere->SetSphereRadius(InteractionRadius);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);

	SkeletalWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalWeaponMesh"));
	SkeletalWeaponMesh->SetupAttachment(PickupComponent);
	SkeletalWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StaticWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticWeaponMesh"));
	StaticWeaponMesh->SetupAttachment(PickupComponent);
	StaticWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	bFullAutoEnabled = bAllowFullAuto && bStartFullAutoEnabled;

	PickupComponent->OnPickUp.AddDynamic(this, &ThisClass::HandleOnPickUp);
	InteractionSphere->SetSphereRadius(InteractionRadius);
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleInteractionSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleInteractionSphereEndOverlap);

	const bool bHasCurrentAmmoData = AmmoDataList.ContainsByPredicate([this](const FSXAmmoData& AmmoData)
	{
		return AmmoData.AmmoType == CurrentAmmoType;
	});

	if (bHasCurrentAmmoData == false && AmmoDataList.IsEmpty() == false)
	{
		const ESXAmmoType PreviousAmmoType = CurrentAmmoType;
		CurrentAmmoType = AmmoDataList[0].AmmoType;
		UE_LOG(LogTemp, Warning, TEXT("Weapon %s has no ammo data for %s. Falling back to %s."),
			*GetNameSafe(this),
			*UEnum::GetValueAsString(PreviousAmmoType),
			*UEnum::GetValueAsString(CurrentAmmoType));
	}

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

bool ASXWeapon::SetFullAutoEnabled(bool bEnabled)
{
	if (bAllowFullAuto == false)
	{
		bFullAutoEnabled = false;
		return false;
	}

	bFullAutoEnabled = bEnabled;
	return true;
}

bool ASXWeapon::ToggleFullAuto()
{
	if (bAllowFullAuto == false)
	{
		bFullAutoEnabled = false;
		return false;
	}

	bFullAutoEnabled = !bFullAutoEnabled;
	return true;
}

void ASXWeapon::HandleOnPickUp(ASXPlayerCharacter* InPickUpCharacter)
{
	EquipToCharacter(InPickUpCharacter);
}

bool ASXWeapon::EquipToCharacter(ASXPlayerCharacter* InPickUpCharacter, bool bGrantInitialAmmo)
{
	if (IsValid(InPickUpCharacter) == false)
	{
		return false;
	}

	USkeletalMeshComponent* TargetMesh = InPickUpCharacter->FindWeaponAttachMesh(CharacterAttachSocketName, LegacyCharacterAttachSocketName);
	if (IsValid(TargetMesh) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon %s pickup failed: player mesh is invalid."), *GetNameSafe(this));
		return false;
	}

	PickupComponent->DisablePickupBehavior();
	PickupComponent->SetSimulatePhysics(false);
	PickupComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionSphere->SetGenerateOverlapEvents(false);
	SetActorEnableCollision(false);
	SetOwner(InPickUpCharacter);

	if (bGrantInitialAmmo)
	{
		if (IsValid(InPickUpCharacter->GetInventoryComponent()))
		{
			int32 SlotIndex = INDEX_NONE;
			TSubclassOf<ASXWeapon> ReplacedWeaponClass = nullptr;
			if (InPickUpCharacter->AddPickedWeaponClassToInventory(GetClass(), SlotIndex, ReplacedWeaponClass))
			{
				InPickUpCharacter->SetCurrentWeaponSlotIndex(SlotIndex);

				if (ReplacedWeaponClass != nullptr)
				{
					InPickUpCharacter->DropWeaponClass(ReplacedWeaponClass, GetActorLocation());
				}
			}
		}
	}

	if (ASXWeapon* PreviousWeapon = InPickUpCharacter->GetCurrentWeapon())
	{
		if (PreviousWeapon != this)
		{
			PreviousWeapon->CancelReload();
			PreviousWeapon->Destroy();
		}
	}

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	FName SocketNameToUse = CharacterAttachSocketName;
	if (TargetMesh->DoesSocketExist(SocketNameToUse) == false && TargetMesh->DoesSocketExist(LegacyCharacterAttachSocketName))
	{
		SocketNameToUse = LegacyCharacterAttachSocketName;
	}

	const bool bSocketExists = TargetMesh->DoesSocketExist(SocketNameToUse);
	if (bSocketExists == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon %s pickup warning: mesh %s has no socket '%s' or legacy socket '%s'."),
			*GetNameSafe(this),
			*GetNameSafe(TargetMesh),
			*CharacterAttachSocketName.ToString(),
			*LegacyCharacterAttachSocketName.ToString());
	}

	const bool bAttached = AttachToComponent(TargetMesh, AttachmentRules, bSocketExists ? SocketNameToUse : NAME_None);
	UE_LOG(LogTemp, Log, TEXT("Weapon %s pickup by %s. Attached: %s, SocketExists: %s"),
		*GetNameSafe(this),
		*GetNameSafe(InPickUpCharacter),
		bAttached ? TEXT("true") : TEXT("false"),
		bSocketExists ? TEXT("true") : TEXT("false"));

	InPickUpCharacter->ClearPickupCandidate(PickupComponent);

	if (bGrantInitialAmmo)
	{
		GrantInitialReserveAmmoTo(InPickUpCharacter);
	}
	InPickUpCharacter->SetCurrentWeapon(this);
	return true;
}

void ASXWeapon::HandleInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASXPlayerCharacter* OverlappedCharacter = Cast<ASXPlayerCharacter>(OtherActor);
	if (IsValid(OverlappedCharacter) == false || IsValid(PickupComponent) == false)
	{
		return;
	}

	if (PickupComponent->CanInteract_Implementation(OverlappedCharacter) == false)
	{
		return;
	}

	OverlappedCharacter->SetInteractionCandidate(PickupComponent);
}

void ASXWeapon::HandleInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASXPlayerCharacter* OverlappedCharacter = Cast<ASXPlayerCharacter>(OtherActor);
	if (IsValid(OverlappedCharacter) == false || IsValid(PickupComponent) == false)
	{
		return;
	}

	OverlappedCharacter->ClearInteractionCandidate(PickupComponent);
}

UMeshComponent* ASXWeapon::GetWeaponVisualMesh() const
{
	if (IsValid(SkeletalWeaponMesh) && IsValid(SkeletalWeaponMesh->GetSkeletalMeshAsset()))
	{
		return SkeletalWeaponMesh;
	}

	if (IsValid(StaticWeaponMesh) && IsValid(StaticWeaponMesh->GetStaticMesh()))
	{
		return StaticWeaponMesh;
	}

	return PickupComponent;
}

FTransform ASXWeapon::GetMuzzleTransform() const
{
	UMeshComponent* VisualMesh = GetWeaponVisualMesh();
	if (IsValid(VisualMesh))
	{
		if (VisualMesh->DoesSocketExist(MuzzleSocketName))
		{
			return VisualMesh->GetSocketTransform(MuzzleSocketName);
		}

		if (VisualMesh->DoesSocketExist(LegacyMuzzleSocketName))
		{
			return VisualMesh->GetSocketTransform(LegacyMuzzleSocketName);
		}
	}

	return GetActorTransform();
}

int32 ASXWeapon::GetReserveAmmo() const
{
	if (bInfiniteReserveAmmo)
	{
		// -1 is reserved as the UI sentinel for infinite reserve ammo.
		return -1;
	}

	if (const USXInventoryComponent* InventoryComponent = GetOwnerInventoryComponent())
	{
		return InventoryComponent->GetAmmoCount(CurrentAmmoType);
	}

	if (const FSXAmmoRuntimeState* AmmoState = GetCurrentAmmoState())
	{
		return AmmoState->ReserveAmmo;
	}

	return ReserveAmmo;
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
	}

	ReserveAmmo = GetReserveAmmo();
	OnAmmoChanged.Broadcast(this, CurrentAmmoType, AmmoInMagazine, ReserveAmmo, GetMagazineSize());
}

void ASXWeapon::SetCurrentAmmoType(ESXAmmoType NewAmmoType)
{
	if (bLockAmmoTypeToWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon %s ignored SetCurrentAmmoType(%s) because ammo type is fixed per weapon."),
			*GetNameSafe(this),
			*UEnum::GetValueAsString(NewAmmoType));
		BroadcastAmmoChanged();
		return;
	}

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

	CancelReload();
	CurrentAmmoType = NewAmmoType;
	BroadcastAmmoChanged();
}

void ASXWeapon::Reload()
{
	StartReload(Cast<ASXPlayerCharacter>(GetOwner()));
}

bool ASXWeapon::StartReload(ASXPlayerCharacter* InPickUpCharacter)
{
	if (IsValid(InPickUpCharacter) == false
		|| InPickUpCharacter->IsAlive() == false
		|| InPickUpCharacter->GetCurrentWeapon() != this
		|| CanReload() == false)
	{
		return false;
	}

	bIsReloading = true;
	bReloadAmmoTransferred = false;
	ReloadingCharacter = InPickUpCharacter;

	PlayReloadSound();

	UAnimInstance* AnimInstance = IsValid(InPickUpCharacter->GetMesh())
		? InPickUpCharacter->GetMesh()->GetAnimInstance()
		: nullptr;

	if (IsValid(AnimInstance) && IsValid(ReloadMontage))
	{
		const float MontageDuration = AnimInstance->Montage_Play(ReloadMontage);
		if (MontageDuration > 0.0f)
		{
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &ThisClass::HandleReloadMontageEnded);
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ReloadMontage);
			return true;
		}
	}

	// Preserve a functional reload for weapons that do not have a valid
	// montage yet. Weapons with a montage transfer ammo only through Notify.
	CompleteReload();
	FinishReload();
	return true;
}

void ASXWeapon::CompleteReload()
{
	if (bIsReloading == false || bReloadAmmoTransferred)
	{
		return;
	}

	FSXAmmoRuntimeState* AmmoState = GetCurrentAmmoState();
	if (AmmoState == nullptr)
	{
		return;
	}

	const int32 MagazineSize = GetMagazineSize();
	if (MagazineSize <= 0 || AmmoState->AmmoInMagazine >= MagazineSize)
	{
		return;
	}

	const int32 NeedAmmo = MagazineSize - AmmoState->AmmoInMagazine;
	const int32 AvailableReserveAmmo = bInfiniteReserveAmmo ? NeedAmmo : GetReserveAmmo();
	if (AvailableReserveAmmo <= 0)
	{
		return;
	}

	const int32 ReloadAmmo = FMath::Min(NeedAmmo, AvailableReserveAmmo);

	AmmoState->AmmoInMagazine += ReloadAmmo;
	if (bInfiniteReserveAmmo == false)
	{
		if (USXInventoryComponent* InventoryComponent = GetOwnerInventoryComponent())
		{
			InventoryComponent->ConsumeAmmo(CurrentAmmoType, ReloadAmmo);
		}
		else
		{
			AmmoState->ReserveAmmo -= ReloadAmmo;
		}
	}

	bReloadAmmoTransferred = true;
	BroadcastAmmoChanged();
}

void ASXWeapon::FinishReload()
{
	bIsReloading = false;
	bReloadAmmoTransferred = false;
	ReloadingCharacter = nullptr;
}

void ASXWeapon::CancelReload()
{
	if (bIsReloading == false)
	{
		return;
	}

	ASXPlayerCharacter* CharacterToStop = ReloadingCharacter.Get();
	bIsReloading = false;
	bReloadAmmoTransferred = false;
	ReloadingCharacter = nullptr;

	if (IsValid(CharacterToStop) && IsValid(CharacterToStop->GetMesh()))
	{
		UAnimInstance* AnimInstance = CharacterToStop->GetMesh()->GetAnimInstance();
		if (IsValid(AnimInstance) && IsValid(ReloadMontage))
		{
			AnimInstance->Montage_Stop(0.1f, ReloadMontage);
		}
	}
}

bool ASXWeapon::CanReload() const
{
	if (bIsReloading)
	{
		return false;
	}

	const FSXAmmoRuntimeState* AmmoState = GetCurrentAmmoState();
	const int32 MagazineSize = GetMagazineSize();
	return AmmoState != nullptr
		&& MagazineSize > 0
		&& AmmoState->AmmoInMagazine < MagazineSize
		&& (bInfiniteReserveAmmo || GetReserveAmmo() > 0);
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

USXInventoryComponent* ASXWeapon::GetOwnerInventoryComponent() const
{
	const ASXPlayerCharacter* OwnerCharacter = Cast<ASXPlayerCharacter>(GetOwner());
	return IsValid(OwnerCharacter) ? OwnerCharacter->GetInventoryComponent() : nullptr;
}

void ASXWeapon::GrantInitialReserveAmmoTo(ASXPlayerCharacter* InPickUpCharacter)
{
	if (bGrantInitialReserveAmmoOnPickup == false || bHasGrantedInitialReserveAmmo || IsValid(InPickUpCharacter) == false)
	{
		return;
	}

	if (bInfiniteReserveAmmo)
	{
		// An infinite fallback weapon must not inject a huge shared ammo count into
		// the inventory, because other weapons may use the same ammo type.
		bHasGrantedInitialReserveAmmo = true;
		BroadcastAmmoChanged();
		return;
	}

	USXInventoryComponent* InventoryComponent = InPickUpCharacter->GetInventoryComponent();
	if (IsValid(InventoryComponent) == false)
	{
		return;
	}

	if (const FSXAmmoData* AmmoData = GetCurrentAmmoData())
	{
		if (AmmoData->InitialReserveAmmo > 0)
		{
			InventoryComponent->AddAmmo(AmmoData->AmmoType, AmmoData->InitialReserveAmmo);
		}
	}

	bHasGrantedInitialReserveAmmo = true;
	BroadcastAmmoChanged();
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

float ASXWeapon::ApplyDamageToHit(const FHitResult& HitResult, ASXPlayerCharacter* InPickUpCharacter, float DamageAmount)
{
	ASXCharacterBase* HittedCharacter = Cast<ASXCharacterBase>(HitResult.GetActor());
	if (IsValid(HittedCharacter) == true)
	{
		FDamageEvent DamageEvent;
		UE_LOG(LogTemp, Log, TEXT("Apply %s ammo damage %.1f to %s"),
			*UEnum::GetValueAsString(CurrentAmmoType),
			DamageAmount,
			*GetNameSafe(HittedCharacter));
		return HittedCharacter->TakeDamage(DamageAmount, DamageEvent, InPickUpCharacter->GetController(), this);
	}
	return 0.0f;
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

	const FVector TracerEndLocation = bIsCollided ? HitResult.ImpactPoint : EndLocation;
	SpawnTracerEffect(AmmoData, StartLocation, TracerEndLocation);

	if (2 == ASXCharacterBase::ShowAttackRangedDebug)
	{
		DrawDebugSphere(GetWorld(), StartLocation, 2.f, 16, FColor::Red, false, 60.f);
		DrawDebugSphere(GetWorld(), TracerEndLocation, 2.f, 16, FColor::Green, false, 60.f);
		DrawDebugLine(GetWorld(), StartLocation, TracerEndLocation, FColor::Blue, false, 60.f, 0, 2.f);
	}

	if (bIsCollided == true)
	{
		UE_LOG(LogTemp, Log, TEXT("Weapon hit actor: %s, component: %s"),
			*GetNameSafe(HitResult.GetActor()),
			*GetNameSafe(HitResult.GetComponent()));

		const float AppliedDamage = ApplyDamageToHit(HitResult, InPickUpCharacter, AmmoData.Damage);
		if (AppliedDamage > 0.0f)
		{
			if (ASXPlayerController* PlayerController = InPickUpCharacter->GetController<ASXPlayerController>())
			{
				PlayerController->ShowHitMarker(Cast<ASXCharacterBase>(HitResult.GetActor())->IsAlive() == false);
			}
		}
		SpawnImpactEffect(HitResult);
	}
}

void ASXWeapon::FirePiercingAmmo(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation, const FCollisionQueryParams& TraceParams, ASXPlayerCharacter* InPickUpCharacter)
{
	SpawnTracerEffect(AmmoData, StartLocation, EndLocation);

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

		const float AppliedDamage = ApplyDamageToHit(HitResult, InPickUpCharacter, AmmoData.Damage);
		if (AppliedDamage > 0.0f)
		{
			if (ASXPlayerController* PlayerController = InPickUpCharacter->GetController<ASXPlayerController>())
			{
				PlayerController->ShowHitMarker(HittedCharacter->IsAlive() == false);
			}
		}
		SpawnImpactEffect(HitResult);
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
		SpawnTracerEffect(AmmoData, StartLocation, EndLocation);
		if (2 == ASXCharacterBase::ShowAttackRangedDebug)
		{
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Orange, false, 60.f, 0, 2.f);
		}
		return;
	}

	const FVector ExplosionLocation = HitResult.ImpactPoint;
	SpawnTracerEffect(AmmoData, StartLocation, ExplosionLocation);
	SpawnImpactEffect(HitResult);
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
	bool bAnyDamageApplied = false;
	bool bAnyKilled = false;
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
		const float AppliedDamage = HittedCharacter->TakeDamage(DamageAmount, DamageEvent, InPickUpCharacter->GetController(), this);
		if (AppliedDamage > 0.0f)
		{
			bAnyDamageApplied = true;
			bAnyKilled |= HittedCharacter->IsAlive() == false;
		}
		DamagedActors.Add(HittedCharacter);
	}

	if (bAnyDamageApplied)
	{
		if (ASXPlayerController* PlayerController = InPickUpCharacter->GetController<ASXPlayerController>())
		{
			PlayerController->ShowHitMarker(bAnyKilled);
		}
	}
}

void ASXWeapon::TryFire(ASXPlayerCharacter* InPickUpCharacter)
{
	if (IsValid(InPickUpCharacter) == false || bIsReloading)
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

#pragma region CalculateAimPoint
		FVector CameraLocation;
		FRotator CameraRotation;

		PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

		const FTransform MuzzleTransform = GetMuzzleTransform();
		const FVector WeaponMuzzleLocation = MuzzleTransform.GetLocation();
		const FVector CameraAimDirection = CameraRotation.Vector().GetSafeNormal();
		const FVector CameraTraceEnd = CameraLocation + CameraAimDirection * GetMaxAttackRange();

		FCollisionQueryParams TraceParams(NAME_None, false, this);
		TraceParams.AddIgnoredActor(this);
		TraceParams.AddIgnoredActor(InPickUpCharacter);

		FHitResult CameraHitResult;
		const bool bCameraHit = GetWorld()->LineTraceSingleByChannel(
			CameraHitResult,
			CameraLocation,
			CameraTraceEnd,
			ECC_Visibility,
			TraceParams
		);

		const FVector AimPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraTraceEnd;
		const FVector BulletDirection = (AimPoint - WeaponMuzzleLocation).GetSafeNormal();

		if (1 == ASXCharacterBase::ShowAttackRangedDebug)
		{
			DrawDebugSphere(GetWorld(), WeaponMuzzleLocation, 2.f, 16, FColor::Red, false, 60.f);
			DrawDebugSphere(GetWorld(), CameraLocation, 2.f, 16, FColor::Yellow, false, 60.f);
			DrawDebugSphere(GetWorld(), AimPoint, 4.f, 16, FColor::Magenta, false, 60.f);
			DrawDebugLine(GetWorld(), CameraLocation, AimPoint, FColor::Blue, false, 60.f, 0, 2.f);
			DrawDebugLine(GetWorld(), WeaponMuzzleLocation, AimPoint, FColor::Red, false, 60.f, 0, 2.f);
		}

#pragma endregion

#pragma region PerformLineTracing

		FireAmmo(*AmmoData, WeaponMuzzleLocation, BulletDirection, TraceParams, InPickUpCharacter);

#pragma endregion

		PlayFireFeedback(InPickUpCharacter, WeaponMuzzleLocation, MuzzleTransform.GetRotation().Rotator());
	}
}

void ASXWeapon::PlayFireFeedback(ASXPlayerCharacter* InPickUpCharacter, const FVector& MuzzleLocation, const FRotator& MuzzleRotation)
{
	UMeshComponent* VisualMesh = GetWeaponVisualMesh();
	if (IsValid(MuzzleFlashEffect))
	{
		if (IsValid(VisualMesh) && VisualMesh->DoesSocketExist(MuzzleSocketName))
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlashEffect, VisualMesh, MuzzleSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
		}
		else if (IsValid(VisualMesh) && VisualMesh->DoesSocketExist(LegacyMuzzleSocketName))
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlashEffect, VisualMesh, LegacyMuzzleSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
		}
		else
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, MuzzleFlashEffect, MuzzleLocation, MuzzleRotation);
		}
	}

	if (IsValid(FireSound))
	{
		if (IsValid(VisualMesh))
		{
			UGameplayStatics::SpawnSoundAttached(FireSound, VisualMesh, MuzzleSocketName);
		}
		else
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLocation, MuzzleRotation);
		}
	}

	if (IsValid(InPickUpCharacter) && IsValid(InPickUpCharacter->GetMesh()))
	{
		UAnimInstance* AnimInstance = InPickUpCharacter->GetMesh()->GetAnimInstance();
		UAnimMontage* CurrentAttackMontage = GetAttackMontage();
		if (IsValid(AnimInstance) && IsValid(CurrentAttackMontage) && AnimInstance->Montage_IsPlaying(CurrentAttackMontage) == false)
		{
			AnimInstance->Montage_Play(CurrentAttackMontage);
		}
	}

	APlayerController* PlayerController = IsValid(InPickUpCharacter) ? InPickUpCharacter->GetController<APlayerController>() : nullptr;
	if (IsValid(PlayerController))
	{
		TSubclassOf<UCameraShakeBase> CameraShakeToPlay = FireCameraShake;
		if (CameraShakeToPlay == nullptr)
		{
			CameraShakeToPlay = InPickUpCharacter->AttackFireCameraShake;
		}

		if (CameraShakeToPlay != nullptr)
		{
			PlayerController->ClientStartCameraShake(CameraShakeToPlay);
		}
	}
}

void ASXWeapon::PlayReloadSound()
{
	if (IsValid(ReloadSound))
	{
		UMeshComponent* VisualMesh = GetWeaponVisualMesh();
		if (IsValid(VisualMesh))
		{
			UGameplayStatics::SpawnSoundAttached(ReloadSound, VisualMesh);
		}
		else
		{
			UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
		}
	}
}

void ASXWeapon::SpawnTracerEffect(const FSXAmmoData& AmmoData, const FVector& StartLocation, const FVector& EndLocation) const
{
	UNiagaraSystem* TracerToSpawn = AmmoData.TracerEffect;
	if (IsValid(TracerToSpawn) == false)
	{
		TracerToSpawn = DefaultTracerEffect;
	}

	if (IsValid(TracerToSpawn) == false)
	{
		return;
	}

	UNiagaraComponent* TracerComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		TracerToSpawn,
		StartLocation,
		(EndLocation - StartLocation).Rotation(),
		FVector(1.0f),
		true,
		false
	);

	if (IsValid(TracerComponent) == false)
	{
		return;
	}

	TracerComponent->SetVariableVec3(TracerStartParameterName, StartLocation);
	TracerComponent->SetVariableVec3(TracerEndParameterName, EndLocation);
	TracerComponent->Activate(true);

	if (TracerLifeTime > 0.0f)
	{
		TWeakObjectPtr<UNiagaraComponent> WeakTracerComponent = TracerComponent;
		FTimerHandle DestroyTracerTimerHandle;
		GetWorldTimerManager().SetTimer(
			DestroyTracerTimerHandle,
			FTimerDelegate::CreateLambda([WeakTracerComponent]()
			{
				if (WeakTracerComponent.IsValid())
				{
					WeakTracerComponent->DestroyComponent();
				}
			}),
			TracerLifeTime,
			false
		);
	}
}

void ASXWeapon::HandleReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ReloadMontage)
	{
		return;
	}

	if (bIsReloading && bInterrupted == false && bReloadAmmoTransferred == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reload montage %s ended without an SX Reload Ammo notify for weapon %s."),
			*GetNameSafe(ReloadMontage),
			*GetNameSafe(this));

		// Keep the weapon functional when an asset is missing its reload notify.
		// A correctly configured montage still transfers ammo at the notify frame.
		CompleteReload();
	}

	FinishReload();
}

void ASXWeapon::SpawnImpactEffect(const FHitResult& HitResult) const
{
	if (IsValid(ImpactEffect) == false || HitResult.bBlockingHit == false)
	{
		return;
	}

	const FRotator ImpactRotation = HitResult.ImpactNormal.Rotation();
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, HitResult.ImpactPoint, ImpactRotation);
}
