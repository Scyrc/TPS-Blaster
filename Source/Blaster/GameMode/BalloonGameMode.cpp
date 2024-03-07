// Fill out your copyright notice in the Description page of Project Settings.


#include "BalloonGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Iteams/Balloon.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"

ABalloonGameMode::ABalloonGameMode()
{
	
}

void ABalloonGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABalloonGameMode::BeginPlay()
{
	Super::BeginPlay();
	//GetWorldTimerManager().SetTimer(SpawnBalloonTimer, this, &ABalloonGameMode::SpawnBalloon, SpawnInterval);
	//GetGameInstance()->GetTimerManager();
	GetWorld()->GetTimerManager().SetTimer(SpawnBalloonTimer, this, &ABalloonGameMode::SpawnBalloon, SpawnInterval);
}
void ABalloonGameMode::SpawnBalloon()
{
	if(CurrentBallNum < MaxBallNum)
	{
		CurrentBallNum += 1;
		FVector Location = FVector(FMath::RandRange(AXisMin, AXisMax), FMath::RandRange(AYisMin, AYisMax), FMath::RandRange(AZisMin, AZisMax) );
		FTransform BalloonTransform;
		BalloonTransform.SetLocation(Location);
		const int32 selectIndex = FMath::RandRange(0, BalloonClassArray.Num() -1);
		GetWorld()->SpawnActor<ABalloon>(BalloonClassArray[selectIndex], BalloonTransform);
	}

	GetWorld()->GetTimerManager().SetTimer(SpawnBalloonTimer, this, &ABalloonGameMode::SpawnBalloon, SpawnInterval);
}
float ABalloonGameMode::CalculateDamage(AController* Attacker, AController* Victim, float Damage)
{
	return 0;
}

void ABalloonGameMode::PlayerHitBalloon(AActor* OwnerCharacter, ABlasterCharacter* Shooter)
{
	UE_LOG(LogTemp, Warning, TEXT("PlayerHitBalloon Called"));
	//CurrentBallNum -= 1;
	
	const auto PlayerState = Cast<ABlasterPlayerState>(Shooter->GetPlayerState());

	PlayerState->AddToScore(1);
	//ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	/*if(PlayerState && BlasterGameState)
	{
		if(PlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BlasterGameState->RedTeamScores();
		}
		else if(PlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BlasterGameState->BlueTeamScores();
		}
	}*/
}

void ABalloonGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	
}
