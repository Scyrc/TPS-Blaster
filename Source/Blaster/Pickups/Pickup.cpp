// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(100.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 66.f));

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlapSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetCustomDepthStencilValue(NumberOfPickupHighLight());
	PickupMesh->MarkRenderStateDirty();
	PickupMesh->SetRenderCustomDepth(true);
	PickupMesh->SetRelativeScale3D(FVector(5.f,5.f,5.f));


	PickupEffectComp = CreateDefaultSubobject<UNiagaraComponent>("PickupEffectComp");
	PickupEffectComp->SetupAttachment(RootComponent);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			BindOverlapTimer,
			this,
			&APickup::BindOverlapFinished,
			BindOverlapTime
		);
	}

}
void APickup::BindOverlapFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OVerlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(PickupMesh)
	{
		PickupMesh->AddWorldRotation(FRotator(0.f, baseTurnRate*DeltaTime, 0.f));
	}
}

void APickup::Destroyed()
{
	if(PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
	
	if(PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}

	Super::Destroyed();
}

int32 APickup::NumberOfPickupHighLight() const
{
	switch (PickupHighLightt)
	{
	case EPickupHighLight::EPHL_PURPLE:
		return 250;
	case EPickupHighLight::EPHL_BLUE:
		return 251;
	case EPickupHighLight::EPHL_TAN:
		return 252;
	default: return 252;;
	}
}
