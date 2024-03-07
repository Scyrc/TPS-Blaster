// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern BLASTER_API const FName Warmup; // warmup time for player to buy guns & actions, discuss
	extern BLASTER_API const FName Round; // Match duration pre round
	extern BLASTER_API const FName Calculate; // 
	extern BLASTER_API const FName Cooldown; // Display winner pre round
	extern BLASTER_API const FName GameSummary; // Game over.Display final winner or team
}

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void PlayerEliminated(class ABlasterCharacter* EliminatedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharcter, class AController* ElimmedController);

	void PlayerLeftGame(class ABlasterPlayerState* BlasterPlayerState);

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float Damage);

	UPROPERTY(EditDefaultsOnly)
	float LoadingTime = 5.f;
	
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 4.f;

	UPROPERTY(EditDefaultsOnly)
	float GameSummaryTime = 10.f;
	
	UPROPERTY(EditDefaultsOnly)
	int32 MaxRoundNum = 10;

	UPROPERTY()
	int32 RoundNum;
	
	float LevelStartingTime = 0.f;
	FORCEINLINE float GetCountdownTime() const{return  CountdownTime;}
	
	UPROPERTY(EditAnywhere)
	bool bTeamsMatch =false;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMatchStateSet() override;
	virtual void CalculateMsg();

	
private:
	float CountdownTime = 0.f;


};
