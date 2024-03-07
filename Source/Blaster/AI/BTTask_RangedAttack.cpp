// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_RangedAttack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

EBTNodeResult::Type UBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//return Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* Controller = OwnerComp.GetAIOwner();

	if(ensure(Controller))
	{
		ABlasterCharacter* MyPawn = Cast<ABlasterCharacter>(Controller->GetPawn());
		if(MyPawn == nullptr)
		{
			return EBTNodeResult::Failed;
		}

		MyPawn->GetCombatComponent()->FireButtonPressed(true);
		MyPawn->GetCombatComponent()->FireButtonPressed(false);
		
		return EBTNodeResult::Succeeded;

	}

	//UGameInstance
	return EBTNodeResult::Failed;
}
