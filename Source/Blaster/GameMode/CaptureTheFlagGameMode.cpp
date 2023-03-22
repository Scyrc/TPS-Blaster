// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"

#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/Weapon/Flag.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	const bool ValidCapture = Flag->GetTeam() != Zone->Team;
	if(!ValidCapture) return;

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(GameState);
	if(BlasterGameState)
	{
		if(Zone->Team == ETeam::ET_BlueTeam)
		{
			BlasterGameState->BlueTeamScores();
		}
		else if(Zone->Team == ETeam::ET_RedTeam)
		{
			BlasterGameState->RedTeamScores();
		}
	}
	
}
