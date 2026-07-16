// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/SXEnemyMelee.h"

#include "Components/SXStatusComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ASXEnemyMelee::ASXEnemyMelee()
{
	if (IsValid(StatusComponent))
	{
		StatusComponent->SetMaxHealth(50.0f);
	}

	AttackDamage = 10.0f;
	AttackRange = 150.0f;
	AttackInterval = 1.0f;

	GetCharacterMovement()->MaxWalkSpeed = 450.0f;
}
