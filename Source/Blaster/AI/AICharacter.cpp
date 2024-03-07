// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Perception/PawnSensingComponent.h"


AAICharacter::AAICharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComponent");

	//Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	//Combat->SetIsReplicated(true);
}


void AAICharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//SpawnDefaultWeapon();
}

void AAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	PawnSensingComponent->OnSeePawn.AddDynamic(this, &AAICharacter::OnPawnSeen);
}


void AAICharacter::OnPawnSeen(APawn* Pawn)
{
	AAIController* AIC = Cast<AAIController>(GetController());
	if(AIC)
	{
		UBlackboardComponent* BBComp = AIC->GetBlackboardComponent();

		BBComp->SetValueAsObject("TargetActor", Pawn);

		DrawDebugString(GetWorld(), GetActorLocation(), "Player SPOTTED", nullptr, FColor::Red, 3.0f, true);
	}
}

void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/*void AAICharacter::SpawnDefaultWeapon()
{
	//BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	UWorld* World = GetWorld();
	if(World && Combat)
	{
		AWeapon* StartingWeapon =  World->SpawnActor<AWeapon>(DefaultWeapon);
		StartingWeapon->bDestroyWeapon = true;
		Combat->EquipWeapon(StartingWeapon);
	}
}*/



