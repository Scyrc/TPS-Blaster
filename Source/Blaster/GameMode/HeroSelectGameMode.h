// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "../BlasterTypes/Team.h"
#include "Blaster/PlayerStart/HeroPickedStart.h"
#include "HeroSelectGameMode.generated.h"



class ABlasterCharacter;
/**
 * 
 */
UCLASS()
class BLASTER_API AHeroSelectGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	void SetHeroMesh(ETeam PlayerTeam,int32 PlayerIndex,FString PlayerName, FString HeroName);
	AHeroSelectGameMode();
	virtual void Tick(float DeltaSeconds) override;

	float LevelStartingTime = 0.f;
	
	UPROPERTY(EditAnywhere)
	float HeroPickedTime = 60.f;

	UPROPERTY(EditAnywhere)
	float CooldownTime =2.f;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	void PollInit();
	void InItTeamLocation();
	UPROPERTY()
	TMap<int32, ABlasterCharacter*> RedTeamCacheMap;
	
	UPROPERTY()
	TMap<int32, ABlasterCharacter*> BlueTeamCacheMap;

	void SaveToGameInstance(ETeam PlayerTeam, int32 PlayerIndex,FString PlayerName, FString HeroName);

	UPROPERTY(EditAnywhere)
	float LoadingTime = 5.f;
private:

	UPROPERTY(EditAnywhere)
	bool bPieEnv = false;

	UPROPERTY()
	int32 TestPlayerId = 0;

	UPROPERTY(EditAnywhere)
	int32 TestPlayerNum= 2;
	
	UPROPERTY(EditAnywhere)
	TArray<FTransform> RedLocationArray;

	UPROPERTY(EditAnywhere)
	TArray<FTransform> BlueLocationArray;
	
	UPROPERTY(EditAnywhere)
	TArray<FRotator> RotationArray;
	
	UPROPERTY()
	ABlasterCharacter* ActorSpawned;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABlasterCharacter> CharacterClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AHeroPickedStart> HeroPickedStartClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> HeroPickedCameraClass;
	
	UPROPERTY()
	int32 BlueTeamIndex = 0;

	UPROPERTY()
	int32 RedTeamIndex = 0;
	
	UPROPERTY()
	bool bServerInitTeam =false;


	float CountdownTime = 0.f;

	UPROPERTY()
	class UBlasterGameInstance* BlasterGameInstance;
};
