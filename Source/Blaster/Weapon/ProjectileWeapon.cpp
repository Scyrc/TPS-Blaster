// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	UWorld* World = GetWorld();

	if(MuzzleFlashSocket == nullptr || World == nullptr) return;
	
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector ToTarget = HitTarget - SocketTransform.GetLocation();
	FRotator TargetRotation = ToTarget.Rotation();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = InstigatorPawn;

	AProjectile* SpawnProjectile = nullptr;
	if(bUseServerSideReWind)
	{
		if(InstigatorPawn->HasAuthority())
		{
			if(InstigatorPawn->IsLocallyControlled()) // server, host use ssr
			{
				SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
				SpawnProjectile->bUseServerSideRewind = false;
				SpawnProjectile->Damage = Damage;
			}
			else     // server, not locally controlled, spawn not replicated no ssr
			{
				SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
				SpawnProjectile->bUseServerSideRewind = false;
			}
		}
		else  // client using ssr
		{
			if(InstigatorPawn->IsLocallyControlled()) // client, locally controlled, spawn not replicated ssr
			{
				SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
				SpawnProjectile->bUseServerSideRewind = true;
				SpawnProjectile->TraceStart = SocketTransform.GetLocation();
				SpawnProjectile->InitialVelocity = SpawnProjectile->GetActorForwardVector() * SpawnProjectile->InitialSpeed;
				SpawnProjectile->Damage = Damage;

			}
			else     // client, not locally controlled, spawn not replicated no ssr
			{
				SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
				SpawnProjectile->bUseServerSideRewind = false;
				
			}
		}
	}
	else // weapon not using ssr
	{
		if(InstigatorPawn->HasAuthority())
		{
			SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
			SpawnProjectile->bUseServerSideRewind = false;
			SpawnProjectile->Damage = Damage;
		}
	}
}
