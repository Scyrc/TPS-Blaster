// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"


void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlasterHUD::AddCharacterOverlay()
{
	const APlayerController* PlayerController = GetOwningPlayerController();

	if(PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(GetWorld(), CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}

	
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();
	
	if(GEngine)
	{
		FVector2D ViewPortSize;
		GEngine->GameViewport->GetViewportSize(ViewPortSize);

		const FVector2D ViewPortCenter = FVector2D(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);

		const float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		if(HUDPackage.CrosshairsCenter)
		{
			const FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewPortCenter, Spread);
		}
		if(HUDPackage.CrosshairsLeft)
		{
			const FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewPortCenter, Spread);
		}
		if(HUDPackage.CrosshairsRight)
		{
			const FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewPortCenter, Spread);
		}
		if(HUDPackage.CrosshairsTop)
		{
			const FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewPortCenter, Spread);
		}
		if(HUDPackage.CrosshairsBottom)
		{
			const FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewPortCenter, Spread);
		}
	}
}



void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	const FVector2D TextureDrawPoint(
		ViewportCenter.X - TextureWidth / 2.f + Spread.X,
		ViewportCenter.Y - TextureHeight / 2.f + + Spread.Y);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		HUDPackage.CrosshairsColor
		);

}
