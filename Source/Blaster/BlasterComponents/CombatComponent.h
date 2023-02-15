// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f;

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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	UFUNCTION()
	void OnRep_EquippedWeaopn();

	void FireButtonPressed(bool bPressed);
	UFUNCTION(Server, Reliable)
	void ServerFire();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

public:	
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	ABlasterCharacter* Character;

	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeaopn)
	AWeapon* EquippedWeapon;
	
	UPROPERTY(Replicated)
	bool bAiming;
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;
		
};
