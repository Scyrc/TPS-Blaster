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
	DOREPLIFETIME(UCombatComponent, PrimaryWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, KnifeWeapon);
	DOREPLIFETIME(UCombatComponent, C4BoomWeapon);
	DOREPLIFETIME(UCombatComponent, TargetWeaponIndex);

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

// run on server
void UCombatComponent::SwitchWeapon(int32 WeaponIndex)
{
	if(CombatState!= ECombatState::ECS_Unoccupied || Character == nullptr || !Character->HasAuthority()) return;
	if(GetWeaponByIndex(WeaponIndex) == nullptr ) return;
	if(EquippedWeapon == GetWeaponByIndex(WeaponIndex)) return;
	TargetWeaponIndex = WeaponIndex;
	
	Character->PlaySwapMontage();
	Character->bFinishedSwapping = false;
	CombatState = ECombatState::ECS_SwappingWeapons;
	GetWeaponByIndex(WeaponIndex)->EnableCustomDepth(false);
	
}

// run on server
void UCombatComponent::DropWeapon()
{
	if(EquippedWeapon == nullptr) return;
	if(EquippedWeapon == PrimaryWeapon) PrimaryWeapon = nullptr;
	if(EquippedWeapon == SecondaryWeapon) SecondaryWeapon = nullptr;
	if(EquippedWeapon == KnifeWeapon) KnifeWeapon = nullptr;
	if(EquippedWeapon == C4BoomWeapon) C4BoomWeapon = nullptr;
	EquippedWeapon->Dropped();
	EquippedWeapon = nullptr;
}

