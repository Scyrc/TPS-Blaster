// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Weapon/Shotgun.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 400.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);

	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME(UCombatComponent, bHoldingTheFlag);

}

// run on server
void UCombatComponent::PickAmmo(float AmmoAmount, EWeaponType WeaponType)
{
	if(CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
	}
	else
	{
		CarriedAmmoMap.Emplace(WeaponType,  FMath::Clamp(AmmoAmount, 0, MaxCarriedAmmo));
	}
	
	UpdateCarriedAmmo();

	// auto Relao
	if(EquippedWeapon && EquippedWeapon->GetWeaponType() == WeaponType && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if(UCameraComponent* FollowCamera =  Character->GetFollowCamera())
		{
			DefaultFOV = FollowCamera->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
	/*if(Character->HasAuthority())
	{
		InitializeCarriedAmmo();
	}*/
	
	UpdateHUDGrenades();

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}

}

// run on server
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr ||  WeaponToEquip == nullptr) return;
	if(CombatState != ECombatState::ECS_Unoccupied) return;

	if(WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Flag)
	{
		Character->Crouch();
		bHoldingTheFlag = true;
		AttachFlagToLeftHand(WeaponToEquip);
		WeaponToEquip->SetOwner(Character);
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);

		TheFlag = WeaponToEquip;
		/*Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = false;*/
	}
	else
	{
		if(EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
		{
			EquipSecondaryWeapon(WeaponToEquip);
		}
		else
		{
			EquipPrimaryWeapon(WeaponToEquip);
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
	
}

// run on server
void UCombatComponent::SwapWeapons()
{
	if(CombatState!= ECombatState::ECS_Unoccupied || Character == nullptr || !Character->HasAuthority()) return;
	Character->PlaySwapMontage();
	Character->bFinishedSwapping = false;
	CombatState = ECombatState::ECS_SwappingWeapons;

	if(SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
}

bool UCombatComponent::ShouldSwapWeapon()
{
	return (EquippedWeapon!=nullptr && SecondaryWeapon!=nullptr);
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr ||  WeaponToEquip == nullptr) return;
	DropEquippedWeapon();
	
	EquippedWeapon = WeaponToEquip;
	
	AttachActorToRightHand(EquippedWeapon);
	
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	EquippedWeapon->IsShowHUDAmmo(true);
	EquippedWeapon->SetHUDAmmo();
	
	UpdateCarriedAmmo();

	PlayEquipWeaponSound(WeaponToEquip);
	
	// auto reload
	ReloadEmptyWeapon();
}

// run on server
void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr ||  WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetOwner(Character);
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);

	AttachActorToBackpack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);

}

void UCombatComponent::OnRep_Aiming()
{
	if(Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	if(EquippedWeapon != nullptr)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if( Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if( Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;
	bool bUsePistolSocket =
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	const FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if(HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachFlagToLeftHand(AWeapon* Flag)
{
	if( Character == nullptr || Character->GetMesh() == nullptr || Flag == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("FlagSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(Flag, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if( Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr ) return;
	
	const FName SocketName = FName("BackpackSocket");
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if(BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if(EquippedWeapon == nullptr) return;
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] += EquippedWeapon->GetRemainAmmo();
	}
	else
	{
		CarriedAmmoMap.Emplace(EquippedWeapon->GetWeaponType(), EquippedWeapon->GetRemainAmmo());
	}
	CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if(Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if(EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShow)
{
	if(Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShow);
	}
}


// run on all client
void UCombatComponent::OnRep_EquippedWeapon()
{
	
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->IsShowHUDAmmo(true);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if(SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);
		PlayEquipWeaponSound(SecondaryWeapon);
	}
}

void UCombatComponent::Fire()
{
	if(CanFire())
	{
		bCanFire = false;
		
		if(EquippedWeapon)
		{
			CrosshairShootingFactor = 1.f;
			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			default: break;
			}
		}
		StartFireTimer();
	}

}

void UCombatComponent::FireHitScanWeapon()
{
	if(EquippedWeapon == nullptr) return;
	
	HitTarget = EquippedWeapon->GetUseScatter() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
	LocalFire(HitTarget);
	ServerFire(HitTarget, EquippedWeapon->GetFireDelay());
}

void UCombatComponent::FireProjectileWeapon()
{
	if(EquippedWeapon == nullptr) return;
	
	HitTarget = EquippedWeapon->GetUseScatter() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
	LocalFire(HitTarget);
	ServerFire(HitTarget, EquippedWeapon->GetFireDelay());
}

void UCombatComponent::FireShotgun()
{
	if(EquippedWeapon == nullptr) return;
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if(Shotgun)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		ShotgunLocalFire(HitTargets);
		ServerShotgunFire(HitTargets, EquippedWeapon->GetFireDelay());
	}
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	// UE_LOG(LogTemp, Warning, TEXT("fire........"));
	if(EquippedWeapon == nullptr) return;

	if(Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTarget)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon); 
	if(Character == nullptr || Shotgun == nullptr) return;
	if(CombatState == ECombatState::ECS_Reloading ||  CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

// run on client
void UCombatComponent::Reload()
{
	if(EquippedWeapon == nullptr ||
		EquippedWeapon->IsAmmoFull() ||
		ECombatState::ECS_Unoccupied != CombatState && !bLocallyReloading || CarriedAmmo<=0) return;
	HandleReload();
	ServerReload();
	bLocallyReloading = true;
}



// RUN ON player machine WHICH CALL
void UCombatComponent::ThrowGrenade()
{
	if(Grenades <=0) {return;}
	if(CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if(Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	if(Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	if(Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades-1, 0, MaxGrenades);

		UpdateHUDGrenades();
	}
}


// RUN ON Server 
void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if(Grenades <=0) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if(Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

	Grenades = FMath::Clamp(Grenades-1, 0, MaxGrenades);

	UpdateHUDGrenades();
}

// run on server
void UCombatComponent::ServerReload_Implementation()
{
	if(Character == nullptr ||
		EquippedWeapon == nullptr ||
		EquippedWeapon->IsAmmoFull() ||
		!CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())||
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] <=0 )
	{
		return;
	}
	
	CombatState = ECombatState::ECS_Reloading;
	if(!Character->IsLocallyControlled()) HandleReload();
}

void UCombatComponent::HandleReload()
{
	if(EquippedWeapon == nullptr) return;
	if(Character)
	{
		Character->PlayReloadMontage();
	}
}

void UCombatComponent::FinishReloading()
{
	if(Character == nullptr) return;
	bLocallyReloading = false;
	if(Character->HasAuthority())
	{
		UpdateAmmoValues();
		CombatState = ECombatState::ECS_Unoccupied;
		
		if(bFireButtonPressed)
		{
			Fire();
		}
	}

}

void UCombatComponent::FinishSwap()
{
	if(Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
	if(Character) Character->bFinishedSwapping = true;
	if(SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true);
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	PlayEquipWeaponSound(EquippedWeapon);

	if(Character == nullptr || !Character->HasAuthority()) return;
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->IsShowHUDAmmo(true);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	// auto reload
	ReloadEmptyWeapon();

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::UpdateAmmoValues()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	int32 NeedAmmo = EquippedWeapon->AmmoReloadNeeded();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 ActualAmmo = CarriedAmmo >= NeedAmmo ? NeedAmmo : CarriedAmmo;
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ActualAmmo;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		EquippedWeapon->Reload(ActualAmmo);
	
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDCarriedAmmo(CarriedAmmo);
		}
	}
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()) && CarriedAmmoMap[EquippedWeapon->GetWeaponType()] > 0)
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		EquippedWeapon->Reload(1);
		bCanFire = true;
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDCarriedAmmo(CarriedAmmo);
		}
		if(EquippedWeapon->IsAmmoFull() || CarriedAmmo == 0)
		{
			JumpToShotgunEnd();
		}
	}
}

