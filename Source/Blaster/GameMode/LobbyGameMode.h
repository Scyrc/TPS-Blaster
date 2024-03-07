// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 5
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void Tick(float DeltaSeconds) override;
private:
	UPROPERTY(EditDefaultsOnly)
	int32 PlayersNum = 1;

	UPROPERTY(EditAnywhere)
	bool bPieEnv = false;

	bool bLeave = false;
};
