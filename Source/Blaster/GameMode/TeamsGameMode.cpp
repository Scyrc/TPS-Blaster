// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"

#include "Blaster/GameInstance/BlasterGameInstance.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"
ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ATeamsGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ATeamsGameMode::PollInit()
{
	if(bServerInit) return;
	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BlasterGameState)
	{
		for(auto PlayerState : BlasterGameState->PlayerArray)
		{
			ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(PlayerState.Get());
			const auto BlasterController =  BlasterPlayerState->GetPlayerController();
			if(BlasterController->IsLocalController())// listen server
			{
				if(BlasterPlayerState->GetTeam() == ETeam::ET_RedTeam)
				{
					BlasterGameState->RedTeam.AddUnique(BlasterPlayerState);
					BlasterPlayerState->SetColor();
				}
				else if(BlasterPlayerState->GetTeam() == ETeam::ET_BlueTeam)
				{
					BlasterGameState->BlueTeam.AddUnique(BlasterPlayerState);
					BlasterPlayerState->SetColor();
				}
				else  // no team
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
	
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	
	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(GameState);
	UBlasterGameInstance* BlasterGameInstance = Cast<UBlasterGameInstance>(GetGameInstance());
	if(BlasterGameInstance && BlasterGameState)
	{
		BlasterGameState->RedTeamScore = BlasterGameInstance->RedTeamScore;
		BlasterGameState->BlueTeamScore = BlasterGameInstance->BlueTeamScore;
	}
	
	if(BlasterGameState)
	{
		for(auto PlayerState : BlasterGameState->PlayerArray)
		{
			ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(PlayerState.Get());
			if(BlasterPlayerState && BlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				FString LogMsg = FString::Printf(TEXT("No team ,player : %s, teamIndex : %d,id: %s"), *BlasterPlayerState->GetSteamPlayerName(), BlasterPlayerState->GetPlayerTeamIndex(), *BlasterPlayerState->GetSteamPlayerId());
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Purple, LogMsg);
			}
			else if(BlasterPlayerState && BlasterPlayerState->GetTeam() == ETeam::ET_RedTeam)
			{
				FString LogMsg = FString::Printf(TEXT("Red team ,player : %s, teamIndex : %d,id: %s"), *BlasterPlayerState->GetSteamPlayerName(), BlasterPlayerState->GetPlayerTeamIndex(), *BlasterPlayerState->GetSteamPlayerId());
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Purple, LogMsg);
			}
			else
			{
				FString LogMsg = FString::Printf(TEXT("Blue team ,player : %s, teamIndex : %d, id: %s"), *BlasterPlayerState->GetSteamPlayerName(), BlasterPlayerState->GetPlayerTeamIndex(), *BlasterPlayerState->GetSteamPlayerId());
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Purple, LogMsg);
			}
		}
	}
}

void ATeamsGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

}

void ATeamsGameMode::CalculateMsg()
{
	UE_LOG(LogTemp, Warning, TEXT("ATeamsGameMode::CalculateMsg"));
	Super::CalculateMsg();
	SetScore();
}

void ATeamsGameMode::SetScore()
{
	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BlasterGameState)
	{
		float RedTeamScores = 0.f;
		float BlueTeamScores = 0.f;

		for (const auto PlayerState : BlasterGameState->RedTeam)
		{
			RedTeamScores += PlayerState->GetScore();
		}

		for (const auto PlayerState : BlasterGameState->BlueTeam)
		{
			BlueTeamScores += PlayerState->GetScore();
		}
		if(RedTeamScores > BlueTeamScores)
			BlasterGameState->RedTeamScores();
		else if (RedTeamScores < BlueTeamScores)
			BlasterGameState->BlueTeamScores();
		else
		{
			BlasterGameState->RedTeamScores();
			BlasterGameState->BlueTeamScores();
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
		if(BlasterPlayerState == nullptr) return;
		if(BlasterPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BlasterGameState->RedTeam.AddUnique(BlasterPlayerState);
			BlasterPlayerState->SetColor();
		}
		else if(BlasterPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BlasterGameState->BlueTeam.AddUnique(BlasterPlayerState);
			BlasterPlayerState->SetColor();
		}
		else  // no team
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

void ATeamsGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);

	/*ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	if(BlasterGameState && AttackerPlayerState)
	{
		if(AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BlasterGameState->RedTeamScores();
		}
		else if(AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BlasterGameState->BlueTeamScores();
		}
	}*/
}
