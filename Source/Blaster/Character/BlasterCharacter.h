// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Sound/SoundCue.h"

#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Destroyed() override;
	UPROPERTY(Replicated)
	bool bDisableGamePlay=false;
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void SpawnDefaultWeapon();

protected:

	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPress();
	void CrouchButtonPress();
	void ReloadButtonPress();
	void AimButtonPress();
	void GrenadeButtonPress();

	void CalculateAO_Pitch();
	void AimOffset(float DeltaTime);
	void SimproxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const  UDamageType* DamageType,  AController* InstigatedBy, AActor* DamageCauser);


	void PollInit();

	void RotateInPlace(float DeltaTime);

	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapon();

private:
	UPROPERTY(VisibleAnywhere, Category="Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess= "true")) 
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class UBuffComponent* Buff;
	
	UFUNCTION(Server, Reliable)	
	void ServerEquipButtonPress();

	


	float AO_Yaw;
	float InterAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* ReloadMontage;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* ThrowGrenadeMontage;
	
	void HideCameraIfCharacterClose();
	
	UPROPERTY(EditAnywhere, Category="Camera")
	float CameraThreshold = 200.f;
	bool bRotateRootBone;

	float TurnThreshold = 15.f;
	float ProxyYaw;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotationCurrentFrame;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed() const;
	
	UPROPERTY(EditAnywhere, Category = playerState)
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = playerState)
	float CurrentHealth = 100.f;
	
	UFUNCTION()
	void OnRep_Health(float LastHealth);
	/*
	 * Player Shield
	 */

	UPROPERTY(EditAnywhere, Category = playerState)
	float MaxShield = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = playerState)
	float CurrentShield = 100.f;
	
	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	void ElimTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	/*
	 * Dissolve Effect
	 */
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	UPROPERTY(VisibleAnywhere, Category="Elim")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category="Elim")	
	UMaterialInstance* DissolveMaterialInstance;

	/*
	 * Elim Bot
	 */
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;
	
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	/*
	 * Grenade
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/*
	 * Default Weapon
	 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AWeapon> DefaultWeapon;
public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped() const;
	bool IsAiming() const;
	FORCEINLINE float GetAO_Yaw() const{return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const{return AO_Pitch;}
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTuringInPlace() const {return TurningInPlace;}
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();

	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}

	FORCEINLINE bool ShouldRotateRootBone() const{ return  bRotateRootBone;}

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	FORCEINLINE bool IsElimmed() const{ return  bElimmed;}
	void Elim();

	FORCEINLINE float GetHealth()const{return  CurrentHealth;}
	FORCEINLINE void SetHealth(float Amount){CurrentHealth = Amount;}
	FORCEINLINE float GetMaxHealth()const{return  MaxHealth;}

	FORCEINLINE float GetShield()const{return  CurrentShield;}
	FORCEINLINE void SetShield(float Amount){CurrentShield = Amount;}
	FORCEINLINE float GetMaxShield()const{return  MaxShield;}

	ECombatState GetCombatState() const;

	FORCEINLINE UCombatComponent* GetCombatComponent()const {return Combat;}
	FORCEINLINE UBuffComponent* GetBuffComponent()const {return Buff;}

	FORCEINLINE bool GetDisableGameplay() const {return bDisableGamePlay;}
	FORCEINLINE UAnimMontage* GetReloadMontage() const {return ReloadMontage;}
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const {return AttachedGrenade;}

};
