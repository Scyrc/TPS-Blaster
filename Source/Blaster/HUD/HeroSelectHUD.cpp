// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroSelectHUD.h"

#include "HeroSelectWidget.h"
#include "Blueprint/UserWidget.h"

void AHeroSelectHUD::AddHeroSelectWidget()
{
	UE_LOG(LogTemp, Warning, TEXT("AddHeroSelectWidget"));

	const APlayerController* PlayerController = GetOwningPlayerController();

	if(PlayerController && HeroSelectWidgetClass)
	{
		if(HeroSelectWidget)
		{
			HeroSelectWidget->RemoveFromParent();
		}

		HeroSelectWidget = CreateWidget<UHeroSelectWidget>(GetWorld(), HeroSelectWidgetClass);
		if(HeroSelectWidget)
		{
			HeroSelectWidget->AddToViewport();
		}
	}
}

void AHeroSelectHUD::RemoveHeroSelectWidget()
{
	if(HeroSelectWidget)
	{
		HeroSelectWidget->RemoveFromParent();
	}
}
