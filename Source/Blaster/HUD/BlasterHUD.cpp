// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

#include "Announcement.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "ElimAnnouncement.h"
#include "HeroSelectWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"


void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
	const APlayerController* PlayerController = GetOwningPlayerController();

	if(PlayerController && CharacterOverlayClass)
	{
		
		if(CharacterOverlay)
		{
			CharacterOverlay->RemoveFromParent();
		}
		CharacterOverlay = CreateWidget<UCharacterOverlay>(GetWorld(), CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddHeroSelectWidget()
{
	UE_LOG(LogTemp, Warning, TEXT("AddHeroSelectWidget"));

	const APlayerController* PlayerController = GetOwningPlayerController();

	if(PlayerController && HeroSelectWidgetClass)
	{
		if(HeroSelectWidget)
		{
			HeroSelectWidget->RemoveFromParent();
		}

		HeroSelectWidget = CreateWidget<UHeroSelectWidget>(GetWorld(), HeroSelectWidgetClass);
		if(HeroSelectWidget)
		{
			HeroSelectWidget->AddToViewport();
		}
	}
}

void ABlasterHUD::RemoveHeroSelectWidget()
{
	if(HeroSelectWidget)
	{
		HeroSelectWidget->RemoveFromParent();
	}
}

void ABlasterHUD::AddAnnouncement()
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;


	if(OwningPlayerController && AnnouncementClass)
	{
		if(Announcement)
		{
			Announcement->RemoveFromParent();
		}
		Announcement = CreateWidget<UAnnouncement>(GetWorld(), AnnouncementClass);
		Announcement->AddToViewport();
	}

}

void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;

	if(OwningPlayerController && ElimAnnouncementClass)
	{
		ElimAnnouncement = CreateWidget<UElimAnnouncement>(GetWorld(), ElimAnnouncementClass);
		if(ElimAnnouncement)
		{
			ElimAnnouncement->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncement->AddToViewport();

			for(UElimAnnouncement* Msg : ElimMessage)
			{
				if(Msg && Msg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if(CanvasPanelSlot)
					{
						const FVector2D Position = CanvasPanelSlot->GetPosition();
						FVector2D NewPosition(
							Position.X,
							Position.Y - CanvasPanelSlot->GetSize().Y
						);
						CanvasPanelSlot->SetPosition(NewPosition);
					}
				}
			}
			
			ElimMessage.Add(ElimAnnouncement);
			FTimerHandle ElimMsgTimer;
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncement);

			GetWorldTimerManager().SetTimer(
				ElimMsgTimer,
				TimerDelegate,
				ElimAnnouncementTime,
				false);
		}

	
	}
}

void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if(MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
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


