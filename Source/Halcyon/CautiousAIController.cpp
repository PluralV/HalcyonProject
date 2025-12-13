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

FVector ACautiousAIController::AcquireLookAtPoint()
{
	return Super::AcquireLookAtPoint();
}

void ACautiousAIController::EngageTarget()
{
	return Super::EngageTarget();
}

void ACautiousAIController::AllocateEnergy()
{
	return Super::AllocateEnergy();
}