void UCombatComponent::OnRep_Grenades()
{
	if(Character && Character->IsLocallyControlled())
	{
		UpdateHUDGrenades();
	}
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		 Controller->SetHUDGrenades(Grenades);
	}
}

void UCombatComponent::OnRep_HoldingTheFlag()
{
	if(Character && bHoldingTheFlag && Character->IsLocallyControlled())
	{
		Character->Crouch();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	// jump to shotgunend selection
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if(AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_Play(Character->GetReloadMontage());
		const FName SectionName("ShotgunEnd");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if(Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if(Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Character;
		SpawnParameters.Instigator = Character;
		UWorld* World = GetWorld();
		if(World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParameters
				);
		}
	}
}

// RUN ON CLIENT
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_SwappingWeapons:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
		break;
	}
}

void UCombatComponent::StartFireTimer()
{
	if(EquippedWeapon == nullptr || Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		 EquippedWeapon->GetFireDelay()
	);
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;

	if(EquippedWeapon == nullptr ) return;

	if(bFireButtonPressed && EquippedWeapon->IsAutomatic())
	{
		Fire();
	}

	// auto reload
	ReloadEmptyWeapon();
}

bool UCombatComponent::CanFire() const
{
    if(EquippedWeapon == nullptr) return false;
	if(!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;
	if(bLocallyReloading) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

// RUN ON CLIENT
void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	//Controller = Cast<ABlasterPlayerController>(Character->Controller);

	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	bool bJumpToShotgunEnd = CarriedAmmo == 0 &&
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun;

	if(bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
		
}

/*void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
}*/



void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(bFireButtonPressed &&EquippedWeapon)
	{
		Fire();
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if(Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TracerHitTarget,float FireDelay)
{
	MulticastFire(TracerHitTarget);
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if(EquippedWeapon)
	{
		const bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->GetFireDelay(), FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}


void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(Character && Character->IsLocallyControlled()) return;

	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets,float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if(EquippedWeapon)
	{
		const bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->GetFireDelay(), FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}


void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if(Character && Character->IsLocallyControlled()) return;
	ShotgunLocalFire(TraceHitTargets);

}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld =  UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
		);

	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if(Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection* (DistanceToCharacter + 80.f);
		}
		
		FVector End = CrosshairWorldPosition + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}

		if(TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
	
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if(Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->GetController()) : Controller;
	if(Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if(HUD)
		{
			if(EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft= EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft= nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			// set crosshairs spread
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelcoityMultiplierRange(0.f, 1.f);
			FVector Velcoity = Character->GetVelocity();
			Velcoity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelcoityMultiplierRange, Velcoity.Size());

			if(Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			if(bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor =  FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
			
			HUDPackage.CrosshairSpread =
				0.5 +
				CrosshairVelocityFactor +
				CrosshairInAirFactor +
				CrosshairAimFactor +
				CrosshairShootingFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(EquippedWeapon == nullptr) return;

	if(bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if(Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if(Character== nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if(Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}

	if(Character->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	if(!EquippedWeapon) return;
	bAiming = bIsAiming;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}


