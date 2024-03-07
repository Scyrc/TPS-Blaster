// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CheckAttackRange.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void UBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();

	if(ensure(BlackboardComponent))
	{
		const AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject("TargetActor"));
		if(TargetActor)
		{
			const AAIController* Controller = OwnerComp.GetAIOwner();
			if(ensure(Controller))
			{
				const APawn* AIPawn = Controller->GetPawn();
				if(ensure(AIPawn))
				{
					const float DistanceTo = FVector::Distance(TargetActor->GetActorLocation(), AIPawn->GetActorLocation());

					const bool bWithRange = DistanceTo <= DistanceAttack;

					bool bHasLos = false;
					if(bWithRange)
					{
						bHasLos = Controller->LineOfSightTo(TargetActor);
					}
					BlackboardComponent->SetValueAsBool(AttackRangeKey.SelectedKeyName, (bWithRange && bHasLos));
				}
			}
		}
	}
}
