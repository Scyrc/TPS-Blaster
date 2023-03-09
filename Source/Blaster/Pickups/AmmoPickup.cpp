// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

// run on server
void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OVerlappedComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);


	const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter && BlasterCharacter->GetCombatComponent())
	{
		BlasterCharacter->GetCombatComponent()->PickAmmo(AmmoAmount, WeaponType);
	}

	Destroy();
}
