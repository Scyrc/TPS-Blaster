// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"


void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, Team);
	DOREPLIFETIME(ABlasterPlayerState, Credits);
	DOREPLIFETIME(ABlasterPlayerState, PlayerTeamIndex);
	DOREPLIFETIME(ABlasterPlayerState, SteamPlayerName);
	DOREPLIFETIME(ABlasterPlayerState, SteamPlayerId);
	DOREPLIFETIME(ABlasterPlayerState, HeroName);

}

// run on server
void ABlasterPlayerState::AddToScore(float ScoreAmount) 
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			 Controller->SetHUDScore(GetScore());
		}
	}
}

// run on client
void ABlasterPlayerState::OnRep_Score()  
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	ABlasterPlayerState* NewPlayerState = Cast<ABlasterPlayerState>(PlayerState);
	if(NewPlayerState)
	{
		NewPlayerState->SteamPlayerId = SteamPlayerId;
		NewPlayerState->SteamPlayerName = SteamPlayerName;
		NewPlayerState->Team = Team;
		NewPlayerState->PlayerTeamIndex = PlayerTeamIndex;
		NewPlayerState->Credits = Credits;
		NewPlayerState->Defeats = Defeats;
		NewPlayerState->HeroName = HeroName;
	}
}

void ABlasterPlayerState::OnRep_Team()
{
	ABlasterCharacter * BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if(BlasterCharacter)
	{
		BlasterCharacter->SetTeamColor(Team);
	}
}

void ABlasterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;
	ABlasterCharacter * BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if(BlasterCharacter)
	{
		BlasterCharacter->SetTeamColor(Team);
	}
}



void ABlasterPlayerState::SetColor()
{
	ABlasterCharacter * BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if(BlasterCharacter)
	{
		BlasterCharacter->SetPickedColor(HeroName);
	}
}

void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::OnRep_Credits()
{
	
}
