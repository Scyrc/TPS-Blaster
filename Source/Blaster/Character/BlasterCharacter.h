// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Sound/SoundCue.h"

#include "BlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

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

	/*
	 * Hit Boxes used for server-side rewind
	 */
	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;
	
	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();
	void SetTeamColor(ETeam Team);
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class ULagCompensationComponent* LagCompensation;
	
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

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* SwapMontage;
	
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

	UPROPERTY(VisibleAnywhere, Category="Elim")	
	UMaterialInstance* DissolveMaterialInstance;

	/*
	 * Team color
	 */

	UPROPERTY(EditAnywhere, Category="Elim")	
	UMaterialInstance* RedDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category="Elim")	
	UMaterialInstance* RedMaterialInstance;
	
	UPROPERTY(EditAnywhere, Category="Elim")	
	UMaterialInstance* BlueDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category="Elim")	
	UMaterialInstance* BlueMaterialInstance;

	UPROPERTY(EditAnywhere, Category="Elim")	
	UMaterialInstance* OriginalMaterialInstance;
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

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	void ElimTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	bool bLeftGame = false;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem;

	UPROPERTY()
	class UNiagaraComponent* CrownComponent;
	
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
	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;
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
	void PlaySwapMontage();

	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}

	FORCEINLINE bool ShouldRotateRootBone() const{ return  bRotateRootBone;}

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool PlayLeftGame);

	FORCEINLINE bool IsElimmed() const{ return  bElimmed;}
	void Elim(bool PlayLeftGame);

	FORCEINLINE float GetHealth()const{return  CurrentHealth;}
	FORCEINLINE void SetHealth(float Amount){CurrentHealth = Amount;}
	FORCEINLINE float GetMaxHealth()const{return  MaxHealth;}

	FORCEINLINE float GetShield()const{return  CurrentShield;}
	FORCEINLINE void SetShield(float Amount){CurrentShield = Amount;}
	FORCEINLINE float GetMaxShield()const{return  MaxShield;}

	ECombatState GetCombatState() const;

	FORCEINLINE UCombatComponent* GetCombatComponent()const {return Combat;}
	FORCEINLINE UBuffComponent* GetBuffComponent()const {return Buff;}
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent()const {return LagCompensation;}

	FORCEINLINE bool GetDisableGameplay() const {return bDisableGamePlay;}
	FORCEINLINE UAnimMontage* GetReloadMontage() const {return ReloadMontage;}
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const {return AttachedGrenade;}
	bool IsLocallyReloading() const ;

	bool bFinishedSwapping = false;
};
