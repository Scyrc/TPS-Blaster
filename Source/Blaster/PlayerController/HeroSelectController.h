// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HeroSelectController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHeroSelectController : public APlayerController
{
	GENERATED_BODY()

public:
	AHeroSelectController();
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual float GetServerTime();
	void CheckTimeSync(float DeltaTime);
	UFUNCTION(Client, Reliable)
	void OnMatchStateSet(FName State);

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void SetMesh(const FString& HeroName);
	
	UFUNCTION(Client, Reliable)
	void ClearHUD();
private:
	void SetHUdTime();
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> HeroPickedCameraClass;
	
	UPROPERTY()
	class AHeroSelectHUD* HeroSelectHUD;

	UPROPERTY()
	class UHeroSelectWidget* HeroSelectWidget;

	void PollInit();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float LevelStart, float HeroPicked);

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	float LevelStartingTime =0.f;
	float HeroPickedTime = 0.f;

	uint32 CountdownInt = 0;
	float ClientServerDelta = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category="Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime =0.f;

	UFUNCTION(Server, Reliable)
	void ServerRequertServerTime(float TimeOfClientRequest);
	
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceiveClientRequest);
	
	float SingleTripTime = 0.f;
	void SetHUDMatchCountdown(float CountdownTime);
	
	UPROPERTY()
	class AHeroSelectGameMode* HeroSelectGameMode;

};
