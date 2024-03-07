// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BlasterAIController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterAIController : public AAIController
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category="AI")
	UBehaviorTree* BehaviorTree;
protected:
	virtual void BeginPlay() override;
	
};
