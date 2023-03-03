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
	virtual void Destroyed() override;
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	float MinDamage = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	float DamageInnerRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	float DamageOuterRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category=Rocket)
	class UNiagaraSystem* TrailSystem;
	
	UPROPERTY(VisibleDefaultsOnly, Category=Rocket)
	class UNiagaraComponent* TrailSystemComp;

	UPROPERTY(EditDefaultsOnly, Category=Rocket)
	class USoundCue* ProjectileLoop;
	
	UPROPERTY(EditDefaultsOnly, Category=Rocket)
	class USoundAttenuation* LoopSoundAttenuation;
	
	UPROPERTY(VisibleDefaultsOnly, Category=Rocket)
	class UAudioComponent* ProjectileLoopComp;
private:
	UPROPERTY(VisibleAnywhere, Category=Rocket)
	UStaticMeshComponent* RocketMesh;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditDefaultsOnly, Category=Rocket)
	float DestroyTime = 3.f;

	void DestroyTimerFinished();
};
