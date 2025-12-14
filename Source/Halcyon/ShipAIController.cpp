#include "ShipAIController.h"
#include <Kismet/GameplayStatics.h>
#include "Kismet/KismetMathLibrary.h"
#include "WeaponSystem.h"

void AShipAIController::BeginPlay()
{
    Super::BeginPlay();
    ControlledShip = Cast<AShipPawn>(GetPawn());
    if (!PlayerPawn) {
        PlayerPawn = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    }
    else {
        //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No player pawn set"));
    }
    HeightOffset = FMath::RandRange(-1000.f, 1000.f);
}

void AShipAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    ControlledShip = Cast<AShipPawn>(InPawn);
    for (UChildActorComponent* WeaponComp : ControlledShip->GetWeaponComponents()) {
        if (AWeaponSystem* AWS = Cast<AWeaponSystem>(WeaponComp->GetChildActor())) {
            //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Weapon with lesser range found %d"), (int)AWS->MaxRange));
            if (AWS->MaxRange < FireRange) FireRange = AWS->MaxRange;//Only fire weapons when in range
        }
    }
    //GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Yellow, FString::Printf(TEXT("AI Controller possessed: %s"), *InPawn->GetName()));

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
    float PitchDelta = FMath::FindDeltaAngleDegrees(CurrentRot.Roll, -TargetRot.Pitch);

    // ADD DEAD ZONE - don't rotate if close enough
    const float DeadZone = 5.0f; // Adjust this value

    float YawInput = 0.f;
    if (FMath::Abs(YawDelta) > DeadZone)
    {
        YawInput = (FMath::Abs(YawDelta) > 1.f ? 1.f : FMath::Abs(YawDelta)) * ((YawDelta > 0.f) ? 1.f : -1.f);
    }

    float PitchInput = 0.f;
    if (FMath::Abs(PitchDelta) > DeadZone)
    {
        PitchInput = (FMath::Abs(PitchDelta) > 1.f ? 1.f : FMath::Abs(PitchDelta)) * ((PitchDelta > 0.f) ? 1.f : -1.f);
    }

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
    else if (Distance > SideOffset * 8) // if closer, approach diagonally by pointing at player offset to the side
    {
        FVector FlankDirection = (MyLoc - PlayerLoc).GetSafeNormal();
        FlankDirection = FVector::CrossProduct(FlankDirection, FVector::UpVector);
        LookAtPoint = PlayerLoc + FlankDirection * SideOffset + FVector::UpVector * HeightOffset;
    }
    else { // otherwise attempt orbit
        FVector PlayerToAI = (MyLoc - PlayerLoc).GetSafeNormal();
        FVector FlankDirection = FVector::CrossProduct(PlayerToAI, FVector::UpVector);
        LookAtPoint = PlayerLoc + (FlankDirection * SideOffset) + (PlayerToAI * SideOffset * .65) + FVector::UpVector * HeightOffset;
        
    }
    RotateToward(LookAtPoint);

    // thrust forwards
    ControlledShip->MovementComponent->SetThrustInput(1);

    // target
    ControlledShip->SetTarget(PlayerPawn);
    // fire if in range

    if (Distance < FireRange)
    {
        ControlledShip->AIFireWeapon();
    }

    AllocateEnergy();
}

void AShipAIController::AllocateEnergy() {    
    TArray<UChildActorComponent*> WeaponComps = ControlledShip->GetWeaponComponents();
    for (int i = 0; i < WeaponComps.Num(); i++) {
        if (AWeaponSystem* TWS = Cast<AWeaponSystem>(WeaponComps[i]->GetChildActor())) {
            if (TWS->MinEnergy <= ControlledShip->GetMaxEnergyAvailable()) ControlledShip->AllocateWeapon(i, TWS->MinEnergy);
        }
    }

    ControlledShip->AllocateMovement(ControlledShip->GetMaxEnergyAvailable());

}