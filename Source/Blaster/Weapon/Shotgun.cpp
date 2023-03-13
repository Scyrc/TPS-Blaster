// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"




void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket == nullptr) return;
	

	FTransform SocketTransFrom =  MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransFrom.GetLocation();
	TMap<ABlasterCharacter*, uint32> HitMap;

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

	AController* InstigatorController = OwnerPawn->GetController();

	if(HasAuthority() && InstigatorController)
	{
		for(auto Hitpair : HitMap)
		{
			if(Hitpair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
				Hitpair.Key,
				Damage * Hitpair.Value,
				InstigatorController,
				this,
				UDamageType::StaticClass()
				);
			}
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
