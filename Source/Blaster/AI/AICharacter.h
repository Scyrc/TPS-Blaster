// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AICharacter.generated.h"

UCLASS()
class BLASTER_API AAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAICharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UPawnSensingComponent* PawnSensingComponent;

	UFUNCTION()
	void OnPawnSeen(APawn* Pawn);
public:	
	virtual void Tick(float DeltaTime) override;

	//bool IsWeaponEquipped() const;

private:
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	//class UCombatComponent* Combat;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	//class UBuffComponent* Buff;


	//void SpawnDefaultWeapon();

	//UPROPERTY(EditDefaultsOnly)
	//TSubclassOf<AWeapon> DefaultWeapon;


};
