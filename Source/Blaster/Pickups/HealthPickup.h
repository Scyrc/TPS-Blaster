// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	AHealthPickup();
	virtual void Destroyed() override;
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Health")
	float HealAmount = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Health")
	float HealTime = 5.f;

	UPROPERTY(VisibleDefaultsOnly)
	class UNiagaraComponent* PickupEffectComp;
		
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* PickupEffect;

	
};
