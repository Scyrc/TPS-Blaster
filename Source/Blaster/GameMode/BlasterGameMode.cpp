// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
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
