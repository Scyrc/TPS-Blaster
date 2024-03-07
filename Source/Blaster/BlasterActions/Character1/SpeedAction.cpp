// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedAction.h"

#include "Blaster/BlasterComponents/ActionComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

USpeedAction::USpeedAction()
{
	ActionState = 0;
}

void USpeedAction::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);
	
	if((ActionComp->GetOwnerRole() == ROLE_Authority) == false) return; // run on server
	if(ActionState == 0)
	{
		ActionState  =1 ; // speed gain state
		ABlasterCharacter* Character  = Cast<ABlasterCharacter>(Instigator);
		if(Character)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed += SpeedGain; // auto replicate by engine
		}
		FTimerDelegate Delagate;
		Delagate.BindUFunction(this, "SpeedGainEnd", Instigator);
		GetWorld()->GetTimerManager().SetTimer(
			SpeedTimer,
			Delagate,
			SpeedGainTime,
			false
		);
	}
	else if(ActionState == 1) // 
	{
		SpeedGainEnd(Instigator);
	}
	
}

bool USpeedAction::CanStart(AActor* Instigator)
{
	//return Super::CanStart(Instigator);

	/*if(ActionRepData.bIsRunning)
	{
		FString DebugMsg = FString::Printf(TEXT("Action is Running: %s"), *ActionName.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
		return false;
	}
	const UActionComponent* Comp = GetOwningComponent();
	if(Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		FString DebugMsg = FString::Printf(TEXT("Action has BlockedTags: %s"), *ActionName.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
		return false;
	}*/
	return true;
}

void USpeedAction::SpeedGainEnd(AActor* Instigator)
{
	if(ActionState == 1)
	{
		ActionState = 2;
		ABlasterCharacter* Character  = Cast<ABlasterCharacter>(Instigator);
		if(Character)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed -= SpeedGain; // auto replicate by engine
			const FVector  CurrentLoc = Character->GetActorLocation();
			
			const FVector  NewLoc = CurrentLoc + SprintDistance * Character->GetActorRotation().Vector();
			Character->TeleportTo(NewLoc, Character->GetActorRotation(), false, false);
		}
		ActionState = 0;

		StopAction(Instigator);
	}
}

