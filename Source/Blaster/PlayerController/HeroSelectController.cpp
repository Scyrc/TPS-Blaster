// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroSelectController.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameMode/HeroSelectGameMode.h"
#include "Blaster/HUD/HeroSelectHUD.h"
#include "Blaster/HUD/HeroSelectWidget.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AHeroSelectController::AHeroSelectController()
{
	
}

void AHeroSelectController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUdTime();

	CheckTimeSync(DeltaSeconds);

}

void AHeroSelectController::BeginPlay()
{
	Super::BeginPlay();

	HeroSelectHUD = Cast<AHeroSelectHUD>(GetHUD());
	ServerCheckMatchState();
}

float AHeroSelectController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();

	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AHeroSelectController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequertServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AHeroSelectController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHeroSelectController, MatchState);
}

void AHeroSelectController::ClearHUD_Implementation()
{
	HeroSelectHUD = HeroSelectHUD == nullptr ?  Cast<AHeroSelectHUD>(GetHUD()) : HeroSelectHUD;
	if(HeroSelectHUD && IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("GO TO delete"));

		HeroSelectHUD->RemoveHeroSelectWidget();
	}
}

void AHeroSelectController::SetHUdTime()
{
	float TimeLeft = 0.f;
	
	TimeLeft = HeroPickedTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);


	if(SecondsLeft != CountdownInt)
	{
		SetHUDMatchCountdown(TimeLeft);
	}
	CountdownInt = SecondsLeft; 
}


void AHeroSelectController::OnMatchStateSet_Implementation(FName State)
{
	MatchState = State;
	if(MatchState == MatchState::InProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("InProgress"));
		HeroSelectHUD = HeroSelectHUD == nullptr ?  Cast<AHeroSelectHUD>(GetHUD()) : HeroSelectHUD;
		if(HeroSelectHUD && IsLocalController())
		{
			UE_LOG(LogTemp, Warning, TEXT("GO TO CREATE"));

			HeroSelectHUD->AddHeroSelectWidget();
		}
		
		TArray<AActor*> PlayerStart;
		UGameplayStatics::GetAllActorsOfClass(this, HeroPickedCameraClass, PlayerStart);

		if(PlayerStart.Num() > 0)
		{
			this->SetViewTargetWithBlend(PlayerStart[0]);
			UE_LOG(LogTemp, Warning, TEXT("SetViewTarget"));
		}
	}

	
}


void AHeroSelectController::PollInit()
{

}

void AHeroSelectController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("InProgress"));
		HeroSelectHUD = HeroSelectHUD == nullptr ?  Cast<AHeroSelectHUD>(GetHUD()) : HeroSelectHUD;
		if(HeroSelectHUD && IsLocalController())
		{
			UE_LOG(LogTemp, Warning, TEXT("GO TO CREATE"));

			HeroSelectHUD->AddHeroSelectWidget();
		}
	}
}

void AHeroSelectController::SetHUDMatchCountdown(float CountdownTime)
{
	HeroSelectHUD = HeroSelectHUD == nullptr ? Cast<AHeroSelectHUD>(GetHUD()) : HeroSelectHUD;
	bool bHUDValid = HeroSelectHUD &&
		HeroSelectHUD->HeroSelectWidget&&
		HeroSelectHUD->HeroSelectWidget->HeroPickCountdownText;

	if(bHUDValid)
	{
		if(CountdownTime<0.f)
		{
			HeroSelectHUD->HeroSelectWidget->HeroPickCountdownText->SetText(FText());
			return;
		}
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		const int32 Seconds = CountdownTime - Minutes*60;
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		HeroSelectHUD->HeroSelectWidget->HeroPickCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AHeroSelectController::ClientReportServerTime_Implementation(float TimeOfClientRequest,float TimeServerReceiveClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() -  TimeOfClientRequest;
	SingleTripTime = RoundTripTime * 0.5f; 
	float CurrentServerTime = TimeServerReceiveClientRequest + (0.5f*RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AHeroSelectController::ServerRequertServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceive = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceive);
}

void AHeroSelectController::ClientJoinMidgame_Implementation(FName StateOfMatch, float LevelStart, float HeroPicked)
{
	MatchState = StateOfMatch;
	LevelStartingTime = LevelStart;
	HeroPickedTime = HeroPicked;
	OnMatchStateSet(MatchState);
}

void AHeroSelectController::ServerCheckMatchState_Implementation()
{
	AHeroSelectGameMode* GameMode = Cast<AHeroSelectGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		MatchState = GameMode->GetMatchState();
		LevelStartingTime = GameMode->LevelStartingTime;
		HeroPickedTime = GameMode->HeroPickedTime;
		ClientJoinMidgame(MatchState, LevelStartingTime, HeroPickedTime);
	}
}


void AHeroSelectController::SetMesh_Implementation(const FString& HeroName)
{
	UE_LOG(LogTemp, Warning, TEXT("AHeroSelectController Called"));

	AHeroSelectGameMode* GameMode = Cast<AHeroSelectGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		/*FPlayerMsgStruct PlayerMsg;
		UGameInstance* GameInstance = GetGameInstance();
		if (GameInstance)
		{
			auto MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
			if (MultiplayerSessionsSubsystem)
			{
				PlayerMsg =  MultiplayerSessionsSubsystem->GetPlayerName(0);
			}
		}*/
		ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState)
		{
			BlasterPlayerState->SetHeroName(HeroName);
			GameMode->SetHeroMesh(BlasterPlayerState->GetTeam(), BlasterPlayerState->GetPlayerTeamIndex(), BlasterPlayerState->GetSteamPlayerName(), HeroName);
		}
	}
}