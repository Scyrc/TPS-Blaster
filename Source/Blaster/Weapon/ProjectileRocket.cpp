// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}



void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	
	if(!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}
	if(TrailSystem)
	{
		TrailSystemComp =  UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem, 
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
			);
	}
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

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	if(HasAuthority())
	{
		APawn* FirePawn = GetInstigator();
		if(FirePawn)
		{
			AController* FireController = FirePawn->GetController();
			if(FireController)
			{
				UGameplayStatics::ApplyRadialDamageWithFalloff(
					this,
					Damage,
					MinDamage,
					GetActorLocation(),
					DamageInnerRadius,
					DamageOuterRadius,
					1.f,
					UDamageType::StaticClass(),
					TArray<AActor*>(),
					this,
					FireController);
			}
		}
		GetWorldTimerManager().SetTimer(
			DestroyTimer,
			this,
			&AProjectileRocket::DestroyTimerFinished,
			DestroyTime
			);
	}
	
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if(RocketMesh)
	{
		RocketMesh->SetVisibility(false);
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