 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ActionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionChanged, UActionComponent*, ActionComp, UBlasterAction*, Action);
class UBlasterAction;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UActionComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Actions")
	void StartActionByName(AActor* Instigator, FName ActionName);

	UFUNCTION(Server, Reliable)
	void ServerStartAction(AActor* Instigator, FName ActionName);
	
	UFUNCTION(BlueprintCallable, Category="Actions")
	void StopActionByName(AActor* Instigator, FName ActionName);

	UFUNCTION(Server, Reliable)
	void ServerStopAction(AActor* Instigator, FName ActionName);
	
	void AddAction(AActor* Instigator, TSubclassOf<UBlasterAction> ActionClass);
public:
	FGameplayTagContainer ActiveGameplayTags;

	UPROPERTY(BlueprintAssignable)
	FOnActionChanged OnActionStarted;
	
	UPROPERTY(BlueprintAssignable)
	FOnActionChanged OnActionStopped;

	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Actions")
	TArray<UBlasterAction*> ActionList;
	
	UPROPERTY(EditAnywhere, Category = "Actions")
	TArray<TSubclassOf<UBlasterAction>> DefaultActions;
	
};
