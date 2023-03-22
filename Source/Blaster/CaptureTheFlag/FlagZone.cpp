// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/CaptureTheFlagGameMode.h"
#include "Blaster/Weapon/Flag.h"
#include "Components/SphereComponent.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphereComponent"));
	SetRootComponent(ZoneSphereComponent);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();


	
	ZoneSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
	
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if(OverlappingFlag && OverlappingFlag->GetTeam() != Team)
	{
		ACaptureTheFlagGameMode* CaptureTheFlagGameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if(CaptureTheFlagGameMode)
		{
			CaptureTheFlagGameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetFlag();
	}
}

void AFlagZone::OnSphereEndOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	
}

