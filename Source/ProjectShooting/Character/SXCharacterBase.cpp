// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/SXCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SXStatusComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/SXWeapon.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

int32 ASXCharacterBase::ShowAttackRangedDebug = 0;

FAutoConsoleVariableRef CVarShowAttackRangedDebug(
	TEXT("SX.ShowAttackRangedDebug"),
	ASXCharacterBase::ShowAttackRangedDebug,
	TEXT(""),
	ECVF_Cheat
);

ASXCharacterBase::ASXCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	StatusComponent = CreateDefaultSubobject<USXStatusComponent>(TEXT("StatusComponent"));

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ASXCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (StatusComponent)
	{
		StatusComponent->OnDeath.AddDynamic(this, &ASXCharacterBase::HandleDeath);
	}
}

float ASXCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float SuperDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float FinalDamage = SuperDamage > 0.0f ? SuperDamage : DamageAmount;

	ReceiveDamage(FinalDamage, DamageCauser ? DamageCauser : EventInstigator);
	return FinalDamage;
}

void ASXCharacterBase::ReceiveDamage(float DamageAmount, AActor* DamageCauser)
{
	if (!StatusComponent || bIsDead)
	{
		return;
	}

	const float AppliedDamage = StatusComponent->ApplyDamage(DamageAmount, DamageCauser);
	if (AppliedDamage > 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("%s received %.1f damage from %s. Health: %.1f / %.1f"),
			*GetName(),
			AppliedDamage,
			*GetNameSafe(DamageCauser),
			StatusComponent->GetHealth(),
			StatusComponent->GetMaxHealth());

		BP_OnDamaged(AppliedDamage, DamageCauser);
	}
}

void ASXCharacterBase::Die(AActor* DamageCauser)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	DetachFromControllerPendingDestroy();

	BP_OnDied(DamageCauser);

	if (bUseDeathDissolve)
	{
		StartDeathDissolve();
	}
	else
	{
		SetLifeSpan(10.0f);
	}
}

void ASXCharacterBase::StartDeathDissolve()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (IsValid(MeshComponent) == false)
	{
		if (bDestroyAfterDeathDissolve)
		{
			Destroy();
		}
		return;
	}

	DeathDissolveMaterials.Reset();

	const int32 MaterialCount = MeshComponent->GetNumMaterials();
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(MaterialIndex);
		if (IsValid(DynamicMaterial))
		{
			DynamicMaterial->SetScalarParameterValue(DissolveParameterName, DeathDissolveStartValue);
			DeathDissolveMaterials.Add(DynamicMaterial);
		}
	}

	if (DeathDissolveMaterials.IsEmpty())
	{
		if (bDestroyAfterDeathDissolve)
		{
			Destroy();
		}
		return;
	}

	DeathDissolveElapsedTime = 0.0f;
	GetWorldTimerManager().ClearTimer(DeathDissolveTimerHandle);

	constexpr float DissolveTickInterval = 0.02f;
	GetWorldTimerManager().SetTimer(
		DeathDissolveTimerHandle,
		this,
		&ThisClass::UpdateDeathDissolve,
		DissolveTickInterval,
		true
	);
}

void ASXCharacterBase::UpdateDeathDissolve()
{
	constexpr float DissolveTickInterval = 0.02f;
	DeathDissolveElapsedTime += DissolveTickInterval;

	const float SafeDuration = FMath::Max(0.01f, DeathDissolveDuration);
	const float Alpha = FMath::Clamp(DeathDissolveElapsedTime / SafeDuration, 0.0f, 1.0f);
	const float DissolveValue = FMath::Lerp(DeathDissolveStartValue, DeathDissolveEndValue, Alpha);

	for (UMaterialInstanceDynamic* DynamicMaterial : DeathDissolveMaterials)
	{
		if (IsValid(DynamicMaterial))
		{
			DynamicMaterial->SetScalarParameterValue(DissolveParameterName, DissolveValue);
		}
	}

	if (Alpha >= 1.0f)
	{
		FinishDeathDissolve();
	}
}

void ASXCharacterBase::FinishDeathDissolve()
{
	GetWorldTimerManager().ClearTimer(DeathDissolveTimerHandle);

	if (bDestroyAfterDeathDissolve)
	{
		Destroy();
	}
}

bool ASXCharacterBase::IsAlive() const
{
	return StatusComponent && StatusComponent->IsAlive() && !bIsDead;
}

FVector ASXCharacterBase::GetTargetLocation(AActor* RequestedBy) const
{
	return GetActorLocation() + FVector(0.0f, 0.0f, BaseEyeHeight);
}

void ASXCharacterBase::HandleDeath(USXStatusComponent* DeadStatusComponent, AActor* InstigatorActor)
{
	Die(InstigatorActor);
}

void ASXCharacterBase::SetCurrentWeapon(ASXWeapon* NewWeapon)
{
	if (CurrentWeapon == NewWeapon)
	{
		return;
	}

	if (IsValid(CurrentWeapon.Get()))
	{
		CurrentWeapon->CancelReload();
	}

	CurrentWeapon = NewWeapon;
	OnCurrentWeaponChanged.Broadcast(CurrentWeapon);

	if (IsValid(CurrentWeapon.Get()) == true)
	{
		CurrentWeapon->BroadcastAmmoChanged();
	}
}

ESXWeaponType ASXCharacterBase::GetCurrentWeaponType() const
{
	if (IsValid(CurrentWeapon.Get()) == false)
	{
		return ESXWeaponType::Unarmed;
	}

	return CurrentWeapon->GetWeaponType();
}

UAnimMontage* ASXCharacterBase::GetCurrentWeaponAttackAnimMontage() const
{
	if (IsValid(CurrentWeapon.Get()) == true)
	{
		return CurrentWeapon->GetAttackMontage();
	}
	return nullptr;
}
