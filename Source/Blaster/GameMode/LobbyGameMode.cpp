// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		//check(Subsystem);
		if (Subsystem && NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;
				FString MatchType = Subsystem->DesiredMatchType;
				if(MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/maps/BlasterMap?listen"));
				}
				else if(MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/maps/HeroPickedMap?listen"));
				}
				else if(MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/maps/CaptureTheFlag?listen"));
				}
			}
		}
	}
}

void ALobbyGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(!bLeave && bPieEnv)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/maps/HeroPickedMap?listen"));
			bLeave = true;
		}
	}
	UGameInstance* GameInstance = GetGameInstance();
	if(GameState && GameInstance)
	{
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		//check(Subsystem);

		if (Subsystem && NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;
				FString MatchType = Subsystem->DesiredMatchType;
				if(MatchType == "FreeForAll" || MatchType == "BlasterMap")
				{
					World->ServerTravel(FString("/Game/maps/BlasterMap?listen"));
				}
				else if(MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/maps/HeroPickedMap?listen"));
				}
				else if(MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/maps/CaptureTheFlag?listen"));
				}
			}
		}
	}
}