AWeapon* UCombatComponent::GetWeaponByIndex(int32 WeaponIndex)
{
	switch (WeaponIndex)
	{
	case 1:
		return PrimaryWeapon;
	case 2:
		return SecondaryWeapon;
	case 3:
		return KnifeWeapon;
	case 4:
		return nullptr;
	case 5:
		return C4BoomWeapon;
	default:
		return nullptr;
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
		WeaponToEquip->SetOwner(Character);
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachFlagToLeftHand(WeaponToEquip);
		TheFlag = WeaponToEquip;
		/*Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = false;*/
	}
	else if(WeaponToEquip->GetWeaponType() == EWeaponType::EWT_C4Bomb)
	{
		WeaponToEquip->SetOwner(Character);
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachBomb(WeaponToEquip);
		C4BoomWeapon = WeaponToEquip;
	}
	else if(WeaponToEquip->GetWeaponEquipType() == EEquippedType::EET_Primary)
	{
		UE_LOG(LogTemp, Warning, TEXT("EET_Primary"));
		EquipPrimaryWeapon(WeaponToEquip);

	}
	else if(WeaponToEquip->GetWeaponEquipType() == EEquippedType::EET_Secondary)
	{
		UE_LOG(LogTemp, Warning, TEXT("EET_Secondary"));
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else if(WeaponToEquip->GetWeaponEquipType() == EEquippedType::EET_Knife)
	{
		UE_LOG(LogTemp, Warning, TEXT("EET_Knife"));
		EquipKnifeWeapon(WeaponToEquip);
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
	if(EquippedWeapon == PrimaryWeapon)
	{
		if(EquippedWeapon)	EquippedWeapon->Dropped();
		PrimaryWeapon = WeaponToEquip;
		EquippedWeapon = PrimaryWeapon;
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		EquippedWeapon->IsShowHUDAmmo(true);
		EquippedWeapon->SetHUDAmmo();
		UpdateCarriedAmmo();
		PlayEquipWeaponSound(WeaponToEquip);
		// auto reload
		ReloadEmptyWeapon();
		AttachActorToRightHand(EquippedWeapon);

		return;
	}
	if(PrimaryWeapon && EquippedWeapon &&EquippedWeapon != PrimaryWeapon)
	{
		PrimaryWeapon->Dropped();
		PrimaryWeapon = WeaponToEquip;
		PrimaryWeapon->SetOwner(Character);
		PrimaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		PlayEquipWeaponSound(WeaponToEquip);
		AttachPrimaryWeapon(PrimaryWeapon);

		return;
	}
	if(PrimaryWeapon == nullptr && EquippedWeapon != nullptr)
	{
		PrimaryWeapon = WeaponToEquip;
		PrimaryWeapon->SetOwner(Character);
		PrimaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		PlayEquipWeaponSound(WeaponToEquip);
		AttachPrimaryWeapon(PrimaryWeapon);

		return;
	}
	if(PrimaryWeapon && EquippedWeapon == nullptr)
	{
		if(PrimaryWeapon)	PrimaryWeapon->Dropped();
		PrimaryWeapon = WeaponToEquip;
		EquippedWeapon = PrimaryWeapon;
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		EquippedWeapon->IsShowHUDAmmo(true);
		EquippedWeapon->SetHUDAmmo();
		UpdateCarriedAmmo();
		PlayEquipWeaponSound(WeaponToEquip);
		// auto reload
		ReloadEmptyWeapon();
		AttachActorToRightHand(EquippedWeapon);

	}
}

// run on server
void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr ||  WeaponToEquip == nullptr) return;
	if(EquippedWeapon == SecondaryWeapon)
	{
		if(EquippedWeapon)	EquippedWeapon->Dropped();
		SecondaryWeapon = WeaponToEquip;
		EquippedWeapon = SecondaryWeapon;
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		EquippedWeapon->IsShowHUDAmmo(true);
		EquippedWeapon->SetHUDAmmo();
		UpdateCarriedAmmo();
		PlayEquipWeaponSound(WeaponToEquip);
		// auto reload
		ReloadEmptyWeapon();
		AttachActorToRightHand(EquippedWeapon);

		return;
	}
	if(SecondaryWeapon && EquippedWeapon &&EquippedWeapon != SecondaryWeapon)
	{
		SecondaryWeapon->Dropped();
		SecondaryWeapon = WeaponToEquip;
		SecondaryWeapon->SetOwner(Character);
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		PlayEquipWeaponSound(WeaponToEquip);
		AttachSecondaryWeapon(SecondaryWeapon);
		return;
	}
	if(SecondaryWeapon == nullptr && EquippedWeapon != nullptr)
	{
		SecondaryWeapon = WeaponToEquip;
		SecondaryWeapon->SetOwner(Character);
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		PlayEquipWeaponSound(WeaponToEquip);
		AttachSecondaryWeapon(WeaponToEquip);
		return;
	}
	if(SecondaryWeapon && EquippedWeapon == nullptr)
	{
		if(SecondaryWeapon)	SecondaryWeapon->Dropped();
		SecondaryWeapon = WeaponToEquip;
		EquippedWeapon = SecondaryWeapon;
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		EquippedWeapon->IsShowHUDAmmo(true);
		EquippedWeapon->SetHUDAmmo();
		UpdateCarriedAmmo();
		PlayEquipWeaponSound(WeaponToEquip);
		// auto reload
		ReloadEmptyWeapon();
		AttachActorToRightHand(EquippedWeapon);
	}
}
void UCombatComponent::EquipKnifeWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr ||  WeaponToEquip == nullptr) return;
	if(EquippedWeapon == KnifeWeapon)
	{
		if(EquippedWeapon)	EquippedWeapon->Dropped();
		KnifeWeapon = WeaponToEquip;
		EquippedWeapon = KnifeWeapon;
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		EquippedWeapon->IsShowHUDAmmo(true);
		EquippedWeapon->SetHUDAmmo();
		UpdateCarriedAmmo();
		PlayEquipWeaponSound(WeaponToEquip);
		// auto reload
		ReloadEmptyWeapon();
		AttachActorToRightHand(EquippedWeapon);

		return;
	}
	if(KnifeWeapon && EquippedWeapon &&EquippedWeapon != KnifeWeapon)
	{
		KnifeWeapon->Dropped();
		KnifeWeapon = WeaponToEquip;
		KnifeWeapon->SetOwner(Character);
		KnifeWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		PlayEquipWeaponSound(WeaponToEquip);
		AttachKnifeWeapon(KnifeWeapon);

		return;
	}
	if(KnifeWeapon == nullptr && EquippedWeapon != nullptr)
	{
		KnifeWeapon = WeaponToEquip;
		KnifeWeapon->SetOwner(Character);
		KnifeWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		PlayEquipWeaponSound(WeaponToEquip);
		AttachKnifeWeapon(KnifeWeapon);
		return;
	}
	if(KnifeWeapon && EquippedWeapon == nullptr)
	{
		if(KnifeWeapon)	KnifeWeapon->Dropped();
		KnifeWeapon = WeaponToEquip;
		EquippedWeapon = KnifeWeapon;
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		EquippedWeapon->IsShowHUDAmmo(true);
		EquippedWeapon->SetHUDAmmo();
		UpdateCarriedAmmo();
		PlayEquipWeaponSound(WeaponToEquip);
		// auto reload
		ReloadEmptyWeapon();
		AttachActorToRightHand(EquippedWeapon);
	}
}

