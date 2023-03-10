// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ASpeedPickup : public APickup
{
	GENERATED_BODY()
public:
	ASpeedPickup();
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Speed Buff")
	float BaseSpeedBuff = 1600.f;

	UPROPERTY(EditDefaultsOnly, Category="Speed Buff")
	float CrouchSpeedBuff = 850.f;

	UPROPERTY(EditDefaultsOnly, Category="Speed Buff")
	float SpeedBuffTime = 10.f;
};
