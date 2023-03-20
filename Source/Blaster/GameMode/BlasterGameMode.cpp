// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}
ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
	//UE_LOG(LogTemp, Warning, TEXT("LevelStartingTime: %f"), LevelStartingTime);
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime + LevelStartingTime - GetWorld()->GetTimeSeconds();
		if(CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if(MatchState == MatchState::InProgress)
	{
		CountdownTime = MatchTime + WarmupTime + LevelStartingTime - GetWorld()->GetTimeSeconds();
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if(MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + MatchTime + WarmupTime + LevelStartingTime - GetWorld()->GetTimeSeconds();
		if(CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayerController)
		{
			BlasterPlayerController->OnMatchStateSet(MatchState);
		}
	}
	
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if(AttackerPlayerState && AttackerPlayerState!=VictimPlayerState&&BlasterGameState)
	{	TArray<ABlasterPlayerState*> PlayerCurrentlyInTheLead;
		for(auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayerCurrentlyInTheLead.Add(LeadPlayer);
		}
		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);
		if(BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlasterCharacter* LeadPlayer = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			if(LeadPlayer)
			{
				LeadPlayer->MulticastGainedTheLead();
			}
		}

		for(auto LastLeadPlayer : PlayerCurrentlyInTheLead)
		{
			if(!BlasterGameState->TopScoringPlayers.Contains(LastLeadPlayer))
			{
				ABlasterCharacter* LostLeadPlayer = Cast<ABlasterCharacter>(LastLeadPlayer->GetPawn());
				LostLeadPlayer->MulticastLostTheLead();
			}
		}
	}
	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Elim(false);
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharcter, AController* ElimmedController)
{
	if(ElimmedCharcter)
	{
		ElimmedCharcter->Reset();
		ElimmedCharcter->Destroy();
	}
	if(ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int Selection = FMath::RandRange(0, PlayerStarts.Num()-1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if(PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
	if(CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}


