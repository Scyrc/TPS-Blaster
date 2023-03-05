// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


void AShotgun::Fire(const FVector& HitTarget)
{
	//Super::Fire(HitTarget);
	AWeapon::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;

	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransFrom =  MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransFrom.GetLocation();
		TMap<ABlasterCharacter*, uint32> HitMap;
		for(int i=0;i<NumberOfPellets;++i)
		{
			FHitResult FireHit;

			WeaponTraceHit(Start, HitTarget, FireHit);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if(BlasterCharacter && HasAuthority() && InstigatorController)
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
