// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameInstance.h"

void UBlasterGameInstance::SetMap(const FString& SelectedMap)
{
	SelectedMapName = SelectedMap;
	UE_LOG(LogTemp, Warning, TEXT("Player select MAP: %s"), *SelectedMap);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Player select MAP: %s"), *SelectedMap));
}
