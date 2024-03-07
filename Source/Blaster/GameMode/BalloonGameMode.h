// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "BalloonGameMode.generated.h"

class ABalloon;
/**
 * 
 */
UCLASS()
class BLASTER_API ABalloonGameMode : public ATeamsGameMode
{
	GENERATED_BODY()

public:
	ABalloonGameMode();
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float Damage);

	
	virtual void PlayerHitBalloon(AActor* OwnerCharacter, ABlasterCharacter* Shooter);

	UPROPERTY()
	int32 CurrentBallNum = 0;
protected:
	
	virtual void HandleMatchHasStarted() override;

	UPROPERTY(EditAnywhere, Category="GAME SET")
	int32 MaxBallNum = 10;

	UPROPERTY(EditAnywhere, Category="GAME SET")
	float AXisMax = 500;

	UPROPERTY(EditAnywhere, Category="GAME SET")
	float AXisMin = 0;

	UPROPERTY(EditAnywhere, Category="GAME SET")
	float AYisMax = 500;

	UPROPERTY(EditAnywhere, Category="GAME SET")
	float AYisMin = 0;

	UPROPERTY(EditAnywhere, Category="GAME SET")
	float AZisMax = 300;

	UPROPERTY(EditAnywhere, Category="GAME SET")
	float AZisMin = 100;
	
	//TSubclassOf<ABalloon> BalloonClass;
	
	UPROPERTY(EditAnywhere, Category="GAME SET")
	TArray<TSubclassOf<ABalloon>> BalloonClassArray;
	//FVector()
	
	UPROPERTY()
	FTimerHandle SpawnBalloonTimer;
	
	UPROPERTY(EditAnywhere, Category="GAME SET")
	float SpawnInterval = 2.f;


	void SpawnBalloon();
};


