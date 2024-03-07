// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterTypes/Announcement.h"
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
	DOREPLIFETIME(ABlasterPlayerController, bShowTeamScores);
	//DOREPLIFETIME(ABlasterPlayerController, LevelStartingTime);

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

void ABlasterPlayerController::OnRep_bShowTeamScores()
{
	UE_LOG(LogTemp, Warning, TEXT("bShowTeamScores change Client value: %hhd"), bShowTeamScores);
	if(bShowTeamScores)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Client going to  update TeamScores"));
		InitTeamScore();
	}
	else
	{
		HideTeamScore();
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



void ABlasterPlayerController::HandleWarmTime(bool bTeamsMatch)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD && IsLocalController())
	{
		BlasterHUD->AddAnnouncement();
		BlasterHUD->AddCharacterOverlay();
		InitRoundNum();
		if(BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			const FString AnnouncementText = Announcement::MatchWarmup;
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			BlasterHUD->Announcement->InfoText->SetText(FText::FromString(FString::Printf(TEXT(""))));
		}
		if(!HasAuthority()) return;
		bShowTeamScores = bTeamsMatch;
		if(bTeamsMatch)
		{
			InitTeamScore();
		}
		else
		{
			HideTeamScore();
		}
	}
}

void ABlasterPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD && IsLocalController())
	{
		if(BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleWaiting()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD && IsLocalController())
	{
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::SetHUdTime()
{
	uint32 SecondsLeft = 0;
	float TimeLeft = 0.f;
	float TimeToHud = 0.f;
	if(!HasAuthority())
	{
		if(MatchState::WaitingToStart == MatchState)
		{
			TimeLeft = LoadingTime - GetServerTime() + LevelStartingTime;
		}
		if(MatchState::Warmup == MatchState)
		{
			TimeLeft = LoadingTime + WarmupTime - GetServerTime() + LevelStartingTime;
		}
		else if (MatchState::Round == MatchState)
		{
			TimeLeft = LoadingTime +  MatchTime + WarmupTime - GetServerTime() + LevelStartingTime;
		}
		else if(MatchState::Cooldown == MatchState)
		{
			TimeLeft = LoadingTime +  CooldownTime + MatchTime + WarmupTime - GetServerTime() + LevelStartingTime;
		}
		else if(MatchState::GameSummary == MatchState)
		{
			TimeLeft =  LoadingTime + GameSummaryTime + CooldownTime + MatchTime + WarmupTime - GetServerTime() + LevelStartingTime;
		}

		SecondsLeft = FMath::CeilToInt(TimeLeft);
		TimeToHud = TimeLeft;
	}else
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if(BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime());
			TimeToHud = BlasterGameMode->GetCountdownTime();
		}
	}
	if(SecondsLeft != CountdownInt)
	{
		if(MatchState::WaitingToStart == MatchState)
		{
			SetHUDAnnouncementCountdown(TimeToHud);
		}
		if(MatchState::Warmup == MatchState)
		{
			SetHUDMatchCountdown(TimeToHud);
			SetHUDAnnouncementCountdown(TimeToHud);
		}
		else if(MatchState::Round == MatchState)
		{
			SetHUDMatchCountdown(TimeToHud);
		}
		else if(MatchState::Cooldown == MatchState)
		{
			SetHUDAnnouncementCountdown(TimeToHud);
			fixScoreError();
		}
		else if(MatchState::GameSummary == MatchState)
		{
			SetHUDAnnouncementCountdown(TimeToHud);
		}
	}
	CountdownInt = SecondsLeft; 

	/*float TimeLeft = 0.f;
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
	CountdownInt = SecondsLeft; */
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

void ABlasterPlayerController::HideTeamScore()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->RedTeamScore&&
	BlasterHUD->CharacterOverlay->BlueTeamScore;

	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
	}
}

void ABlasterPlayerController::InitTeamScore()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->RedTeamScore&&
	BlasterHUD->CharacterOverlay->BlueTeamScore;
	
	if(bHUDValid)
	{
		const FString Zero("0");
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		BlasterGameState = BlasterGameState == nullptr ?  Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;
		if(BlasterGameState)
		{
			SetBlueTeamScore(BlasterGameState->BlueTeamScore);
			SetRedTeamScore(BlasterGameState->RedTeamScore);
		}
	}
}

void ABlasterPlayerController::InitRoundNum()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->RoundNum;

	if(bHUDValid)
	{
		const FString RoundNumStr = FString::Printf(TEXT("%d"), RoundNum);
		BlasterHUD->CharacterOverlay->RoundNum->SetText(FText::FromString(RoundNumStr));
	}
}

void ABlasterPlayerController::SetRedTeamScore(int32 RedScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->RedTeamScore;

	if(bHUDValid)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::SetBlueTeamScore(int32 BlueScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
	BlasterHUD->CharacterOverlay&&
	BlasterHUD->CharacterOverlay->BlueTeamScore;

	if(bHUDValid)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
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
		LoadingTime = GameMode->LoadingTime;
		RoundNum = GameMode->RoundNum;
		GameSummaryTime = GameMode->GameSummaryTime;
		bShowTeamScores= GameMode->bTeamsMatch;
		ClientJoinMidgame(MatchState, MatchTime, WarmupTime, LevelStartingTime, CooldownTime, RoundNum, LoadingTime, GameSummaryTime);
	}
}

