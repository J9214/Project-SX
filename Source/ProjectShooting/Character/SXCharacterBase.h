// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Item/SXWeapon.h"
#include "SXCharacterBase.generated.h"

class USXStatusComponent;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSXOnCurrentWeaponChangedSignature, ASXWeapon*, CurrentWeapon);

UCLASS(Abstract)
class PROJECTSHOOTING_API ASXCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ASXCharacterBase();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category="SX|Character")
	virtual void ReceiveDamage(float DamageAmount, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category="SX|Character")
	virtual void Die(AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category="SX|Character")
	void BP_OnDamaged(float DamageAmount, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category="SX|Character")
	void BP_OnDied(AActor* DamageCauser);

	UFUNCTION(BlueprintPure, Category="SX|Character")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category="SX|Character")
	virtual FVector GetTargetLocation(AActor* RequestedBy = nullptr) const override;

	UFUNCTION(BlueprintPure, Category="SX|Character")
	USXStatusComponent* GetStatusComponent() const { return StatusComponent; }

	UPROPERTY(BlueprintAssignable, Category="SX|Weapon")
	FSXOnCurrentWeaponChangedSignature OnCurrentWeaponChanged;

	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void HandleDeath(USXStatusComponent* DeadStatusComponent, AActor* InstigatorActor);

	void StartDeathDissolve();
	void UpdateDeathDissolve();
	void FinishDeathDissolve();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Components")
	TObjectPtr<USXStatusComponent> StatusComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Character")
	bool bIsDead = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Death|Dissolve")
	bool bUseDeathDissolve = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Death|Dissolve")
	bool bDestroyAfterDeathDissolve = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Death|Dissolve", meta=(ClampMin="0.01", Units=s))
	float DeathDissolveDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Death|Dissolve")
	FName DissolveParameterName = TEXT("Dissolve");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Death|Dissolve")
	float DeathDissolveStartValue = -0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SX|Death|Dissolve")
	float DeathDissolveEndValue = 0.7f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DeathDissolveMaterials;

	FTimerHandle DeathDissolveTimerHandle;

	float DeathDissolveElapsedTime = 0.0f;

#pragma region Attack
public:
	UFUNCTION(BlueprintPure, Category="SX|Weapon")
	ASXWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintPure, Category="SX|Weapon")
	ESXWeaponType GetCurrentWeaponType() const;

	UFUNCTION(BlueprintCallable, Category="SX|Weapon")
	void SetCurrentWeapon(ASXWeapon* NewWeapon);
	
	UAnimMontage* GetCurrentWeaponAttackAnimMontage() const;

	static int32 ShowAttackRangedDebug;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SX|Weapon")
	TObjectPtr<ASXWeapon> CurrentWeapon;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> AttackMeleeMontage;
#pragma endregion
};
 