// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;
	//SetIsReplicated(true);
	SetIsReplicatedByDefault(true);

}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}


// run on server
void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if(Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeed,
		BuffTime);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
		MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
	}
}

void UBuffComponent::ResetSpeed()
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
		MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
	}
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed; 
	}
}

// run on server
void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if(Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
		
		MulticastJumpBuff(BuffJumpVelocity);
	}
}

void UBuffComponent::SetInitialJumpVelocity(float JumpZVelocity)
{
	InitialJumpZVelocity = JumpZVelocity;
}

void UBuffComponent::ResetJump()
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpZVelocity;
		
		MulticastJumpBuff(InitialJumpZVelocity);
	}
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

// run on server
void UBuffComponent::HealRampUp(float DeltaTime)
{
	if(!bHealing || Character== nullptr || Character->IsElimmed()) return;

	if(AmountToHeal <= 0.f)
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
	if(bHealing)
	{
		const float HealThisFrame = HealingRate * DeltaTime;
		Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
		Character->UpdateHUDHealth(); 
		AmountToHeal -= HealThisFrame;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if(!bReplenishShield || Character== nullptr || Character->IsElimmed()) return;

	if(AmountToShield <= 0.f)
	{
		bReplenishShield = false;
		AmountToShield = 0.f;
	}
	if(bReplenishShield)
	{
		const float ShieldThisFrame = ShieldReplenishRate * DeltaTime;
		Character->SetShield(FMath::Clamp(Character->GetShield() + ShieldThisFrame, 0.f, Character->GetMaxShield()));
		Character->UpdateHUDShield(); 
		AmountToShield -= ShieldThisFrame;
	}
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;

	AmountToHeal += HealAmount;
	HealingRate += HealAmount / HealingTime;
}

void UBuffComponent::ReplenishShield(float Amount, float BuffTime)
{
	bReplenishShield = true;

	AmountToShield += Amount;
	ShieldReplenishRate += Amount / BuffTime;
}
