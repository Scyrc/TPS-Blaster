// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();

	UFUNCTION()
	virtual void OnRep_Credits();

	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);
protected:
	virtual void CopyProperties(APlayerState* PlayerState) override;
private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = Onrep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Credits)
	int32 Credits;

	UPROPERTY(ReplicatedUsing=OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	UPROPERTY(Replicated)
	int32 PlayerTeamIndex = -1;

	UPROPERTY(Replicated)
	FString SteamPlayerName;

	UPROPERTY(Replicated)
	FString SteamPlayerId;

	UPROPERTY(Replicated)
	FString HeroName;

	

private:
	UFUNCTION()
	void OnRep_Team();

public:
	FORCEINLINE ETeam GetTeam()const{return Team;}
	void SetTeam(ETeam TeamToSet);
	//UFUNCTION(NetMulticast, Reliable)
	void SetColor();

	FORCEINLINE void SetPlayerTeamIndex(int32 Index){PlayerTeamIndex = Index;}
	FORCEINLINE int32 GetPlayerTeamIndex() const {return PlayerTeamIndex;}

	FORCEINLINE void SetSteamPlayerName(FString Name){SteamPlayerName = Name;}
	FORCEINLINE FString GetSteamPlayerName() const {return SteamPlayerName;}


	FORCEINLINE void SetSteamPlayerId(FString Id){SteamPlayerId = Id;}
	FORCEINLINE FString GetSteamPlayerId() const {return SteamPlayerId;}
	FORCEINLINE FString GetHeroName() const{ return HeroName;}
	FORCEINLINE void SetHeroName(const FString& Name){this->HeroName = Name;}

};