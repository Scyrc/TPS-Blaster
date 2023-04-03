// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "RocketMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	ProjectMesh->SetupAttachment(RootComponent);
	ProjectMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent =  CreateDefaultSubobject<URocketMovementComponent>("RocketMovementComponent");
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(false);
}



void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	
	if(!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	SpawnTrailSystem();
	
	if(ProjectileLoop&& LoopSoundAttenuation)
	{
		ProjectileLoopComp = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0,
			LoopSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
			);
	}
	
}



void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	if(OtherActor == GetOwner())
	{
		return;
	}
	ExplodeDamage();
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if(ProjectMesh)
	{
		ProjectMesh->SetVisibility(false);
	}
	if(CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if(TrailSystemComp && TrailSystemComp->GetSystemInstanceController())
	{
		TrailSystemComp->GetSystemInstanceController()->Deactivate();
	}
	if(ProjectileLoopComp && ProjectileLoopComp->IsPlaying())
	{
		ProjectileLoopComp->Stop();
	}
}

void AProjectileRocket::Destroyed()
{
	//Super::Destroyed();
}