// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "C4.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AC4 : public AWeapon
{
	GENERATED_BODY()
public:
	AC4();
	virtual void Dropped() override;
protected:
	virtual void OnEquipped() override;
	virtual void OnDropped() override;
	virtual void BeginPlay() override;
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BoomMesh;
	FTransform InitialTransform;

public:
	FORCEINLINE FTransform GetInitialTransform() const{return  InitialTransform;}
};
