// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;

	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransFrom =  MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransFrom.GetLocation();
		FVector End = Start + (HitTarget - Start)*1.25;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if(World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);
			FVector BeamEnd = End;
			if(FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
				if(HasAuthority() && BlasterCharacter && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(
						BlasterCharacter,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
						);
				}

				if(ParticleSystem)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ParticleSystem,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
						);
				}

				
			}

			if(BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					SocketTransFrom
				);

				if(Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
}
