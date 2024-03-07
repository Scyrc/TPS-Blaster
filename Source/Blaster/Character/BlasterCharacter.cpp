// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterComponents/ActionComponent.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerStart/TeamPlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/C4.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;
	TPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPSCamera"));
	TPSCamera->SetupAttachment(CameraBoom);
	TPSCamera->bUsePawnControlRotation = true;
	TPSCamera->SetActive(true);

	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	FPSCamera->SetupAttachment(GetMesh(), FName("FPSCameraSocket"));
	FPSCamera->bUsePawnControlRotation = true;
	FPSCamera->SetActive(false);

	FPSMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSMesh"));
	FPSMesh->SetOnlyOwnerSee(true);
	FPSMesh->SetupAttachment(FPSCamera);
	FPSMesh->bCastDynamicShadow = false;
	FPSMesh->CastShadow = true;
	
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	//Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	//Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	ActionComp = CreateDefaultSubobject<UActionComponent>(TEXT("ActionComp"));
	//ActionComp->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	TurningInPlace = ETurningInPlace::ETIP_NotTuring;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(	FName("head"), head);
	
	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(	FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(	FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(	FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(	FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(	FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(	FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(	FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(	FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(	FName("hand_r"), hand_r);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(	FName("backpack"), backpack);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(	FName("blanket"), blanket);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(	FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(	FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(	FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(	FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(	FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(	FName("foot_r"), foot_r);

	for(auto& Box : HitCollisionBoxes)
	{
		if(Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if(CrownSystem == nullptr) return;
	if(CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if(CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if(GetMesh() == nullptr) return;
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, OriginalMaterialInstance);
		DissolveMaterialInstance = BlueDissolveMaterialInstance;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterialInstance);
		DissolveMaterialInstance = RedDissolveMaterialInstance;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterialInstance);
		DissolveMaterialInstance = BlueDissolveMaterialInstance;
		break;
	}
}

void ABlasterCharacter::SetPickedColor_Implementation(const FString& HeroName)
{
	if(GetMesh() == nullptr) return;
	
	if( HeroName == "Default")
		GetMesh()->SetMaterial(0, DefaultMaterialInstance);
		
	else if( HeroName == "Red")
		GetMesh()->SetMaterial(0, RedMaterialInstance);
	
	else if( HeroName == "Blue")
		GetMesh()->SetMaterial(0, BlueMaterialInstance);
	
	else if( HeroName == "Purple")
		GetMesh()->SetMaterial(0, PurpleMaterialInstance);
	
	else if( HeroName == "Gold")
		GetMesh()->SetMaterial(0, GoldMaterialInstance);
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUDHealth();
	UpdateHUDShield();
	/*if(Combat)
	{
		Combat->UpdateHUDGrenades();
	}*/
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
		SpawnDefaultWeapon();
	}

	if(AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, ELifetimeCondition::COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, CurrentHealth);
	DOREPLIFETIME(ABlasterCharacter, CurrentShield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGamePlay);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(Combat)
	{
		Combat->Character = this; 
	}
	if(Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);	
	}
	if(LagCompensation)
	{
		LagCompensation->Character = this;
		if(Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	HideCameraIfAimming();
	PollInit();
}

void ABlasterCharacter:: RotateInPlace(float DeltaTime)
{
	if(Combat && Combat->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTuring;
		return;
	}
	if(Combat && Combat->EquippedWeapon)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
	}

	if(bDisableGamePlay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTuring;
		return;
	}
	if(GetLocalRole() > ENetRole::ROLE_SimulatedProxy &&IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if(TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}



void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimproxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if(ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState()!=MatchState::InProgress;
	if(Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(CurrentHealth, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(CurrentShield, MaxShield);
	}
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	UWorld* World = GetWorld();
	if(BlasterGameMode && World && !bElimmed && DefaultWeapon && Combat)
	{
		AWeapon* StartingWeapon =  World->SpawnActor<AWeapon>(DefaultWeapon);
		StartingWeapon->bDestroyWeapon = true;
		Combat->EquipWeapon(StartingWeapon);
	}
}

void ABlasterCharacter::PollInit()
{
	if(BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = Cast<ABlasterPlayerState>(GetPlayerState());
		if(BlasterPlayerState)
		{
			OnPlayerStateInitialized();
			
			ABlasterGameState* GameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if(GameState && GameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void ABlasterCharacter::OnPlayerStateInitialized()
{
	BlasterPlayerState->AddToScore(0.f);
	BlasterPlayerState->AddToDefeats(0);
	SetTeamColor(BlasterPlayerState->GetTeam());
	BlasterPlayerState->SetColor();
	SetSpawnPoint();
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if(!IsLocallyControlled()) return;
		
	if(IsFPS) return;

	if((TPSCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if(Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if(Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::HideCameraIfAimming()
{
	if(!IsLocallyControlled()) return;

	if(Combat->bAiming)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if(Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if(Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}


void ABlasterCharacter::MoveForward(float Value)
{
	if(bDisableGamePlay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator	YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if(bDisableGamePlay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator	YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPress()
{
	if(bDisableGamePlay) return;
	if(Combat)   
	{
		if(Combat->bHoldingTheFlag) return;
		if(Combat->CombatState == ECombatState::ECS_Unoccupied) ServerEquipButtonPress();
		/*const bool bLocalSwap = !HasAuthority()&&
			Combat->ShouldSwapWeapon()&&
			Combat->CombatState == ECombatState::ECS_Unoccupied&&
			OverlappingWeapon == nullptr;
		if(bLocalSwap)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping =false;
		}*/
	}
}

void ABlasterCharacter::ServerEquipButtonPress_Implementation()
{
	if(Combat)
	{
		if(OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		/*else if(Combat->ShouldSwapWeapon())
		{
			Combat->SwapWeapons();
		}*/
	}
}

void ABlasterCharacter::CrouchButtonPress()
{
	if(bDisableGamePlay) return;
	if(Combat && Combat->bHoldingTheFlag) return;
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::ReloadButtonPress()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if(bDisableGamePlay) return;
	if(Combat)
	{
		Combat->Reload();
	}
}
void ABlasterCharacter::AimButtonPress()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if(bDisableGamePlay) return;
	if(Combat)
	{
		Combat->SetAiming(!Combat->bAiming);
	}
}

void ABlasterCharacter::GrenadeButtonPress()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if(Combat)
	{
		Combat->ThrowGrenade();
	}
}
inline void ABlasterCharacter::ServerSwitchButtonPress_Implementation(int32 WeaponIndex)
{
	if(Combat)
	{
		Combat->SwitchWeapon(WeaponIndex);
	}
}


inline void ABlasterCharacter::ServerDropButtonPress_Implementation()
{
	if(Combat)
	{
		Combat->DropWeapon();
	}
}

void ABlasterCharacter::SwitchPrimaryButtonPress()
{
	if(bDisableGamePlay) return;
	if(Combat)   
	{
		if(Combat->bHoldingTheFlag) return;
		if(Combat->CombatState == ECombatState::ECS_Unoccupied) ServerSwitchButtonPress(1);
		/*const bool bLocalSwap = !HasAuthority()&&
			Combat->ShouldSwapWeapon()&&
			Combat->CombatState == ECombatState::ECS_Unoccupied&&
			OverlappingWeapon == nullptr;
		if(bLocalSwap)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping =false;
		}*/
	}
}

void ABlasterCharacter::SwitchSecondaryButtonPress()
{
	if(bDisableGamePlay) return;
	if(Combat == nullptr) return;   
	if(Combat->bHoldingTheFlag) return;
	if(Combat->CombatState == ECombatState::ECS_Unoccupied) ServerSwitchButtonPress(2);
}

void ABlasterCharacter::SwitchKnifeButtonPress()
{
	if(bDisableGamePlay) return;
	if(Combat == nullptr) return;   
	if(Combat->bHoldingTheFlag) return;
	if(Combat->CombatState == ECombatState::ECS_Unoccupied) ServerSwitchButtonPress(3);
	
}

void ABlasterCharacter::SwitchPropsButtonPress()
{
	if(bDisableGamePlay) return;
	if(Combat == nullptr) return;   
	if(Combat->bHoldingTheFlag) return;
	if(Combat->CombatState == ECombatState::ECS_Unoccupied) ServerSwitchButtonPress(4);
}

void ABlasterCharacter::SwitchBombButtonPress()
{
	if(bDisableGamePlay) return;
	if(Combat == nullptr) return;   
	if(Combat->bHoldingTheFlag) return;
	if(Combat->CombatState == ECombatState::ECS_Unoccupied) ServerSwitchButtonPress(5);
}

void ABlasterCharacter::DropWeaponButtonPress()
{
	if(Combat == nullptr) return;   
	if(Combat->bHoldingTheFlag) return;
	if(Combat->CombatState == ECombatState::ECS_Unoccupied) ServerDropButtonPress();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if(GetAO_Pitch() > 90.f && !IsLocallyControlled())
	{
		// map pitch from [250,360) to [-90, 0)
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}
float ABlasterCharacter::CalculateSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if(Combat && Combat->EquippedWeapon == nullptr) return;
	const float Speed = CalculateSpeed();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();
	if(Speed == 0.f && !bIsInAir)  // standing still, not jumping
	{
		bRotateRootBone = true;
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator( CurrentAimRotation,StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if(TurningInPlace == ETurningInPlace::ETIP_NotTuring)
		{
			InterAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	} 
	if(Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTuring;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::SimproxiesTurn()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	if(CalculateSpeed()>0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTuring;
		return;
	}
	ProxyRotationLastFrame = ProxyRotationCurrentFrame;
	ProxyRotationCurrentFrame = GetControlRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotationCurrentFrame, ProxyRotationLastFrame).Yaw;

	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if(ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTuring;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTuring;
}

void ABlasterCharacter::Jump()
{
	if(bDisableGamePlay) return;
	if(Combat && Combat->bHoldingTheFlag) return;

	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if(bDisableGamePlay) return;
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if(bDisableGamePlay) return;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if(TurningInPlace != ETurningInPlace::ETIP_NotTuring)
	{
		InterAO_Yaw = FMath::FInterpTo(InterAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterAO_Yaw;
		if(FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTuring;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if(LastHealth > CurrentHealth)
	{
		PlayHitReactMontage(); 
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if(LastShield > CurrentShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,AController* InstigatedBy, AActor* DamageCauser)
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if(bElimmed || BlasterGameMode == nullptr) return;
	Damage = BlasterGameMode->CalculateDamage(InstigatedBy, Controller, Damage);
	if(Damage == 0.f) return;
	float DamageToHealth = Damage;
	if(CurrentShield > 0.f)
	{
		if(CurrentShield >= Damage)
		{
			CurrentShield = FMath::Clamp(CurrentShield - Damage, 0.f, CurrentShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(Damage - CurrentShield, 0.f, Damage);
			CurrentShield = 0.f;
		}
	}
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageToHealth, 0.f, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();
	
	PlayHitReactMontage();

	if(CurrentHealth == 0.f)
	{
		if(BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerPlayerController = Cast<ABlasterPlayerController>(InstigatedBy);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerPlayerController);
		}
	}
}


void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if(DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;

	if (OverlappingWeapon&& IsLocallyControlled())
	{
		OverlappingWeapon->ShowPickupWidget(true);
		if(AC4* C4Boom = Cast<AC4>(OverlappingWeapon))
		{
			UE_LOG(LogTemp, Warning, TEXT("c4boom"));
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
	return (Combat &&Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming() const
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if(Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

void ABlasterCharacter::SetInBombZone(bool bInZone)
{
	if(Combat == nullptr) return;
	Combat->bInBombZone = bInZone;
}

bool ABlasterCharacter::IsLocallyReloading() const
{
	if(Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

bool ABlasterCharacter::IsHoldingTheFlag() const
{
	if(Combat == nullptr) return false;
	return Combat->bHoldingTheFlag;
}

ETeam ABlasterCharacter::GetTeam()
{
	BlasterPlayerState = BlasterPlayerState == nullptr? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if(BlasterPlayerState)
	{
		return BlasterPlayerState->GetTeam();
	}
	return ETeam::ET_NoTeam;
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		const FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UE_LOG(LogTemp, Warning, TEXT("PlayElimMontage Called"));

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ElimMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("going to play Montage_Play"));

		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		const FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr ) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlaySwapMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr ) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if(Weapon == nullptr) return;
	if(Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapon()
{
	if(Combat)
	{
		if(Combat->PrimaryWeapon)
		{
			DropOrDestroyWeapon(Combat->PrimaryWeapon);
		}
		if(Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
		if (Combat->bHoldingTheFlag && Combat->TheFlag)
		{
			Combat->TheFlag->Dropped();
		}
		if (Combat->C4BoomWeapon)
		{
			Combat->C4BoomWeapon->Dropped();
			Combat->C4BoomWeapon = nullptr;
		}
	}
}

void ABlasterCharacter::SetSpawnPoint()
{
	if(HasAuthority() && BlasterPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStart;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStart);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for(auto& Start : PlayerStart)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if(TeamStart && TeamStart->Team == BlasterPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}

		if(TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(
				ChosenPlayerStart->GetActorLocation(),
				ChosenPlayerStart->GetActorRotation()
			);
		}
		
	}
}

void ABlasterCharacter::Elim(bool PlayLeftGame)
{
	MulticastElim(PlayLeftGame);  // run on all clinets

	// run on server
	DropOrDestroyWeapon();
}

void ABlasterCharacter::MulticastElim_Implementation(bool PlayLeftGame)
{
	bLeftGame = PlayLeftGame;
	bElimmed = true;
	PlayElimMontage();
	// start Dissolve effect
	if(DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	//Disable Character Movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGamePlay = true;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
	/*if(BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);
	}*/

	//Disable Collision

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// spawn Elin effect

	if(ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
			);
	}
	if(ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			GetWorld(),
			ElimBotSound,
			GetActorLocation()
			);
	}
	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if(bHideSniperScope)
	{
		 ShowSniperScopeWidget(false);
	}
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(
	ElimTimer,
	this,
	&ABlasterCharacter::ElimTimerFinished,
	ElimDelay
);
}

void ABlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if(bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
	if(BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
	if(ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	BlasterPlayerState = BlasterPlayerState == nullptr? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if(BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(Combat == nullptr) return  FVector();

	return Combat->HitTarget;
}

void ABlasterCharacter::SkillOneButtonPress()
{
	if(SkillsName.Num() > 0) ActionComp->StartActionByName(this, SkillsName[0]);
}

void ABlasterCharacter::SkillTwoButtonPress()
{
	if(SkillsName.Num() > 1) ActionComp->StartActionByName(this, SkillsName[1]);
}

void ABlasterCharacter::SkillThreeButtonPress()
{
	if(SkillsName.Num() > 2) ActionComp->StartActionByName(this, SkillsName[2]);
}

void ABlasterCharacter::SkillFourButtonPress()
{
	if(SkillsName.Num() > 3) ActionComp->StartActionByName(this, SkillsName[3]);
}

void ABlasterCharacter::SwitchView()
{
	if(TPSCamera->IsActive())
	{
		FPSCamera->SetActive(true);
		GetMesh()->SetOwnerNoSee(true);
		TPSCamera->SetActive(false);
	}
	else if(FPSCamera->IsActive())
	{
		TPSCamera->SetActive(true);
		GetMesh()->SetOwnerNoSee(false);
		FPSCamera->SetActive(false);
	}
}

void ABlasterCharacter::BuyItems()
{
	//BlasterPlayerState = BlasterPlayerState == nullptr ? Cast<ABlasterPlayerState>(GetPlayerState()) : BlasterPlayerState;
	
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

	PlayerInputComponent->BindAction("EquipWeapon", IE_Pressed, this, &ABlasterCharacter::EquipButtonPress);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPress);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPress);
	
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPress);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeButtonPress);

	PlayerInputComponent->BindAction("SwitchPrimary", IE_Pressed, this, &ABlasterCharacter::SwitchPrimaryButtonPress);
	PlayerInputComponent->BindAction("SwitchSecondary", IE_Pressed, this, &ABlasterCharacter::SwitchSecondaryButtonPress);
	PlayerInputComponent->BindAction("SwitchKnife", IE_Pressed, this, &ABlasterCharacter::SwitchKnifeButtonPress);
	PlayerInputComponent->BindAction("SwitchProps", IE_Pressed, this, &ABlasterCharacter::SwitchPropsButtonPress);
	PlayerInputComponent->BindAction("SwitchBomb", IE_Pressed, this, &ABlasterCharacter::SwitchBombButtonPress);
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &ABlasterCharacter::DropWeaponButtonPress);

	
	PlayerInputComponent->BindAction("SkillOne", IE_Pressed, this, &ABlasterCharacter::SkillOneButtonPress);
	PlayerInputComponent->BindAction("SkillTwo", IE_Pressed, this, &ABlasterCharacter::SkillTwoButtonPress);
	PlayerInputComponent->BindAction("SkillThree", IE_Pressed, this, &ABlasterCharacter::SkillThreeButtonPress);
	PlayerInputComponent->BindAction("SkillFour", IE_Pressed, this, &ABlasterCharacter::SkillFourButtonPress);

	PlayerInputComponent->BindAction("SwitchView", IE_Pressed, this, &ABlasterCharacter::SwitchView);

	PlayerInputComponent->BindAction("BuyItems", IE_Pressed, this, &ABlasterCharacter::SwitchView);
	
}


void ABlasterCharacter::SetHoldingTheFlag(bool bHolding)
{
	if(Combat == nullptr) return;
	Combat->bHoldingTheFlag = bHolding;
}

void ABlasterCharacter::RemoveFlag()
{
	if(Combat == nullptr) return;
	
	Combat->TheFlag = nullptr;
}
