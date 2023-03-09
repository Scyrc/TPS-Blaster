// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"

UBuffComponent::UBuffComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
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


void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;

	AmountToHeal += HealAmount;
	HealingRate += HealAmount / HealingTime;
}

