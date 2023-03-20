// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/ReturnWidget.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if(InputComponent == nullptr) return;


	InputComponent->BindAction("Quit",IE_Pressed, this, &ABlasterPlayerController::ShowReturnToMainMenu);
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());

	ServerCheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}


void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHUdTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
	CheckPing(DeltaSeconds);
	
}
void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if(HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : PlayerState;
		if(PlayerState)
		{
			//UE_LOG(LogTemp, Warning, TEXT("PlayerState->GetPing()*4: %d"), PlayerState->GetPing()*4)
			if(PlayerState->GetPing()*4 > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}

	bool bHighPingAnimPlaying =
		BlasterHUD && 
		BlasterHUD->CharacterOverlay&&
		BlasterHUD->CharacterOverlay->HighPingAnim&&
		BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnim);

	if(bHighPingAnimPlaying)
	{
		PingAnimRunningTime += DeltaTime;
		if(PingAnimRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABlasterPlayerController::ShowReturnToMainMenu()
{
	// show the

	if(ReturnWidgetClass == nullptr) return;

	if(ReturnWidget == nullptr)
	{
		ReturnWidget = CreateWidget<UReturnWidget>(this, ReturnWidgetClass);
	}
	if(ReturnWidget)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if(bReturnToMainMenuOpen)
		{
			ReturnWidget->MenuSetup();
		}
		else
		{
			ReturnWidget->MenuTearDown();
		}
	}
}

void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}
void ABlasterPlayerController::PollInit()
{
	if(CharacterOverlay == nullptr && BlasterHUD && BlasterHUD->CharacterOverlay)
	{
		CharacterOverlay = BlasterHUD->CharacterOverlay;
		if(CharacterOverlay)
		{
			if(bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
			if(bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
			if(bInitializeScore) SetHUDScore(HUDScore);
			if(bInitializeDefeats) SetHUDDefeats(HUDDefeats);
			if(bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
			if(bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
			IsShowHUDWeaponAmmo(bInitializeShowAmmo);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
			if(BlasterCharacter && BlasterCharacter->GetCombatComponent()&&bInitializeGrenades)
			{
				SetHUDGrenades(BlasterCharacter->GetCombatComponent()->GetGrenades());
			}
		}
	}
	
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequertServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}



void ABlasterPlayerController::SetHUdTime()
{
	float TimeLeft = 0.f;
	if(MatchState::WaitingToStart == MatchState)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState::InProgress == MatchState)
	{
		TimeLeft = MatchTime + WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if(MatchState::Cooldown == MatchState)
	{
		TimeLeft = CooldownTime + MatchTime + WarmupTime - GetServerTime() + LevelStartingTime;
	}
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if(HasAuthority())
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if(BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime()); // + LevelStartingTime ??? 
		}
	}
	if(SecondsLeft != CountdownInt)
	{
		if(MatchState::WaitingToStart == MatchState || MatchState::Cooldown == MatchState)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if(MatchState::InProgress == MatchState)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft; 
}


void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceiveClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() -  TimeOfClientRequest;
	SingleTripTime = RoundTripTime * 0.5f; 
	float CurrentServerTime = TimeServerReceiveClientRequest + (0.5f*RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
	
}

void ABlasterPlayerController::ServerRequertServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceive = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceive);
}


void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);

	if(BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
		SetHUDShield(BlasterCharacter->GetShield(), BlasterCharacter->GetMaxShield());
		if(BlasterCharacter->GetCombatComponent())
		{
			SetHUDGrenades(BlasterCharacter->GetCombatComponent()->GetGrenades());
		}
		UCombatComponent* Combat = BlasterCharacter->GetCombatComponent();
		if(Combat && Combat->GetEquippedWeapon())
		{
			Combat->GetEquippedWeapon()->IsShowHUDAmmo(true);
			Combat->UpdateCarriedAmmo();
			Combat->GetEquippedWeapon()->SetHUDAmmo();
		}
	}
	
}

float ABlasterPlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();

	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if(IsLocalController())
	{
		ServerRequertServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay&&
		BlasterHUD->CharacterOverlay->HealthBar&&
		BlasterHUD->CharacterOverlay->HealthText;

	if(bHUDValid)
	{
		const float  HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay&&
		BlasterHUD->CharacterOverlay->ShieldBar&&
		BlasterHUD->CharacterOverlay->ShieldText;

	if(bHUDValid)
	{
		const float  ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		const FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay&&
		BlasterHUD->CharacterOverlay->ScoreAmount;

	if(bHUDValid)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}

}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay&&
		BlasterHUD->CharacterOverlay->DefeatsAmount;

	if(bHUDValid)
	{
		const FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->GrenadesText;

	if(bHUDValid)
	{
		const FString GrenadesTextText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesTextText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
	
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->WeaponAmmoAmount;

	if(bHUDValid)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->CarriedAmmoAmount;

	if(bHUDValid)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
} 

void ABlasterPlayerController::IsShowHUDWeaponAmmo(bool bShow)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->WeaponAmmoAmount&&
	BlasterHUD->CharacterOverlay->CarriedAmmoAmount&&
	BlasterHUD->CharacterOverlay->AmmoText&&
	BlasterHUD->CharacterOverlay->SlashText;

	if(bHUDValid)
	{
		if(bShow)
		{
			BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->CharacterOverlay->AmmoText->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->CharacterOverlay->SlashText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->AmmoText->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->SlashText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		bInitializeShowAmmo = bShow;
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay&&
		BlasterHUD->CharacterOverlay->MatchCountdownText;

	if(bHUDValid)
	{
		if(CountdownTime<0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		const int32 Seconds = CountdownTime - Minutes*60;
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement&&
		BlasterHUD->Announcement->WarmupTime;
	
	if(bHUDValid)
	{
		if(CountdownTime<0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		const int32 Seconds = CountdownTime - Minutes*60;
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD && IsLocalController())
	{
		BlasterHUD->AddCharacterOverlay();
		if(BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

// run on server
void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{ 
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		MatchState = GameMode->GetMatchState();
		MatchTime = GameMode->MatchTime;
		WarmupTime = GameMode->WarmupTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		CooldownTime = GameMode->CooldownTime;
		ClientJoinMidgame(MatchState, MatchTime, WarmupTime, LevelStartingTime, CooldownTime);
	}
}

// run on client
void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMacth, float Match, float Warmup, float LevelStarting, float Cooldown)
{
	MatchState = StateOfMacth;
	MatchTime = Match;
	WarmupTime = Warmup;
	LevelStartingTime = LevelStarting;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);
	if(BlasterHUD && MatchState==MatchState::WaitingToStart && BlasterHUD->Announcement==nullptr &&IsLocalController())
	{
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleMatchCooldown();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleMatchCooldown();
	}
}

void ABlasterPlayerController::HandleMatchCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		const bool bHUDValid = BlasterHUD->Announcement&&
			BlasterHUD->Announcement->AnnouncementText&&
			BlasterHUD->Announcement->InfoText;
		if(bHUDValid)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			const FString AnnouncementText("New Match Starts In:");
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if(BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				FString InfoString;
				if(TopPlayers.Num() == 0)
				{
					InfoString = FString("There is no Winner");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
				{
					InfoString = FString("You are the winner!");
				}
				else if(TopPlayers.Num() == 0)
				{
					InfoString = FString::Printf(TEXT("Winner: %s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 0)
				{
					InfoString = FString("Players tied for the win: \n");
					for(auto TiedPlayer : TopPlayers)
					{
						InfoString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoString));
			}
		}
	}
	
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if(BlasterCharacter && BlasterCharacter->GetCombatComponent())
	{
		BlasterCharacter->bDisableGamePlay = true;
		BlasterCharacter->GetCombatComponent()->FireButtonPressed(false);
	}
	
}

void ABlasterPlayerController::HighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->HighPingImage&&
	BlasterHUD->CharacterOverlay->HighPingAnim;

	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(
			BlasterHUD->CharacterOverlay->HighPingAnim,
			0.f,
			5);
	}
}

void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->HighPingImage&&
	BlasterHUD->CharacterOverlay->HighPingAnim;
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if(BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnim))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnim);
		}
	}
}


