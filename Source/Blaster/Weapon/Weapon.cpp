// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Casing.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn ,ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(NumberOfWeaponHighLight());
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
	
	AreaSphere = CreateDefaultSubobject<USphereComponent>("AreaSphere");
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>("PickupWidget");
	PickupWidget->SetupAttachment(RootComponent);
}


void AWeapon::Fire(const FVector& HitTarget)
{
	if(FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if(CasingClass)
	{
		if(const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject")))
		{
			const FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			
			if(UWorld* World = GetWorld())
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator() 
					);
			}
		}
	}
	/*if(HasAuthority())
	{
		SpendRound();
	}*/

	SpendRound();
}

void AWeapon::IsShowHUDAmmo(bool bShow)
{
	OwnerCharacter =  Cast<ABlasterCharacter>(GetOwner());
	if(OwnerCharacter)
	{
		OwnerController =  Cast<ABlasterPlayerController>(OwnerCharacter->Controller) ;
		if(OwnerController)
		{
			OwnerController->IsShowHUDWeaponAmmo(bShow);
		}
	}
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

// run on server
void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachmentTransformRules);

	IsShowHUDAmmo(false);
	Multcast_Dropped();
	
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

int32 AWeapon::AmmoReloadNeeded() const
{
	return FMath::Clamp(MagCapacity-Ammo, 0, MagCapacity);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicateMovement(true);

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	
}

void AWeapon::Multcast_Dropped_Implementation()
{
	IsShowHUDAmmo(false);
}


void AWeapon::OnSphereOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && PickupWidget)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && PickupWidget)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}
void AWeapon:: SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}


void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break; 
	}
}

void AWeapon::OnEquipped()
{
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if(OwnerCharacter && bUseServerSideReWind)
		{
			OwnerController = OwnerController == nullptr ?  Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController ;
			if(HasAuthority() && OwnerController && !OwnerController->HighPingDelegate.IsBound())
			{
				OwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
			}
		}
	}
	ShowPickupWidget(false);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if(WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);
	OwnerCharacter = OwnerCharacter==nullptr?  Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
	
}

void AWeapon::OnEquippedSecondary()
{
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if(OwnerCharacter && bUseServerSideReWind)
		{
			OwnerController = OwnerController == nullptr ?  Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController ;
			if(HasAuthority() && OwnerController && OwnerController->HighPingDelegate.IsBound())
			{
				OwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
			}
		}
	}
	ShowPickupWidget(false);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if(WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	}
	WeaponHighLight = EWeaponHighLight::EWHL_TAN;
	WeaponMesh->SetCustomDepthStencilValue(NumberOfWeaponHighLight());
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

void AWeapon::OnDropped()
{
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		if(OwnerCharacter && bUseServerSideReWind)
		{
			OwnerController = OwnerController == nullptr ?  Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController ;
			if(HasAuthority() && OwnerController && OwnerController->HighPingDelegate.IsBound())
			{
				OwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
			}
		}
	}
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn ,ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera ,ECollisionResponse::ECR_Ignore);

	WeaponHighLight = EWeaponHighLight::EWHL_BLUE;
	WeaponMesh->SetCustomDepthStencilValue(NumberOfWeaponHighLight());
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}


void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideReWind = !bPingTooHigh;
}

// run on client
void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}




void AWeapon::SpendRound()
{
	Ammo= FMath::Clamp(Ammo - 1, 0, MagCapacity);

	SetHUDAmmo();
	if(HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++Sequence;
	}
	
}


void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if(HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	 Ammo -= Sequence;
	SetHUDAmmo();
	IsShowHUDAmmo(true);
}

void AWeapon::Reload(int32 AmmoAmount)
{
	if(Ammo == MagCapacity) return;

	Ammo = FMath::Clamp(Ammo+AmmoAmount,Ammo, MagCapacity);
	ClientAddAmmo(AmmoAmount);
	SetHUDAmmo();
	 
	OwnerCharacter = Cast<ABlasterCharacter>(GetOwner()) ;
	if(OwnerCharacter && OwnerCharacter->GetCombatComponent() && IsAmmoFull() && WeaponType == EWeaponType::EWT_Shotgun)
	{
		OwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if(HasAuthority()) return;

	Ammo = FMath::Clamp(Ammo+AmmoToAdd,Ammo, MagCapacity);

	SetHUDAmmo();
	
	OwnerCharacter = Cast<ABlasterCharacter>(GetOwner()) ;
	if(OwnerCharacter && OwnerCharacter->GetCombatComponent() && IsAmmoFull() && WeaponType == EWeaponType::EWT_Shotgun)
	{
		OwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
	
}

int32 AWeapon::GetRemainAmmo() 
{
	const int32 res = StartingRemainAmmo;
	StartingRemainAmmo = 0;
	return res;
}

bool AWeapon::IsEmpty() const
{
	return Ammo <= 0;
}

int32 AWeapon::NumberOfWeaponHighLight() const
{
	switch (WeaponHighLight)
	{
	case EWeaponHighLight::EWHL_PURPLE:
		return 250;
	case EWeaponHighLight::EWHL_BLUE:
		return 251;
	case EWeaponHighLight::EWHL_TAN:
		return 252;
	default: return 252;;
	}
}

// run on all client
/*void AWeapon::OnRep_Ammo()
{
		OwnerCharacter = Cast<ABlasterCharacter>(GetOwner()) ;
		if(OwnerCharacter)
		{
			OwnerController =  Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
			if(OwnerController)
			{
				OwnerController->SetHUDWeaponAmmo(Ammo);
			}
		}

		if(OwnerCharacter && OwnerCharacter->GetCombatComponent() && IsAmmoFull() && WeaponType == EWeaponType::EWT_Shotgun)
		{
			OwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
		}
}*/

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideReWind, COND_OwnerOnly);
	// DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::SetHUDAmmo()
{
	OwnerCharacter = OwnerCharacter==nullptr?  Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
	if(OwnerCharacter)
	{
		OwnerController = OwnerController == nullptr ?  Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController ;
		if(OwnerController)
		{
			OwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

// run on client
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if(GetOwner() == nullptr)
	{
		OwnerCharacter = nullptr;
		OwnerController = nullptr;
	}
	else
	{
		OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : OwnerCharacter;
		if(OwnerCharacter && OwnerCharacter->GetEquippedWeapon() && OwnerCharacter->GetEquippedWeapon() == this)
		{
			IsShowHUDAmmo(true);
			SetHUDAmmo();
		}
	}
	
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}


FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket == nullptr) return FVector();
	
	const FTransform SocketTransFrom =  MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransFrom.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized*DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLocNormalized = (EndLoc - TraceStart).GetSafeNormal()*TRACE_LENGTH;
	
	/*DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);*/

	
	return FVector(TraceStart + ToEndLocNormalized);
}