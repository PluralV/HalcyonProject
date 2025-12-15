// Fill out your copyright notice in the Description page of Project Settings.


#include "CautiousAIController.h"
#include <Kismet/GameplayStatics.h>
#include "Kismet/KismetMathLibrary.h"
#include "ShipPawn.h"
#include "WeaponSystem.h"
#include "ShipPawnMovementComponent.h"

void ACautiousAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ACautiousAIController::BeginPlay()
{
	Super::BeginPlay();
}

void ACautiousAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

//always run away
FVector ACautiousAIController::AcquireLookAtPoint()
{
	if (!ControlledShip) return FVector(0.f,0.f,0.f);
	{
		if (!CurrentTarget) {
			if (AShipPawn* PlayerShip = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))) {
				FVector PlayerLoc = PlayerShip->GetActorLocation();
				FVector MyLoc = ControlledShip->GetActorLocation();
				FVector LookAtPoint;

				FVector FlankDirection = (MyLoc - PlayerLoc).GetSafeNormal();
				FlankDirection = FVector::CrossProduct(FlankDirection, FVector::UpVector);
				LookAtPoint = PlayerLoc + FlankDirection * 4500 + FVector::UpVector * HeightOffset;

				return LookAtPoint;
			}else return ControlledShip->GetActorLocation();
		}
	}

	FVector PlayerLoc = CurrentTarget->GetActorLocation();
	FVector MyLoc = ControlledShip->GetActorLocation();
	FVector LookAtPoint;

	LookAtPoint = MyLoc + (-1.f * (PlayerLoc - MyLoc));
	return LookAtPoint;

}

void ACautiousAIController::EngageTarget()
{
	return Super::EngageTarget();
}

void ACautiousAIController::AllocateEnergy()
{
	return Super::AllocateEnergy();
}
