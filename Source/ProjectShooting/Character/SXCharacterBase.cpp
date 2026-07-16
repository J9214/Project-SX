// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/SXCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "Components/SXStatusComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/SXWeapon.h"

int32 ASXCharacterBase::ShowAttackRangedDebug = 2;

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

	SetLifeSpan(10.0f);
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

	CurrentWeapon = NewWeapon;
	OnCurrentWeaponChanged.Broadcast(CurrentWeapon);

	if (IsValid(CurrentWeapon.Get()) == true)
	{
		CurrentWeapon->BroadcastAmmoChanged();
	}
}

UAnimMontage* ASXCharacterBase::GetCurrentWeaponAttackAnimMontage() const
{
	if (IsValid(CurrentWeapon.Get()) == true)
	{
		return CurrentWeapon->GetAttackMontage();
	}
	return nullptr;
}
