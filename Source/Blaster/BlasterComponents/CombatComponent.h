// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include"Blaster/BlasterTypes/CombatState.h"

#include "CombatComponent.generated.h"

class AWeapon;
class ABlasterCharacter;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend  class ABlasterCharacter;
	void EquipWeapon(AWeapon* WeaponToEquip);
	void SwapWeapons();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();
	
	void FireButtonPressed(bool bPressed);
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();
	void JumpToShotgunEnd();
	
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	FORCEINLINE int32 GetGrenades() const{return Grenades;}

	void PickAmmo(float AmmoAmount, EWeaponType WeaponType);

	void UpdateCarriedAmmo();

	FORCEINLINE AWeapon* GetEquippedWeapon() const{return  EquippedWeapon;}

	bool ShouldSwapWeapon();

	bool bLocallyReloading = false;

protected:
	
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();
	
	void Fire();
	void FireHitScanWeapon();
	void FireProjectileWeapon();
	void FireShotgun();

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTarget);

	void Reload();
	UFUNCTION(Server, Reliable)	
	void ServerReload();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	
	void HandleReload();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;
	
	void DropEquippedWeapon();

	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachFlagToLeftHand(AWeapon* Flag);

	void AttachActorToBackpack(AActor* ActorToAttach);

	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);

	void ReloadEmptyWeapon();

	void ShowAttachedGrenade(bool bShow);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

private:
	UPROPERTY()
	ABlasterCharacter* Character;
	UPROPERTY()

	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing=OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;
	
	UPROPERTY(ReplicatedUsing="OnRep_Aiming")
	bool bAiming;
	UFUNCTION()
	void OnRep_Aiming();

	bool bAimButtonPressed;
	
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	FHUDPackage HUDPackage;

	FVector HitTarget;


	float DefaultFOV;
	float CurrentFOV;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;


	void InterpFOV(float DeltaTime);

	/*
	 *  Automatic Fire
	 */

	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire() const;

	//Carried Ammo for the currently-equipped weapon
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo = -1;
	
	UPROPERTY(EditDefaultsOnly)
	int32 MaxCarriedAmmo = 500;
	
	UFUNCTION()
	void OnRep_CarriedAmmo();

	
	//void InitializeCarriedAmmo();
	UPROPERTY(VisibleAnywhere)
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
	UPROPERTY(ReplicatedUsing=Onrep_Grenades)
	int32 Grenades = 4;
	UFUNCTION()
	void OnRep_Grenades();
	int32 MaxGrenades = 4;

	void UpdateHUDGrenades();

	UPROPERTY(ReplicatedUsing=OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;

	UPROPERTY()
	AWeapon* TheFlag;
	
	UFUNCTION()
	void OnRep_HoldingTheFlag();
};

