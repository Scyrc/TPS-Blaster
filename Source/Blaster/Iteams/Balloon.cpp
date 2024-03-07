// Fill out your copyright notice in the Description page of Project Settings.


#include "Balloon.h"

#include "NiagaraFunctionLibrary.h"
#include "Blaster/Blaster.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/BalloonGameMode.h"
#include "Blaster/Weapon/Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABalloon::ABalloon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_Balloon);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);

	MeshComponent =  CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionBox);
}

void ABalloon::Destroyed()
{
	if(DestroyEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			DestroyEffect,
			GetActorLocation(),
			GetActorRotation());
	}
	Super::Destroyed();
}

void ABalloon::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &ABalloon::ABalloon::OnHit);

		GetWorld()->GetTimerManager().SetTimer(DestroyTimer, this, &ABalloon::AutoDestroy, AutoDestroyTime);
	}
	
}

void ABalloon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AddActorWorldRotation(FRotator(0.f, baseTurnRate*DeltaTime, 0.f));
	
	SetActorLocation(GetActorLocation() + FVector(0.f, 0.f, DeltaTime*MoveSpeed*100));
}

void ABalloon::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	if(OtherActor)
	{
		

		AProjectile* HitProjectile = Cast<AProjectile>(OtherActor);
		if(HitProjectile)
		{
			auto Shooter1 = HitProjectile->GetInstigator();
			auto Shooter = Cast<ABlasterCharacter>(HitProjectile->GetInstigator());

			if(Shooter1)
			{
				UE_LOG(LogTemp, Warning, TEXT("Shooter Hit"));
				ABalloonGameMode* BalloonGameMode =  Cast<ABalloonGameMode>(GetWorld()->GetAuthGameMode());

				//check(BalloonGameMode);
				if(BalloonGameMode)
				{
					UE_LOG(LogTemp, Warning, TEXT("going to Call PlayerHitBalloon"));
					BalloonGameMode->PlayerHitBalloon(Shooter1, Shooter);
				}
			}
		}
	}
	AutoDestroy();
}

void ABalloon::AutoDestroy()
{
	ABalloonGameMode* BalloonGameMode =  Cast<ABalloonGameMode>(GetWorld()->GetAuthGameMode());
	if(BalloonGameMode)
	{
		BalloonGameMode->CurrentBallNum -= 1;
	}

	Destroy();
}
