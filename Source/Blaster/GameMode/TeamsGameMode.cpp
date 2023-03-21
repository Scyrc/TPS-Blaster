// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"

#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BlasterGameState)
	{
		for(auto PlayerState : BlasterGameState->PlayerArray)
		{
			ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(PlayerState.Get());
			if(BlasterPlayerState && BlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if(BlasterGameState->BlueTeam.Num() >= BlasterGameState->RedTeam.Num())
				{
					BlasterGameState->RedTeam.AddUnique(BlasterPlayerState);
					BlasterPlayerState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BlasterGameState->BlueTeam.AddUnique(BlasterPlayerState);
					BlasterPlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BlasterGameState)
	{
		ABlasterPlayerState* BlasterPlayerState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState && BlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if(BlasterGameState->BlueTeam.Num() >= BlasterGameState->RedTeam.Num())
			{
				BlasterGameState->RedTeam.AddUnique(BlasterPlayerState);
				BlasterPlayerState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BlasterGameState->BlueTeam.AddUnique(BlasterPlayerState);
				BlasterPlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* BlasterPlayerState = Exiting->GetPlayerState<ABlasterPlayerState>();

		if(BlasterPlayerState && BlasterGameState)
		{
			if(BlasterGameState->BlueTeam.Contains(BlasterPlayerState))
			{
				BlasterGameState->BlueTeam.Remove(BlasterPlayerState);
			}
			else if(BlasterGameState->RedTeam.Contains(BlasterPlayerState))
			{
				BlasterGameState->RedTeam.Remove(BlasterPlayerState);
			}
		}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float Damage)
{
	//return Super::CalculateDamage(Attacker, Victim, Damage);

	ABlasterPlayerState* AttckerPlayerState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPlayerState = Victim->GetPlayerState<ABlasterPlayerState>();

	if(AttckerPlayerState == nullptr || VictimPlayerState == nullptr) return Damage;
	if(VictimPlayerState == AttckerPlayerState) return Damage;
	if(AttckerPlayerState->GetTeam() == VictimPlayerState->GetTeam()) return 0.f;
	return Damage;
}
