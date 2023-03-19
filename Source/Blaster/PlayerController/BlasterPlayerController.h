// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);

	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void IsShowHUDWeaponAmmo(bool bShow);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);

	virtual void OnPossess(APawn* InPawn) override;
	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	float SingleTripTime = 0.f;
	FHighPingDelegate HighPingDelegate;
protected:
	virtual void SetupInputComponent() override;

	void SetHUdTime();
	void PollInit();
	/*
	 * Sync time between client and server
	 */
	UFUNCTION(Server, Reliable)
	void ServerRequertServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceiveClientRequest);

	float ClientServerDelta = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category="Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime =0.f;
	void CheckTimeSync(float DeltaTime);

	void HandleMatchHasStarted();
	void HandleMatchCooldown();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMacth, float Match, float Warmup, float LevelStarting, float Cooldown);

	void HighPingWarning(); 
	void StopHighPingWarning();

	void CheckPing(float DeltaTime);

	void ShowReturnToMainMenu();

private:
	/*
	 * Return to main menu
	 */
	UPROPERTY(EditAnywhere, Category="HUD")
	TSubclassOf<class UUserWidget> ReturnWidgetClass;
	
	UPROPERTY()
	class UReturnWidget* ReturnWidget;

	bool bReturnToMainMenuOpen = false;
	
	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;
	
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	float LevelStartingTime =0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;

	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState();
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeHealth = false;
	float HUDShield;
	float HUDMaxShield;
	bool bInitializeShield = false;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	int32 HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	int32 HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
	bool bInitializeShowAmmo = false;
	
	float HighPingRunningTime = 0.f;
	float PingAnimRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;
	
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;
	
	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
};
