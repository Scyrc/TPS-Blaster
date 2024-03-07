// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionComponent.h"

#include "Blaster/BlasterActions/BlasterAction.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UActionComponent::UActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	//SetIsReplicated(true);
	SetIsReplicatedByDefault(true);
}




void UActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// add default actions to component on server
	if(GetOwner()->HasAuthority())  
	{
		for(auto ActionClass : DefaultActions)
		{
			if(ActionClass)
			{
				AddAction(GetOwner(), ActionClass);
			}
		}
	}
}


void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{
	ServerStartAction(Instigator, ActionName);
}

void UActionComponent::ServerStartAction_Implementation(AActor* Instigator, FName ActionName)
{
	for(auto Action : ActionList)
	{
		if(Action && Action->ActionName == ActionName)
		{
			if(!Action->CanStart(Instigator))
			{
				FString DebugMsg = FString::Printf(TEXT("Failed to run: %s"), *ActionName.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
				continue;
			}

			Action->StartAction(Instigator);
			return;
		}
	}
}
void UActionComponent::StopActionByName(AActor* Instigator, FName ActionName)
{
	ServerStopAction(Instigator, ActionName);
}
void UActionComponent::ServerStopAction_Implementation(AActor* Instigator, FName ActionName)
{
	for(auto Action : ActionList)
	{
		if(Action && Action->ActionName == ActionName)
		{
			if(Action->IsRunning())
			{
				Action->StopAction(Instigator);
				return;
			}
		}
	}
}

// run on server
void UActionComponent::AddAction(AActor* Instigator, TSubclassOf<UBlasterAction> ActionClass)
{
	if(!ensure(ActionClass)) return;

	if(!GetOwner()->HasAuthority()) return;

	UBlasterAction* ActionToAdd = NewObject<UBlasterAction>(GetOwner(), ActionClass);

	if(ensure(ActionToAdd))
	{
		ActionToAdd->Initialize(this);
		ActionList.Add(ActionToAdd);

		if(ActionToAdd->bAutoStart && ensure(ActionToAdd->CanStart(Instigator)))
		{
			ActionToAdd->StartAction(Instigator);
		}
	}
}


void UActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UActionComponent, ActionList);
}

bool UActionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UBlasterAction* Action : ActionList)
	{
		if (Action)
		{
			WroteSomething |= Channel->ReplicateSubobject(Action, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}
