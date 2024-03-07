// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "BlasterAction.generated.h"

class UActionComponent;

USTRUCT()
struct FActionRepData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsRunning;

	UPROPERTY()
	AActor* InstigatorActor;
};

/**
 * 
 */
UCLASS(Blueprintable)
class BLASTER_API UBlasterAction : public UObject
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void Initialize(UActionComponent* ActionComponent);

	UPROPERTY(EditDefaultsOnly, Category = "Action")
	bool bAutoStart;
	
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	FName ActionName;

	bool IsRunning() const;

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
	void StartAction(AActor* Instigator);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
	void StopAction(AActor* Instigator);

	virtual bool IsSupportedForNetworking() const override { return true;}

	UWorld* GetWorld() const override;

	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	UTexture2D* Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer GrantsTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer BlockedTags;
	
	UPROPERTY(Replicated)
	UActionComponent* ActionComp;

	UPROPERTY(ReplicatedUsing ="OnRep_ActionRepData")
	FActionRepData ActionRepData;

	UFUNCTION()
	void OnRep_ActionRepData();

	UPROPERTY(Replicated, BlueprintReadOnly)
	float TimeStarted;

	UActionComponent* GetOwningComponent();
};
