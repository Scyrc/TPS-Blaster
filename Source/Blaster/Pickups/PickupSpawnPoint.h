// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="pickup")
	TArray<TSubclassOf<class APickup>> PickupClasses;

	void SpawnPickup();
	UFUNCTION()
	void StartPickupTimer(AActor* DestroyedActor);
	void SpawnPickupTimersFinished();

	UPROPERTY()
	APickup* SpawnedPickup;
	
private:

	FTimerHandle SpawnPickupTimer;
	
	UPROPERTY(EditDefaultsOnly, Category="pickup")
	float SpawnPickupTimeMin = 5.f;
	
	UPROPERTY(EditDefaultsOnly, Category="pickup")
	float SpawnPickupTimeMax = 10.f;

};
