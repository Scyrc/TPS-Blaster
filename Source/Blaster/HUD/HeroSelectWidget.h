// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HeroSelectWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UHeroSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable)
	void OnClicked(FString HeroName);
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HeroPickCountdownText;
protected:
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
private:
	void MenuTearUp();

	void MenuTearDown();
};
