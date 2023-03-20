// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "WeaponTypes.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	//FString TargetText = FString::Printf(TEXT("Fire Called, %s"), *HitTarget.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *TargetText);
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("OwnerPawn not null"));

	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MuzzleFlashSocket not null"));
		FTransform SocketTransFrom =  MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransFrom.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		

		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
		if(BlasterCharacter && InstigatorController)
		{
			//UE_LOG(LogTemp, Warning, TEXT("InstigatorController not null"));
			bool bCauseAuthDamage = !bUseServerSideReWind || OwnerPawn->IsLocallyControlled();
			if(GetOwner()->HasAuthority() && bCauseAuthDamage)
			{
				//UE_LOG(LogTemp, Warning, TEXT("server Called"))

				UGameplayStatics::ApplyDamage(
				BlasterCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
				);
			}
			if(!GetOwner()->HasAuthority() && bUseServerSideReWind && OwnerCharacter->IsLocallyControlled())
			{
				//UE_LOG(LogTemp, Warning, TEXT("client Called"))

				OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
				OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : OwnerController;

				if(OwnerCharacter && OwnerController && OwnerCharacter->GetLagCompensationComponent())
				{
					OwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(
					BlasterCharacter,
					Start,
					HitTarget,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime,
					this
					);
				}
			}	
		}

		if(ParticleSystem)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ParticleSystem,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
				);
		}
		if(HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}
		

		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransFrom
			);
		}
		if(FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
	
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if(World)
	{
		FVector End =  TraceStart + (HitTarget - TraceStart) * 1.25;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
			);

		FVector BeamEnd = End;
		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		// DrawDebugSphere(GetWorld(), BeamEnd, 4.f, 12, FColor::Purple, true);

		if(BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}