void UCombatComponent::MulticastAttachWeapon_Implementation(const int32 WeaponIndex)
{
	const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);

	if(WeaponIndex == 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachPrimaryWeapon"))
		PrimaryWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
		AttachPrimaryWeapon(PrimaryWeapon);
	}
	else if(WeaponIndex == 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachSecondaryWeapon"))
		SecondaryWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
		AttachSecondaryWeapon(SecondaryWeapon);
	}
	else if(WeaponIndex == 3)
	{
		KnifeWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);

		AttachKnifeWeapon(KnifeWeapon);
	}
	else if(WeaponIndex == 5)
	{
		C4BoomWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
		AttachBomb(C4BoomWeapon);
	}
	
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
void UCombatComponent::AttachBomb(AWeapon* Bomb)
{
	if( Character == nullptr || Character->GetMesh() == nullptr || Bomb == nullptr) return;
	const USkeletalMeshSocket* BombSocket = Character->GetMesh()->GetSocketByName(FName("BombSocket"));
	if(BombSocket)
	{
		BombSocket->AttachActor(Bomb, Character->GetMesh());
	}
}

inline void UCombatComponent::AttachPrimaryWeapon(AActor* ActorToAttach)
{
	if( Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* PrimaryWeaponSocket = Character->GetMesh()->GetSocketByName(FName("PrimaryWeaponSocket"));
	if(PrimaryWeaponSocket)
	{
		PrimaryWeaponSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

inline void UCombatComponent::AttachSecondaryWeapon(AActor* ActorToAttach)
{
	if( Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* SecondaryWeaponSocket = Character->GetMesh()->GetSocketByName(FName("SecondaryWeaponSocket"));
	if(SecondaryWeaponSocket)
	{
		SecondaryWeaponSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

inline void UCombatComponent::AttachKnifeWeapon(AActor* ActorToAttach)
{
	if( Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* KnifeWeaponSocket = Character->GetMesh()->GetSocketByName(FName("KnifeWeaponSocket"));
	if(KnifeWeaponSocket)
	{
		KnifeWeaponSocket->AttachActor(ActorToAttach, Character->GetMesh());
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
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->IsShowHUDAmmo(true);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_PrimaryWeapon()
{
	if(PrimaryWeapon && Character)
	{
		if(PrimaryWeapon != EquippedWeapon)
		{
			PrimaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
			AttachPrimaryWeapon(PrimaryWeapon);
			PlayEquipWeaponSound(PrimaryWeapon);
		}
	}
}

void UCombatComponent::OnRep_KnifeWeapon()
{
	if(KnifeWeapon && Character)
	{
		if(KnifeWeapon != EquippedWeapon)
		{
			KnifeWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
			AttachKnifeWeapon(KnifeWeapon);
			PlayEquipWeaponSound(KnifeWeapon);
		}
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if(SecondaryWeapon && Character)
	{
		if(SecondaryWeapon != EquippedWeapon)
		{
			SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
			AttachSecondaryWeapon(SecondaryWeapon);
			PlayEquipWeaponSound(SecondaryWeapon);
		}
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
	//if(GetWeaponByIndex(TargetWeaponIndex)) GetWeaponByIndex(TargetWeaponIndex)->EnableCustomDepth(true);
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	PlayEquipWeaponSound(EquippedWeapon);

	if(Character == nullptr || !Character->HasAuthority()) return;
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = GetWeaponByIndex(TargetWeaponIndex);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	EquippedWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
	AttachActorToRightHand(EquippedWeapon);

	EquippedWeapon->IsShowHUDAmmo(true);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	// auto reload
	ReloadEmptyWeapon();
	if(TempWeapon)
	{
		EEquippedType EquippedType = TempWeapon->GetWeaponEquipType();
		if(EquippedType == EEquippedType::EET_Primary)
		{
			PrimaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
			//AttachPrimaryWeapon(PrimaryWeapon);
			PrimaryWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
			AttachPrimaryWeapon(PrimaryWeapon);
			//MulticastAttachWeapon(1);
		}
		else if(EquippedType == EEquippedType::EET_Secondary)
		{
			SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
			//AttachSecondaryWeapon(SecondaryWeapon);
			//MulticastAttachWeapon(2);

			SecondaryWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
			AttachSecondaryWeapon(SecondaryWeapon);

		}
		else if(EquippedType == EEquippedType::EET_Knife)
		{
			
			KnifeWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
			//AttachKnifeWeapon(KnifeWeapon);
			//MulticastAttachWeapon(3);
			KnifeWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
			AttachKnifeWeapon(KnifeWeapon);

		}
		else if(EquippedType == EEquippedType::EET_Bomb)
		{
			C4BoomWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
			//AttachBomb(C4BoomWeapon);
			//MulticastAttachWeapon(5);
			C4BoomWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
			AttachBomb(C4BoomWeapon);
		}
	}
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
			Start += CrosshairWorldDirection* (DistanceToCharacter + CrossDistance);
			//DrawDebugPoint(GetWorld(), Start, 2, FColor::Red);
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


