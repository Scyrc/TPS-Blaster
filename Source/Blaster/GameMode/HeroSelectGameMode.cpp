// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroSelectGameMode.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameInstance/BlasterGameInstance.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/HeroSelectController.h"
#include "Blaster/PlayerStart/HeroPickedStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"


AHeroSelectGameMode::AHeroSelectGameMode()
{
	
	
}

void AHeroSelectGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();

}

void AHeroSelectGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = LoadingTime + LevelStartingTime - GetWorld()->GetTimeSeconds();
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::InProgress);
		}
	}

	if(MatchState == MatchState::InProgress)
	{
		PollInit();
		InItTeamLocation();
		
		//SetMatchState(MatchState::Warmup);
		CountdownTime = CooldownTime + HeroPickedTime + LevelStartingTime - GetWorld()->GetTimeSeconds();
		if(CountdownTime <= 0.f) // hero picked end , go to map
		{
			for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
			{
				
				AHeroSelectController* HeroSelectController = Cast<AHeroSelectController>(*It);
				if(HeroSelectController)
				{
					HeroSelectController->ClearHUD();
				}
			}
			
			BlasterGameInstance = BlasterGameInstance == nullptr?  Cast<UBlasterGameInstance>(GetGameInstance()) : BlasterGameInstance;
			
			if(BlasterGameInstance)
			{
				UWorld* World = GetWorld();
				if(World)
				{
					bUseSeamlessTravel = true;
					if(BlasterGameInstance->GetMapName() == "BlasterMap" || bPieEnv)
					{
						World->ServerTravel(FString("/Game/maps/BlasterMap?listen"));
					}
					if(BlasterGameInstance->GetMapName() == "BalloonMap" || bPieEnv)
					{
						World->ServerTravel(FString("/Game/maps/balloon/Balloon?listen"));
					}
				}
			}
		}
	}
}
void AHeroSelectGameMode::SaveToGameInstance(ETeam PlayerTeam, int32 PlayerIndex, FString PlayerName, FString HeroName)
{
	BlasterGameInstance = BlasterGameInstance == nullptr?  Cast<UBlasterGameInstance>(GetGameInstance()) : BlasterGameInstance;

	FString SaveKey = FString::Printf(TEXT("%ls_%hhd_%d"), *PlayerName, PlayerTeam, PlayerIndex);
	FPlayerInfo SaveInfo;
	SaveInfo.PlayerTeam = PlayerTeam;
	SaveInfo.PlayerTeamIndex = PlayerIndex;
	SaveInfo.HeroPicked = HeroName;
	
	check(BlasterGameInstance);
	BlasterGameInstance->PlayersInfo.Emplace(SaveKey, SaveInfo);
}

void AHeroSelectGameMode::SetHeroMesh(ETeam PlayerTeam, int32 PlayerIndex,FString PlayerName, FString HeroName)
{
	FString LogMsg = FString::Printf(TEXT("GameMode Called PlayerTeam: %hhd, PlayerIndex: %d PlayerName: %s, HeroName: %s"), PlayerTeam, PlayerIndex, *PlayerName, *HeroName);
	UE_LOG(LogTemp, Warning, TEXT("GameMode Called PlayerTeam: %hhd, PlayerIndex: %d PlayerName: %s, HeroName: %s"), PlayerTeam, PlayerIndex, *PlayerName, *HeroName);
	
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, LogMsg);

	SaveToGameInstance(PlayerTeam,  PlayerIndex, PlayerName,  HeroName);
	
	if (CharacterClass)
	{
		UWorld* MyWorld = GetWorld();
		
		if (MyWorld)
		{
			if(PlayerTeam == ETeam::ET_RedTeam &&  RedLocationArray.Num() > 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("GameMode Called: Spawn RedTeam actor"));
				
				if(RedTeamCacheMap.Contains(PlayerIndex))
				{
					RedTeamCacheMap[PlayerIndex]->Destroy();
					RedTeamCacheMap.Remove(PlayerIndex);
				}
				ActorSpawned = MyWorld->SpawnActor<ABlasterCharacter>(
					CharacterClass,
					RedLocationArray[FMath::Clamp(PlayerIndex, 0, RedLocationArray.Num())]
					);
				RedTeamCacheMap.Emplace(PlayerIndex, ActorSpawned);
					//ActorSpawned->PlayElimMontage();
				ActorSpawned->SetPickedColor(HeroName);
			}
			else if(PlayerTeam == ETeam::ET_BlueTeam &&  BlueLocationArray.Num() > 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("GameMode Called: Spawn BlueTeam actor"));
				
				if(BlueTeamCacheMap.Contains(PlayerIndex))
				{
					BlueTeamCacheMap[PlayerIndex]->Destroy();
					BlueTeamCacheMap.Remove(PlayerIndex);
				}
				ActorSpawned = MyWorld->SpawnActor<ABlasterCharacter>(
					CharacterClass,
					BlueLocationArray[FMath::Clamp(PlayerIndex, 0, BlueLocationArray.Num())]
					);
				BlueTeamCacheMap.Emplace(PlayerIndex, ActorSpawned);
				ActorSpawned->SetPickedColor(HeroName);
			}
		}
	}
}

void AHeroSelectGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnMatchStateSet"));

		AHeroSelectController* HeroSelectController = Cast<AHeroSelectController>(*It);
		if(HeroSelectController)
		{
			HeroSelectController->OnMatchStateSet(MatchState);
		}
	}
	
}

void AHeroSelectGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	FString LogMsg = FString::Printf(TEXT("PostLogin........"));
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, LogMsg);
	
	auto BlasterPlayerState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
	
	BlasterGameInstance = BlasterGameInstance == nullptr?  Cast<UBlasterGameInstance>(GetGameInstance()) : BlasterGameInstance;
	if (BlasterGameInstance)
	{
		const auto MultiplayerSessionsSubsystem = BlasterGameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			const auto PlayerMsg =  MultiplayerSessionsSubsystem->GetPlayerName(0);
			if(BlasterPlayerState)
			{
				BlasterPlayerState->SetSteamPlayerName(PlayerMsg.PlayerName);
				BlasterPlayerState->SetSteamPlayerId(PlayerMsg.PlayerId);
			}
		}
		else if(bPieEnv) // 测试环境
		{
			FString PlayerId = FString::Printf(TEXT("%d"), TestPlayerId);
			FString PlayerName = FString::Printf(TEXT("player_%d"), TestPlayerId);
		
			BlasterPlayerState->SetSteamPlayerName(PlayerName);
			BlasterPlayerState->SetSteamPlayerId(PlayerId);
		}
	}
	if(BlasterPlayerState &&  BlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
	{
		if(RedTeamIndex <= BlueTeamIndex)
		{
			BlasterPlayerState->SetTeam(ETeam::ET_RedTeam);
			BlasterPlayerState->SetPlayerTeamIndex(RedTeamIndex++); // min: 0
		}
		else
		{
			BlasterPlayerState->SetTeam(ETeam::ET_BlueTeam);
			BlasterPlayerState->SetPlayerTeamIndex(BlueTeamIndex++); // min: 0
		}
	}
	
	//AHeroSelectController* Controller = Cast<AHeroSelectController>(NewPlayer);

	/*TArray<AActor*> PlayerStart;
	UGameplayStatics::GetAllActorsOfClass(this, HeroPickedCameraClass, PlayerStart);

	if(PlayerStart.Num() > 0)
	{
		NewPlayer->SetViewTargetWithBlend(PlayerStart[0]);
		UE_LOG(LogTemp, Warning, TEXT("client SetViewTarget"));
	}*/
}

void AHeroSelectGameMode::PollInit()
{
	if(!bServerInitTeam)
	{
		if(GameState)
		{
			for(auto PlayerState : GameState->PlayerArray)
			{
				
				auto Controller =  PlayerState->GetPlayerController();
				ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(PlayerState.Get());

				if(Controller && Controller->IsLocalController())// listen server
				{
					FString LogMsg = FString::Printf(TEXT("Listen Server PostLogin........"));
					GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, LogMsg);
					//TArray<AActor*> PlayerStart;
					//UGameplayStatics::GetAllActorsOfClass(this, HeroPickedCameraClass, PlayerStart);
					/*if(PlayerStart.Num() > 0)
					{
						BlasterController->SetViewTargetWithBlend(PlayerStart[0]);
						UE_LOG(LogTemp, Warning, TEXT("server SetViewTarget "));
					}*/
					BlasterGameInstance = BlasterGameInstance == nullptr?  Cast<UBlasterGameInstance>(GetGameInstance()) : BlasterGameInstance;
					if (BlasterGameInstance)
					{
						const auto MultiplayerSessionsSubsystem = BlasterGameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
						if (MultiplayerSessionsSubsystem)
						{
							const auto PlayerMsg =  MultiplayerSessionsSubsystem->GetPlayerName(0);

							if(BlasterPlayerState)
							{
								BlasterPlayerState->SetSteamPlayerName(PlayerMsg.PlayerName);
								BlasterPlayerState->SetSteamPlayerId(PlayerMsg.PlayerId);
							}
						}
						else if(bPieEnv) // 测试环境
						{
							FString PlayerId = FString::Printf(TEXT("%d"), TestPlayerId);
							FString PlayerName = FString::Printf(TEXT("player_%d"), TestPlayerId);
		
							BlasterPlayerState->SetSteamPlayerName(PlayerName);
							BlasterPlayerState->SetSteamPlayerId(PlayerId);
						}
					}
					
					if(BlasterPlayerState &&  BlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
					{
						if(RedTeamIndex <= BlueTeamIndex)
						{
							BlasterPlayerState->SetTeam(ETeam::ET_RedTeam);
							BlasterPlayerState->SetPlayerTeamIndex(RedTeamIndex++); // min: 0
							
						}
						else
						{
							BlasterPlayerState->SetTeam(ETeam::ET_BlueTeam);
							BlasterPlayerState->SetPlayerTeamIndex(BlueTeamIndex++); // min: 0
						}
						bServerInitTeam = true;
					}
					bServerInitTeam = true;
					if(BlasterPlayerState)
					{
						FString LogMsg1 = FString::Printf(TEXT("playerInfo name:%s,id:%s,team:%hhd,teamindex:%d"),
						*BlasterPlayerState->GetSteamPlayerName(),
						*BlasterPlayerState->GetSteamPlayerId(),
						BlasterPlayerState->GetTeam(),
						BlasterPlayerState->GetPlayerTeamIndex());
						GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Orange, LogMsg1);
					}
				}
			}
		}
	}
}

void AHeroSelectGameMode::InItTeamLocation()
{
	if(BlueLocationArray.Num() >0 && RedLocationArray.Num() > 0) return;
	TArray<AActor*> PlayerStart;
	UGameplayStatics::GetAllActorsOfClass(this, HeroPickedStartClass, PlayerStart);

	
	for(auto& Start : PlayerStart)
	{
		AHeroPickedStart* PickedStart = Cast<AHeroPickedStart>(Start);
		if(PickedStart)
		{
			if(PickedStart->Team == ETeam::ET_RedTeam)
			{
				RedLocationArray.Emplace(PickedStart->GetTransform());
			}
			else if(PickedStart->Team == ETeam::ET_BlueTeam)
			{
				BlueLocationArray.Emplace(PickedStart->GetTransform());
			}
		}
	}
}


