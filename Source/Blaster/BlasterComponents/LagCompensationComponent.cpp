// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Blaster.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}




void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}



void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveFramePackage();
}

void ULagCompensationComponent::SaveFramePackage()
{
	if(Character == nullptr || !Character->HasAuthority()) return;
	if(FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		
		//ShowFramePackage(ThisFrame, FColor::Orange);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame, FColor::Orange);
	}
}
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if(Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		for(auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

FFramePackage ULagCompensationComponent::InterBetweenFrames(const FFramePackage& OlderFrame,const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);
	FFramePackage InterFramePackage;
	InterFramePackage.Time = HitTime;
	for(auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterBoxInfo;
		InterBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterFraction);
		InterBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterFraction);
		InterBoxInfo.BoxExtent = YoungerBox.BoxExtent;
		InterFramePackage.HitBoxInfo.Add(BoxInfoName, InterBoxInfo);
	}

	return InterFramePackage;
}



void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr) return;
    for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
    {
	    if(HitBoxPair.Value != nullptr)
	    {
			FBoxInformation BoxInfo;
	    	BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
	    	BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
	    	BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();

	    	OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
	    }
    }
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr) return;
	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr) return;
	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnable)
{
	if(HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnable);
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for(auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	const FFramePackage FrameToCheck =  GetFrameToCheck(HitCharacter, HitTime);

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);

}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	const FFramePackage FrameToCheck =  GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<ABlasterCharacter*> HitCharacters, const FVector_NetQuantize& TraceStart,const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for(auto& HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}

	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;

	if(bReturn) return FFramePackage();
	//Frame package that we check to verify a hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	const TDoubleLinkedList<FFramePackage>& Histroy = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = Histroy.GetTail()->GetValue().Time;
	const float NewestHistoryTime = Histroy.GetHead()->GetValue().Time;

	if(OldestHistoryTime > HitTime)
	{
		// too for back
		return FFramePackage();
	}
	if(OldestHistoryTime == HitTime)
	{
		// too for back
		FrameToCheck = Histroy.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if(NewestHistoryTime <= HitTime)
	{
		FrameToCheck = Histroy.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = Histroy.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	while (Older->GetValue().Time > HitTime)
	{
		if(Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if(Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}

	if(Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	if(bShouldInterpolate)
	{
		FrameToCheck = InterBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	if(HitCharacter && Character && Character->GetEquippedWeapon() &&Confirm.bHitConfirmed)
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ServerProjectileScoreRequest_Implementation(ABlasterCharacter* HitCharacter,const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	if(HitCharacter && Character && Character->GetEquippedWeapon() &&Confirm.bHitConfirmed)
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}
void ULagCompensationComponent::ServerShotgunScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr || Character == nullptr || Character->GetEquippedWeapon() == nullptr) continue;
		float TotalDamage = 0.f;
		if (Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * Character->GetEquippedWeapon()->GetHeadShotDamage();
			TotalDamage += HeadShotDamage;
		}
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * Character->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentPackage;
	CacheBoxPositions(HitCharacter, CurrentPackage);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	//Enable Collision for the head first

	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	bool bheadhit =false;
	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart)*1.25;
	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);

		if(ConfirmHitResult.bBlockingHit)
		{
			if(ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if(Box)
				{
					//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 10.f);
					ResetHitBoxes(HitCharacter, CurrentPackage);
					EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
					bheadhit = true;
					return FServerSideRewindResult{true, true};
				}
			}
			
			
		}
		if(!bheadhit) // didn't hit head, check the rest of boxes
		{
			for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if(HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
			);

			if(ConfirmHitResult.bBlockingHit)
			{
				if(ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if(Box)
					{
						//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Orange, false,10.f);
						ResetHitBoxes(HitCharacter, CurrentPackage);
						EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
						return FServerSideRewindResult{true, false};
					}
				}
				
			}
		}
	}

	ResetHitBoxes(HitCharacter, CurrentPackage);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false};
	
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package,ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if(HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentPackage;
	CacheBoxPositions(HitCharacter, CurrentPackage);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	//Enable Collision for the head first

	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FPredictProjectilePathParams PathParams;
	//PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 20.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	//PathParams.DrawDebugTime = 5.f;
	//PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	bool bHeadHit = false;
	if(PathResult.HitResult.bBlockingHit) // hit head
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit head error"))

		if(PathResult.HitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			if(Box)
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit head"))
				//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 10.f);
				ResetHitBoxes(HitCharacter, CurrentPackage);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				bHeadHit = true;
				return FServerSideRewindResult{true, true};
			}
			/*UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(PathResult.HitResult.Component);
			if(CapsuleComponent)
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit CapsuleComponent"))
				//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 10.f);
			}
			USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(PathResult.HitResult.Component);
			if(SkeletalMeshComponent)
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit SkeletalMeshComponent"))
				//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 10.f);
			}*/
		}
	}
	if(!bHeadHit)  // check the rest of the boxes
	{
		for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if(HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if(PathResult.HitResult.bBlockingHit) // hit body
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit body error"))

			if(PathResult.HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
				if(Box)
				{
					UE_LOG(LogTemp, Warning, TEXT("Hit body"))
					//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Orange, false, 10.f);
					ResetHitBoxes(HitCharacter, CurrentPackage);
					EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
					return FServerSideRewindResult{true, false};
				}
			}
		}
	}
	
	ResetHitBoxes(HitCharacter, CurrentPackage);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false};
	
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for(auto& Frame : FramePackages)
	{
		if(Frame.Character == nullptr) return FShotgunServerSideRewindResult();
	}
	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame : FramePackages)
	{
		// Enable collision for the head first
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}

	UWorld* World = GetWorld();
	// check for head shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				if(ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if(Box)
					{
						//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false,10.f);
						if (ShotgunResult.HeadShots.Contains(BlasterCharacter))
						{
							ShotgunResult.HeadShots[BlasterCharacter]++;
						}
						else
						{
							ShotgunResult.HeadShots.Emplace(BlasterCharacter, 1);
						}
					}
				}
			}
		}
	}

	// enable collision for all boxes, then disable for head box
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// check for body shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				if(ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if(Box)
					{
						//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Orange, false, 10.f);
						if (ShotgunResult.BodyShots.Contains(BlasterCharacter))
						{
							ShotgunResult.BodyShots[BlasterCharacter]++;
						}
						else
						{
							ShotgunResult.BodyShots.Emplace(BlasterCharacter, 1);
						}
					}
				}
			}
		}
	}

	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotgunResult;
	 
	
}
