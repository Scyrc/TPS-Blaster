// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/BlasterTypes/Team.h"

#include "HeroPickedStart.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHeroPickedStart : public APlayerStart
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	ETeam Team;
};
