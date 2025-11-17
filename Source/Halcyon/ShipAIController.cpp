// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipAIController.h"
#include <Kismet/GameplayStatics.h>
#include "Kismet/KismetMathLibrary.h"

void AShipAIController::BeginPlay()
{
    Super::BeginPlay();

    ControlledShip = Cast<AShipPawn>(GetPawn());
    if (!PlayerPawn) {
        PlayerPawn = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    }
}

// x = -1 up, x = 1 down, y = 1 right, y = -1 left
void AShipAIController::RotateToward(const FVector& TargetLocation)
{
    if (!ControlledShip) return;
    
    FVector ToTarget = TargetLocation - ControlledShip->GetActorLocation();
    ToTarget.Normalize();
    FRotator TargetRot = ToTarget.Rotation(); 
    FRotator CurrentRot = ControlledShip->GetActorRotation();
    CurrentRot.Yaw += 90.f; // add 90 for ship facing y axis
    float YawDelta = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, TargetRot.Yaw);
    float YawInput = (YawDelta > 0.f) ? 1.f : -1.f;
    float PitchDelta = FMath::FindDeltaAngleDegrees(CurrentRot.Roll, -TargetRot.Pitch);
    float PitchInput = (PitchDelta > 0.f) ? 1.f : -1.f;
    
    ControlledShip->MovementComponent->SetRotationalInput(FRotator(PitchInput, YawInput, 0.f));
}

void AShipAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!ControlledShip || !PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    FVector MyLoc = ControlledShip->GetActorLocation();
    float Distance = FVector::Dist(MyLoc, PlayerLoc);

    FVector LookAtPoint;

    if (Distance > SideOffset)  // if far, point towards the player
    {
        LookAtPoint = PlayerLoc;
    }
    else  // if closer, orbit by pointing at player offset to the side
    {
        FVector Right = PlayerPawn->GetActorForwardVector();
        LookAtPoint = PlayerLoc + Right * SideOffset;
    }
    RotateToward(LookAtPoint);

    // thrust forwards
    ControlledShip->MovementComponent->SetThrustInput(.5);

    // target if in range
    if (Distance < FireRange)
    {
        ControlledShip->SetTarget(PlayerPawn);
    }
}
