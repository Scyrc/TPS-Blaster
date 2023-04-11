// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UReturnWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	
	void MenuSetup();
	void MenuTearDown();
protected:
	virtual bool Initialize() override;
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnPlayerLeftGame();
private:
	UPROPERTY(meta=(BindWidget))
	class UButton* ReturnButton;
	
	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY()
	class APlayerController* PlayerController;
};
