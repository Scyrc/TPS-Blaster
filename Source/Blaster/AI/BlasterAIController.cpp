// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

void ABlasterAIController::BeginPlay()
{
	Super::BeginPlay();

	RunBehaviorTree(BehaviorTree);


	auto Player = UGameplayStatics::GetPlayerPawn(this, 0);

	GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), Player);
}
