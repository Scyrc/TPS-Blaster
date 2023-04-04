// Fill out your copyright notice in the Description page of Project Settings.


#include "BombZone.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"


ABombZone::ABombZone()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	ZoneSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphereComponent"));
	SetRootComponent(ZoneSphereComponent);
}


void ABombZone::BeginPlay()
{
	Super::BeginPlay();
	
	ZoneSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ABombZone::OnSphereOverlap);
	ZoneSphereComponent->OnComponentEndOverlap.AddDynamic(this, &ABombZone::OnSphereEndOverlap);

}

void ABombZone::OnSphereOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* OverlappingCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(OverlappingCharacter && OverlappingCharacter->GetTeam() != Team)
	{
		OverlappingCharacter->SetInBombZone(true);
		UE_LOG(LogTemp, Warning, TEXT("Enter Zoom"))
	}
}

void ABombZone::OnSphereEndOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	ABlasterCharacter* OverlappingCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(OverlappingCharacter && OverlappingCharacter->GetTeam() != Team)
	{
		OverlappingCharacter->SetInBombZone(false);
		UE_LOG(LogTemp, Warning, TEXT("Exit Zoom"))
	}
}