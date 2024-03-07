// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MapSelectWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UMapSelectWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath);
protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	//
	// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
private:
	UFUNCTION(BlueprintCallable)
	void setMap(const FString& MapName);
	
	UPROPERTY(meta=(BindWidget))
	class UButton* BombMapButton;

	UFUNCTION()
	void BombMapButtonClicked();


	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
	UButton* NameButton;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameText;
	
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void GetNameClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();
	
	// The subsystem designed to handle all online session functionality
	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections{4};

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchType{TEXT("FreeForAll")};

	FString PathToLobby{TEXT("")};

};
