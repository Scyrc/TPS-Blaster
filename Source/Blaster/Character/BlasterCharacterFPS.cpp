// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacterFPS.h"

#include "Camera/CameraComponent.h"

ABlasterCharacterFPS::ABlasterCharacterFPS()
{
	GetFollowCamera()->SetupAttachment(GetMesh());
	GetFollowCamera()->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = true;
}
