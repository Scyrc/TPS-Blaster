// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharcter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (BlasterCharcter == nullptr)
	{
		BlasterCharcter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}

	if (BlasterCharcter == nullptr) return;

	FVector Velocity = BlasterCharcter->GetVelocity();

	Velocity.Z = 0.f;

	speed = Velocity.Size();

	bIsInAir = BlasterCharcter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = BlasterCharcter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;


}
