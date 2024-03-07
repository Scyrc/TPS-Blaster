// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../BlasterAction.h"
#include "SpeedAction.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API USpeedAction : public UBlasterAction
{
	GENERATED_BODY()
public:
	USpeedAction();

	virtual void StartAction_Implementation(AActor* Instigator) override;

	virtual bool CanStart(AActor* Instigator) override;;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	float SpeedGain;

	UPROPERTY(EditDefaultsOnly, Category = "Action")
	float SpeedGainTime;
	
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	float SprintDistance;

	int32 ActionState;
	
	UFUNCTION()
	void SpeedGainEnd(AActor* Instigator);

	FTimerHandle SpeedTimer;

};

