// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupTypes.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
protected:
	UPROPERTY(EditDefaultsOnly, Category="pickup")
	float baseTurnRate = 45.f;
	
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OVerlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UPROPERTY(VisibleDefaultsOnly)
	class UNiagaraComponent* PickupEffectComp;
		
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* PickupEffect;
	
private:
	UPROPERTY(EditDefaultsOnly, Category="pickup")
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditDefaultsOnly, Category="pickup")
	class UStaticMeshComponent* PickupMesh;
	
	UPROPERTY(EditDefaultsOnly, Category="pickup")
	class USoundCue* PickupSound;

	UPROPERTY(EditDefaultsOnly, Category="pickup")
	EPickupHighLight PickupHighLightt = EPickupHighLight::EPHL_PURPLE;
	
	int32 NumberOfPickupHighLight() const;
	
public:	
	

};
