// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
private:

	UPROPERTY(EditDefaultsOnly)
	float Damage = 20.f;
	
	UPROPERTY(EditDefaultsOnly)
	class UParticleSystem* ParticleSystem;
	
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MuzzleFlash;
	
	UPROPERTY(EditDefaultsOnly)
	USoundCue* FireSound;
	
	UPROPERTY(EditDefaultsOnly)
	USoundCue* HitSound;
};
