// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Balloon.generated.h"

UCLASS()
class BLASTER_API ABalloon : public AActor
{
	GENERATED_BODY()
	
public:	
	ABalloon();

	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;
	
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(EditAnywhere)
	float MoveSpeed = 1.f;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);

	UPROPERTY()
	FTimerHandle DestroyTimer;


	void AutoDestroy();

	UPROPERTY(EditAnywhere)
	float AutoDestroyTime = 10.f;

	UPROPERTY(EditDefaultsOnly, Category="pickup")
	float baseTurnRate = 45.f;

	/*
	 *  Effect
	 */

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* DestroyEffect;
	
public:	
	
	virtual void Tick(float DeltaTime) override;

};


