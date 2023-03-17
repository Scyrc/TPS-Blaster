// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"




void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket == nullptr) return;
	

	FTransform SocketTransFrom =  MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransFrom.GetLocation();

	// Maps hit character to number of times hit
	TMap<ABlasterCharacter*, uint32> HitMap;
	TMap<ABlasterCharacter*, uint32> HeadShotHitMap;
	
	for(FVector_NetQuantize HitTarget : HitTargets)
	{
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if(BlasterCharacter)
		{
			if(HitMap.Contains(BlasterCharacter))
			{
				HitMap[BlasterCharacter] += 1;
			}
			else
			{
				HitMap.Emplace(BlasterCharacter, 1);
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
				FireHit.ImpactPoint,
				0.5f,
				FMath::FRandRange(-.5f, .5f)
			);
		}

		
	}

	TArray<ABlasterCharacter*> HitCharacters;

	// Maps Character hit to total damage
	TMap<ABlasterCharacter*, float> DamageMap;

	// Calculate body shot damage by multiplying times hit x Damage - store in DamageMap
	for (auto HitPair : HitMap)
	{
		if (HitPair.Key)
		{
			DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);

			HitCharacters.AddUnique(HitPair.Key);
		}
	}

	// Calculate head shot damage by multiplying times hit x HeadShotDamage - store in DamageMap
	/*for (auto HeadShotHitPair : HeadShotHitMap)
	{
		if (HeadShotHitPair.Key)
		{
			if (DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
			else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);

			HitCharacters.AddUnique(HeadShotHitPair.Key);
		}
	}*/

	// Loop through DamageMap to get total damage for each character
	for (auto DamagePair : DamageMap)
	{
		if (DamagePair.Key && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideReWind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				UGameplayStatics::ApplyDamage(
					DamagePair.Key, // Character that was hit
					DamagePair.Value, // Damage calculated in the two for loops above
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}

	if (!HasAuthority() && bUseServerSideReWind)
	{
		OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : OwnerCharacter;
		OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : OwnerController;
		if (OwnerController && OwnerCharacter && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
		{
			OwnerCharacter->GetLagCompensationComponent()->ServerShotgunScoreRequest(
				HitCharacters,
				Start,
				HitTargets,
				OwnerController->GetServerTime() - OwnerController->SingleTripTime
			);
		}
	}

	
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket == nullptr) return;
	
	const FTransform SocketTransFrom =  MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransFrom.GetLocation();
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized*DistanceToSphere;

	for(uint32 i=0; i < NumberOfPellets; ++i)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		const FVector ToEndLoc = (EndLoc - TraceStart).GetSafeNormal()*TRACE_LENGTH;
	
		//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
		// DrawDebugSphere(GetWorld(), TraceStart + ToEndLoc, 4.f, 12, FColor::Orange, true);
 
	
		HitTargets.Push(TraceStart + ToEndLoc);
		
	}
	
}
