// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BlasterGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterGameInstance : public UGameInstance
{
	GENERATED_BODY()
private:
	FString SelectedMapName;

public:
	void SetMap(const FString& SelectedMap);
};