// run on client
void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Match, float Warmup, float LevelStarting, float Cooldown, int32 Round, float Loading, float SummaryTime)
{
	MatchState = StateOfMatch;
	MatchTime = Match;
	WarmupTime = Warmup;
	LevelStartingTime = LevelStarting;
	CooldownTime = Cooldown;
	RoundNum = Round;
	LoadingTime = Loading;
	GameSummaryTime = SummaryTime;
	OnMatchStateSet(MatchState);
	
	/*if(BlasterHUD && MatchState==MatchState::WaitingToStart && BlasterHUD->Announcement==nullptr &&IsLocalController())
	{
		BlasterHUD->AddAnnouncement();
	}*/
	
}

void ABlasterPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	//ServerGetMatchTypeStartingTime();
	
	MatchState = State;
	if(MatchState == MatchState::WaitingToStart)
	{
		HandleWaiting();
	}
	if(MatchState == MatchState::Warmup)
	{
		HandleWarmTime(bTeamsMatch);
	}
	if(MatchState == MatchState::Round)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleMatchCooldown();
	}
	else if(MatchState == MatchState::GameSummary)
	{
		HandleMatchOver();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::WaitingToStart)
	{
		HandleWaiting();
	}
	if(MatchState == MatchState::Warmup)
	{
		HandleWarmTime();
	}
	if(MatchState == MatchState::Round)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleMatchCooldown();
	}
	else if(MatchState == MatchState::GameSummary)
	{
		HandleMatchOver();
	}
}

/*void ABlasterPlayerController::ClientReceiveMatchTypeStartingTime_Implementation(float MatchTypeStartingTime)
{
	LevelStartingTime = MatchTypeStartingTime;
}

void ABlasterPlayerController::ServerGetMatchTypeStartingTime_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	if(BlasterGameMode)
	{
		ClientReceiveMatchTypeStartingTime(BlasterGameMode->LevelStartingTime);
	}
}*/
void ABlasterPlayerController::fixScoreError()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		//BlasterHUD->CharacterOverlay->RemoveFromParent();
		const bool bHUDValid = BlasterHUD->Announcement&&
			BlasterHUD->Announcement->AnnouncementText&&
			BlasterHUD->Announcement->InfoText;
		if(bHUDValid)
		{
			
			BlasterGameState = BlasterGameState == nullptr ?  Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if(BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				const FString InfoString = bShowTeamScores ? GetTeamInfoText() : GetInfoText(TopPlayers);
				
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoString));
			}
		}
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
			const FString AnnouncementText = Announcement::NewMatchStartsIn;
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			BlasterGameState = BlasterGameState == nullptr ?  Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;

			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if(BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				const FString InfoString = bShowTeamScores ? GetTeamInfoText() : GetInfoText(TopPlayers);
				
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


void ABlasterPlayerController::HandleMatchOver()
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
			UE_LOG(LogTemp, Warning, TEXT("bHUDValid TRUE"));
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			const FString AnnouncementText = Announcement::MatchGameOver;

			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			BlasterGameState = BlasterGameState == nullptr ?  Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if(BlasterGameState && BlasterPlayerState)
			{
				if(HasAuthority())
				{
					UE_LOG(LogTemp, Warning, TEXT("server GOING TO SET InfoString"));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("client GOING TO SET InfoString"));
				}
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				const FString InfoString = bShowTeamScores ? GetTeamInfoText() : GetInfoText(TopPlayers);
				
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoString));
			}
		}
	}
}

FString ABlasterPlayerController::GetInfoText(const TArray<ABlasterPlayerState*>& TopPlayers)
{
	FString InfoString;
	ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if(BlasterPlayerState == nullptr) return InfoString;
	if(TopPlayers.Num() == 0)
	{
		InfoString = Announcement::ThereIsNoWinner;
	}
	else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
	{
		InfoString = Announcement::YouAreTheWinner;
	}
	else if(TopPlayers.Num() == 0)
	{
		InfoString = FString::Printf(TEXT("Winner: %s"), *TopPlayers[0]->GetPlayerName());
	}
	else if (TopPlayers.Num() > 0)
	{
		InfoString = Announcement::PlayersTiedForWin;
		InfoString.Append(FString("\n"));
		for(auto TiedPlayer : TopPlayers)
		{
			InfoString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoString;
}

FString ABlasterPlayerController::GetTeamInfoText()
{
	FString InfoString;
	BlasterGameState = BlasterGameState == nullptr ?  Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;

	if(BlasterGameState == nullptr) return  InfoString;
	const int32 RedTeamScore = BlasterGameState->RedTeamScore;
	const int32 BlueTeamScore = BlasterGameState->BlueTeamScore;
	if(RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoString = Announcement::ThereIsNoWinner;
	}
	else if(RedTeamScore == BlueTeamScore)
	{
		InfoString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForWin);
		/*InfoString.Append(Announcement::RedTeam);
		InfoString.Append(TEXT("\n"));
		InfoString.Append(Announcement::BlueTeam);
		InfoString.Append(TEXT("\n"));*/
		InfoString.Append(TEXT("\n"));
		InfoString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if(RedTeamScore > BlueTeamScore)
	{
		InfoString = Announcement::RedTeamWins;
		InfoString.Append(TEXT("\n"));
		InfoString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if(BlueTeamScore > RedTeamScore)
	{
		InfoString = Announcement::BlueTeamWins;
		InfoString.Append(TEXT("\n"));
		InfoString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}
	return InfoString;
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

void ABlasterPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if(BlasterHUD)
	{
		if(Attacker == Self && Victim != Self)
		{
			BlasterHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
			return;
		}
		if(Victim == Self && Attacker != Self)
		{
			BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
			return;
		}
		if(Victim == Attacker && Attacker == Self)
		{
			BlasterHUD->AddElimAnnouncement("You", "yourself");
			return;
		}
		if(Victim == Attacker && Attacker != Self)
		{
			BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
			return;
		}
		BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());

	}
}