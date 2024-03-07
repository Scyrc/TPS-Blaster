// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAction.h"

#include "Blaster/BlasterComponents/ActionComponent.h"
#include "Net/UnrealNetwork.h"

void UBlasterAction::Initialize(UActionComponent* ActionComponent)
{
	ActionComp = ActionComponent;
}

bool UBlasterAction::IsRunning() const
{
	return ActionRepData.bIsRunning;
}

UWorld* UBlasterAction::GetWorld() const
{
	AActor* Actor = Cast<AActor>(GetOuter());

	if(Actor)
	{
		return Actor->GetWorld();
	}

	return nullptr;
	
}

void UBlasterAction::OnRep_ActionRepData()
{
	if(ActionRepData.bIsRunning)
	{
		StartAction(ActionRepData.InstigatorActor);
	}
	else
	{
		StopAction(ActionRepData.InstigatorActor);
	}
}

UActionComponent* UBlasterAction::GetOwningComponent()
{
	return ActionComp;
}

bool UBlasterAction::CanStart(AActor* Instigator)
{
	if(ActionRepData.bIsRunning)
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
	}
	return true;
}

void UBlasterAction::StartAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Running: %s"), *GetNameSafe(this));

	ActionComp->ActiveGameplayTags.AppendTags(GrantsTags);

	ActionRepData.bIsRunning = true;
	ActionRepData.InstigatorActor = Instigator;

	// run on server

	if(ActionComp->GetOwnerRole() == ROLE_Authority)
	{
		TimeStarted = GetWorld()->TimeSeconds;
	}

	GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
}

void UBlasterAction::StopAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Stopped: %s"), *GetNameSafe(this));

	if(ActionComp->GetOwnerRole() == ROLE_Authority) ensureAlways(ActionRepData.bIsRunning);

	GetOwningComponent()->ActiveGameplayTags.RemoveTags(GrantsTags);

	ActionRepData.bIsRunning = false;
	ActionRepData.InstigatorActor = Instigator;

	GetOwningComponent()->OnActionStopped.Broadcast(GetOwningComponent(), this);

}

void UBlasterAction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBlasterAction, ActionRepData);
	DOREPLIFETIME(UBlasterAction, ActionComp);
	DOREPLIFETIME(UBlasterAction, TimeStarted);
}