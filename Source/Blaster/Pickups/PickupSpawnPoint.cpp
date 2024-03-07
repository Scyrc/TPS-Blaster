// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"

#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	StartPickupTimer((AActor*)nullptr);
}

void APickupSpawnPoint::SpawnPickup()
{
	int NumPickupClasses = PickupClasses.Num();
	if(NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		SpawnedPickup =  GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		if(HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartPickupTimer);
		}
	}
}

void APickupSpawnPoint::StartPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	//FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &SpawnPickupTimersFinished);
	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer,
		this,
		&APickupSpawnPoint::SpawnPickupTimersFinished,
		SpawnTime
		);
}

void APickupSpawnPoint::SpawnPickupTimersFinished()
{
	if(HasAuthority())
	{
		SpawnPickup();
	}
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

