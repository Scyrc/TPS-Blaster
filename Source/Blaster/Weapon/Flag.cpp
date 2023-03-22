// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);
	GetAreaSphere()->SetupAttachment(FlagMesh);
	GetPickupWidget()->SetupAttachment(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFlag::Dropped()
{
	//Super::Dropped();
	SetWeaponState(EWeaponState::EWS_Dropped);
	const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachmentTransformRules);
	
	Multcast_Dropped();
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

void AFlag::ResetFlag()
{
	
	ABlasterCharacter* FlagBearer = Cast<ABlasterCharacter>(GetOwner());
	if(FlagBearer)
	{
		FlagBearer->SetHoldingTheFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->UnCrouch();
	}
	if(!HasAuthority()) return;
	SetActorTransform(InitialTransform);
	SetWeaponState(EWeaponState::EWS_Initial);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Overlap);
	OwnerCharacter = OwnerCharacter==nullptr?  Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;

	OwnerCharacter->RemoveFlag();
	
	const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachmentTransformRules);
	Multcast_Dropped();
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

void AFlag::OnDropped()
{
	//Super::OnDropped();

	if(HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn ,ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera ,ECollisionResponse::ECR_Ignore);
	
	EnableCustomDepth(true);
	
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
}

void AFlag::OnEquipped()
{
	//Super::OnEquipped();

	if(HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	ShowPickupWidget(false);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	EnableCustomDepth(false);
	OwnerCharacter = OwnerCharacter==nullptr?  Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;

}
