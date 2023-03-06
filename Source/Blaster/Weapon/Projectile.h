// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	void ExplodeDamage();
protected:
	virtual void BeginPlay() override;
	void DestroyTimerFinished();
	void StartDestroyTimers();
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);

	UPROPERTY(VisibleAnywhere, Category="Projectile pros")
	UStaticMeshComponent* ProjectMesh;
	
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	UPROPERTY(EditDefaultsOnly)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;
	
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditDefaultsOnly, Category="Projectile pros")
	class UNiagaraSystem* TrailSystem;
	
	UPROPERTY(VisibleDefaultsOnly, Category=Rocket)
	class UNiagaraComponent* TrailSystemComp;

	void SpawnTrailSystem();

	UPROPERTY(EditDefaultsOnly, Category=Damage)
	float MinDamage = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	float DamageInnerRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	float DamageOuterRadius = 500.f;
	
private:
	UPROPERTY(EditAnywhere)	
	class UParticleSystem* Tracer;
	
	UPROPERTY()	
	class UParticleSystemComponent* TracerComp;


	
	FTimerHandle DestroyTimer;

	UPROPERTY(EditDefaultsOnly, Category="Projectile pros")
	float DestroyTime = 3.f;

};
