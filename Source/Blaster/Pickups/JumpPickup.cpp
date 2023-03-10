// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickup.h"

#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

AJumpPickup::AJumpPickup()
{
	bReplicates = true;
}

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OVerlappedComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);
	
	Super::OnSphereOverlap(OVerlappedComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);

	
	Super::OnSphereOverlap(OVerlappedComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);

	const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter && BlasterCharacter->GetBuffComponent())
	{
		BlasterCharacter->GetBuffComponent()->BuffJump(JumpZVelocityBuff, JumpBuffTime);
	}

	Destroy();
}
