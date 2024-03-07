// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HeroSelectHUD.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHeroSelectHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Player HUDClass")
	TSubclassOf<UUserWidget> HeroSelectWidgetClass;
	UPROPERTY()
	class UHeroSelectWidget* HeroSelectWidget;
	void AddHeroSelectWidget();

	void RemoveHeroSelectWidget();
	
};
