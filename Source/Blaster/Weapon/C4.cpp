// Fill out your copyright notice in the Description page of Project Settings.


#include "C4.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

// Fill out your copyright notice in the Description page of Project Settings.

AC4::AC4()
{
	BoomMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoomMesh"));
	SetRootComponent(BoomMesh);
	GetAreaSphere()->SetupAttachment(BoomMesh);
	GetPickupWidget()->SetupAttachment(BoomMesh);
	BoomMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoomMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AC4::Dropped()
{
	//Super::Dropped();
	SetWeaponState(EWeaponState::EWS_Dropped);
	const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	BoomMesh->DetachFromComponent(DetachmentTransformRules);
	
	Multcast_Dropped();
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}


void AC4::OnDropped()
{
	//Super::OnDropped();

	if(HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	BoomMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoomMesh->SetSimulatePhysics(true);
	BoomMesh->SetEnableGravity(true);
	BoomMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	BoomMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn ,ECollisionResponse::ECR_Ignore);
	BoomMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera ,ECollisionResponse::ECR_Ignore);
	
	EnableCustomDepth(true);
	
}

void AC4::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
}

void AC4::OnEquipped()
{
	//Super::OnEquipped();

	if(HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	ShowPickupWidget(false);
	BoomMesh->SetSimulatePhysics(false);
	BoomMesh->SetEnableGravity(false);
	BoomMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoomMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	EnableCustomDepth(false);
	OwnerCharacter = OwnerCharacter==nullptr?  Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
}
