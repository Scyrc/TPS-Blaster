// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;

	UPROPERTY(EditDefaultsOnly, Category=Damage)
	float MinDamage = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	float DamageInnerRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	float DamageOuterRadius = 500.f;
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;
};
