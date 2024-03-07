// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Engine/GameInstance.h"
#include "BlasterGameInstance.generated.h"

/**
 * 
 */

USTRUCT()
struct FPlayerInfo
{
	GENERATED_BODY()

	ETeam PlayerTeam;
	int32 PlayerTeamIndex;
	FString HeroPicked;
	
};
UCLASS()
class BLASTER_API UBlasterGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	void SetMap(const FString& SelectedMap);

	FORCEINLINE const FString& GetMapName() const{return  SelectedMapName;}
	TMap<FString, FPlayerInfo> PlayersInfo;
private:
	FString SelectedMapName;


public:
	int32 RoundNum = 1;

	int32 RedTeamScore = 0;
	int32 BlueTeamScore = 0;

	
};
