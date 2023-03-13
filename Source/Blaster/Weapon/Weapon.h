// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8 
{
	EWS_Initial UMETA(DisplayName="Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),

	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_Max UMETA(DisplayName = "DefaultMax")
};


UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName="HitScan Weapon"),
	EFT_Projectile UMETA(DisplayName="Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName="Shotgun Weapon"),
	
	EWS_Max UMETA(DisplayName = "DefaultMax")
};
UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);

	void SetWeaponState(EWeaponState State);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void OnRep_Owner() override;
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const {return WeaponMesh;}
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void Reload(int32 AmmoAmount);
	int32 AmmoReloadNeeded() const;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;
	
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsLeft;
	
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsRight;
	
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsTop;
	
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsBottom;
	void SetHUDAmmo();
	void IsShowHUDAmmo(bool bShow);

	/*
	 * Enable or Disable custom depth
	 */

	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	FORCEINLINE bool GetUseScatter() const{return bUseScatter;}
	FVector TraceEndWithScatter(const FVector& HitTarget);

protected:
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;
	
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = Fire)
	float FireDelay = 0.1f;
	
	UPROPERTY(EditAnywhere, Category = Fire)
	bool bAutomatic = true;
	
	virtual void BeginPlay() override;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multcast_Dropped();

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OVerlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OVerlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex
	);

	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnEquippedSecondary();
	virtual void OnDropped();
	

	UPROPERTY(EditDefaultsOnly, Category="Weapon Scatter")
	float DistanceToSphere = 800.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon Scatter")
	float SphereRadius = 75.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon Scatter")
	bool bUseScatter = false;
private:
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_WeaponState, Category="Weapon Properties")
	EWeaponState  WeaponState;
	
	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category="Weapon Properies")
	class UWidgetComponent* PickupWidget;
	
	UPROPERTY(EditAnywhere, Category="Weapon Properies")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category="Weapon Properties")
	EWeaponHighLight WeaponHighLight = EWeaponHighLight::EWHL_BLUE;
	
	int32 NumberOfWeaponHighLight() const;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_Ammo)
	int32 Ammo;
	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();
	UPROPERTY(EditDefaultsOnly)
	int32 MagCapacity;
	UPROPERTY(EditDefaultsOnly)
	int32 StartingRemainAmmo= 90;

	UPROPERTY()
	class ABlasterCharacter* OwnerCharacter;
	UPROPERTY()
	class ABlasterPlayerController* OwnerController;
	
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
public:
	FORCEINLINE float GetZoomedFOV() const{return ZoomedFOV;}
	FORCEINLINE float GetZoomInterpSpeed() const{return ZoomInterpSpeed;}

	FORCEINLINE float GetFireDelay() const{return FireDelay;}
	FORCEINLINE bool IsAutomatic() const{return bAutomatic;}

	bool IsEmpty() const;
	FORCEINLINE bool IsAmmoFull() const { return Ammo == MagCapacity;}
	FORCEINLINE EWeaponType GetWeaponType () const {return WeaponType;}
	int32  GetRemainAmmo();

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;
};
