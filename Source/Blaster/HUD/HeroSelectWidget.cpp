// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroSelectWidget.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Blaster/PlayerController/HeroSelectController.h"

class UMultiplayerSessionsSubsystem;

void UHeroSelectWidget::OnClicked(FString HeroName)
{
	UE_LOG(LogTemp, Warning, TEXT("UHeroSelectWidget Called"));

	AHeroSelectController* HeroSelectController = Cast<AHeroSelectController>(GetOwningPlayer());

	// todo fix
	
	if(HeroSelectController)
	{
		HeroSelectController->SetMesh(HeroName);
	}
}

bool UHeroSelectWidget::Initialize()
{
	return Super::Initialize();
}

void UHeroSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	MenuTearUp();
}

void UHeroSelectWidget::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UHeroSelectWidget::MenuTearUp()
{
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

void UHeroSelectWidget::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
