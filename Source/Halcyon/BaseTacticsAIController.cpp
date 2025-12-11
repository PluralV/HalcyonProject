// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseTacticsAIController.h"
#include <Kismet/GameplayStatics.h>
#include "Kismet/KismetMathLibrary.h"
#include "ShipPawn.h"
#include "WeaponSystem.h"
#include "ShipPawnMovementComponent.h"

//
void ABaseTacticsAIController::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
    TimeSinceLastRealloc += DeltaSeconds; 
    TimeSinceLastTarget += DeltaSeconds;
    if (TimeSinceLastRealloc >= EnergyCycle) {
        AllocateEnergy();
    }
    if (TimeSinceLastTarget >= TargetCycle && EligibleTargets.Num() > 1) {
        ReappraiseTargets();
    }

    if (CurrentTarget) {
        FVector ToTarget = CurrentTarget->GetActorLocation() - ControlledShip->GetActorLocation();
        RotToTarget = ToTarget.Rotation();
        RangeToTarget = ToTarget.Length();
        FRotator CurrentRot = ControlledShip->GetActorRotation();
        float YawField = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw + 90, RotToTarget.Yaw) + 30.f;
        ShieldFacingIndex = (int)(YawField / 60.f);
        for (int32 i = 0; i < WeaponCount; i++) {
            CheckIsBearing(i);
        }
    }

    FVector LookAtPoint = AcquireLookAtPoint();

    RotateToward(LookAtPoint);

    // thrust forwards
    ControlledShip->MovementComponent->SetThrustInput(1);

    //Choose whether to hold or use weapons
    EngageTarget();

}

void ABaseTacticsAIController::BeginPlay() {
    Super::BeginPlay();
    ControlledShip = Cast<AShipPawn>(GetPawn());
    HeightOffset = FMath::RandRange(-1000.f, 1000.f);
    TimeSinceLastRealloc = EnergyCycle;
    //Acquire all targets on first tick (once all initialized)
    WeaponCount = ControlledShip->GetWeaponComponents().Num();
    GetWorldTimerManager().SetTimerForNextTick(this, &ABaseTacticsAIController::AcquireEligibleTargets);
    CurrentWeaponStatus = ECombatStatus::ArmedClose;
}

void ABaseTacticsAIController::OnPossess(APawn* InPawn) {
    Super::OnPossess(InPawn);

}

void ABaseTacticsAIController::AcquireEligibleTargets() {
    TArray<AActor*> PotentialTargetShips;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShipPawn::StaticClass(), PotentialTargetShips);
    int32 MaxScore = 0;
    for (AActor* PotentialTarget : PotentialTargetShips) {
        if (AShipPawn* TgShip = Cast<AShipPawn>(PotentialTarget)) {
            if (TgShip->Team != ControlledShip->Team) {
                //LOGIC: Appraise target based on range, size, importance, to create a sort of integer "score" to rank targets by
                FTargetScore TargetEntry;
                TargetEntry.Target = PotentialTarget;
                TargetEntry.AppraisalScore = AppraiseTarget(PotentialTarget);
                int32 IndexAdded = EligibleTargets.Add(TargetEntry);
                //If this is the current highest-scoring target, swap it with the first element
                if (TargetEntry.AppraisalScore > MaxScore) {
                    MaxScore = TargetEntry.AppraisalScore;
                    EligibleTargets[IndexAdded] = EligibleTargets[0];
                    EligibleTargets[0] = TargetEntry;
                }
            }
        } //POTENTIAL ELSE: also have ability to shoot down missiles
    }
    //If there are no enemies around, just loiter
    if (EligibleTargets.IsEmpty()) {
        CurrentTarget = nullptr;
        return;
    }
    AcquireBestTarget();
    
}

void ABaseTacticsAIController::AcquireBestTarget() {
    if (AShipPawn* TargetAsShip = Cast<AShipPawn>(CurrentTarget)) {
        TargetAsShip->OnShipDestroyed.RemoveDynamic(this, &ABaseTacticsAIController::HandleTargetDestroyed);
    }
    CurrentTarget = EligibleTargets[0].Target;
    if (AShipPawn* TargetAsShip = Cast<AShipPawn>(CurrentTarget)) {
        TargetAsShip->OnShipDestroyed.AddDynamic(this, &ABaseTacticsAIController::HandleTargetDestroyed);
    }
    TimeSinceLastTarget = 0.f;
}

void ABaseTacticsAIController::HandleTargetDestroyed(int32 CauseOfDeath, AShipPawn* DestroyedShip)
{
    CurrentTarget = nullptr;
    EligibleTargets.Swap(0, EligibleTargets.Num() - 1);
    EligibleTargets.Pop();
    ReappraiseTargets();
}


// If a new actor spawns, check its score against the current best target, then ignore
void ABaseTacticsAIController::HandleNewSpawn(AActor* SpawnedActor)
{
    if (!(Cast<AShipPawn>(SpawnedActor))) return;
    FTargetScore TargetEntry;
    TargetEntry.Target = SpawnedActor;
    TargetEntry.AppraisalScore = AppraiseTarget(SpawnedActor);
    if (TargetEntry.AppraisalScore > EligibleTargets[0].AppraisalScore) {
        EligibleTargets.Add(EligibleTargets[0]);
        EligibleTargets[0] = TargetEntry;
        AcquireBestTarget();
    }
    else {
        EligibleTargets.Add(TargetEntry);
    }
    
}


//Calculate an appraisal score for a target roughly based on distance, whether it is facing the controlled ship, how dangerous the ship is,
//and how much damage it has taken as a percentage of health
int32 ABaseTacticsAIController::AppraiseTarget(AActor* PossibleTarget) {
    int32 Score =
        //Closer distance = higher score
        (100000.f/ControlledShip->GetDistanceTo(PossibleTarget))
        //Facing the target = higher score
        + (int)(-30 * (PossibleTarget->GetActorLocation().Normalize() * ControlledShip->GetActorLocation().Normalize()));

    if (AShipPawn* TargetAsShip = Cast<AShipPawn>(PossibleTarget)) {
        
        //More damage done to the ship = higher score
        Score += 
            (int)(100.f * (float)(TargetAsShip->GetMaxHullIntegrity()-TargetAsShip->GetHullIntegrity()) / (float)TargetAsShip->GetMaxHullIntegrity()) 
            //More energy = higher score
            + TargetAsShip->GetMaxEnergyCurr() / 2;
        
        //More weapons with higher energy load = higher score
        for (UChildActorComponent* CAC : TargetAsShip->GetWeaponComponents()) {
            if (AWeaponSystem* AWS = Cast<AWeaponSystem>(CAC->GetChildActor())) {
                if (!AWS->bIsDamaged) {
                    Score += (AWS->MinEnergy + (AWS->MaxEnergy - AWS->MinEnergy) / 2);
                }
            }
        }

        //If the target is a mission protect objective, multiply its appraisal score
        if (TargetAsShip->GetIsObjective()) {
            return Score * 3 / 2;
        }
    }
    return Score;
}


//Go back through the list of targets and calculate an appraisal score for each
void ABaseTacticsAIController::ReappraiseTargets() {
    if (EligibleTargets.IsEmpty()) CurrentTarget = nullptr;
    for (FTargetScore& EligibleTarget : EligibleTargets) {
        EligibleTarget.AppraisalScore = AppraiseTarget(EligibleTarget.Target);
        if (EligibleTarget.AppraisalScore > EligibleTargets[0].AppraisalScore) {
            FTargetScore Temp = EligibleTargets[0];
            EligibleTargets[0].AppraisalScore = EligibleTarget.AppraisalScore;
            EligibleTargets[0].Target = EligibleTarget.Target;
            EligibleTarget.AppraisalScore = Temp.AppraisalScore;
            EligibleTarget.Target = Temp.Target;
        }
    }
    AcquireBestTarget();
}

//Relative to that ship and current state, determine where to move next
FVector ABaseTacticsAIController::AcquireLookAtPoint() {
    if (!ControlledShip || !CurrentTarget) return FVector(0,0,0);

    FVector PlayerLoc = CurrentTarget->GetActorLocation();
    FVector MyLoc = ControlledShip->GetActorLocation();
    float Distance = FVector::Dist(MyLoc, PlayerLoc);
    FVector LookAtPoint;
    
    if (Distance > SideOffset)  // if far, point towards the player
    {
        if (CurrentWeaponStatus == ECombatStatus::Arming) {
            LookAtPoint = MyLoc + (- 1.f * (PlayerLoc - MyLoc));
        }
        else {
            LookAtPoint = PlayerLoc;
        }
        
    }
    else  // if closer, orbit by pointing at player offset to the side
    {
        
        FVector FlankDirection = (MyLoc - PlayerLoc).GetSafeNormal();
        if (CurrentWeaponStatus == ECombatStatus::Arming) { 
            FlankDirection *= -1.f; 
            LookAtPoint = PlayerLoc + FlankDirection * MaxRange + FVector::UpVector * HeightOffset;
        } 
        else {
            FlankDirection = FVector::CrossProduct(FlankDirection, FVector::UpVector);
            LookAtPoint = PlayerLoc + FlankDirection * SideOffset + FVector::UpVector * HeightOffset;
        }
    }

    return LookAtPoint;
}

//Determine what to do with weapons - hold fire, shoot, or potentially free/allocate further energy
void ABaseTacticsAIController::EngageTarget() {
    int32 FiredWeapons = 0;
    
    for (FWeaponCapability& WPEntry : ShipWeapons) {
        if (WPEntry.bInRange && WPEntry.bInArc && (WPEntry.Weapon->DamageScaling == 0 || WPEntry.Weapon->MaxRange / 3 <= RangeToTarget)) {
            WPEntry.Weapon->FireWeapon(CurrentTarget);
        }
        if (WPEntry.Weapon->bIsArming) FiredWeapons++;
    }
    if (FiredWeapons >= WeaponCount / 2) {
        CurrentWeaponStatus = ECombatStatus::Arming;
    }
    else {
        CurrentWeaponStatus = ECombatStatus::ArmedClose;
    }
}

void ABaseTacticsAIController::CheckIsBearing(int32 WeaponIndex)
{
    if (WeaponIndex < WeaponCount) {
        FWeaponCapability* WPEntry = &ShipWeapons[WeaponIndex];
        WPEntry->bInArc = WPEntry->Weapon->IsInArc(CurrentTarget);
        WPEntry->bInRange = WPEntry->Weapon->IsInRange(CurrentTarget);
        WPEntry->bCouldOverload = (WPEntry->Weapon->MaxEnergy > WPEntry->Weapon->MinEnergy) && WPEntry->Weapon->IsInRange(CurrentTarget, true);
    }
    return;
}

//Allocate energy to systems. In overrides can take into account things like range to target, health, etc.
void ABaseTacticsAIController::AllocateEnergy() {
    ////Determine movement needs by current target range
    //if (CurrentTarget) {
    //    if (RangeToTarget >= MaxRange) {
    //        if (ControlledShip->GetMovementEnergy() < ControlledShip->GetMaxEnergyCurr()) {
    //            int32 MoveAllocGoal = (ControlledShip->GetMaxEnergyCurr() - ControlledShip->GetMovementEnergy()) / 2;
    //            if (!MoveAllocGoal) MoveAllocGoal = 1;
    //            FreeEnergyByPriority(MoveAllocGoal);
    //            ControlledShip->AllocateMovement(MoveAllocGoal);
    //        }
    //    }
    //    else {
    //        
    //        for (int i = 0; i < WeaponCount; i++) {
    //            CheckIsBearing(i);
    //            if (AWeaponSystem* TWS = ShipWeapons[i].Weapon){
    //                if (ShipWeapons[i].bInArc && ShipWeapons[i].bInRange) {
    //                    if (TWS->AllocatedEnergy < TWS->MinEnergy) {
    //                        TWS->AllocateEnergy(TWS->MinEnergy - TWS->AllocatedEnergy);
    //                    }
    //                }
    //                
    //            
    //                //IF the weapon is a) charged b) not arming and c) not in-arc and range, free its energy
    //                //IF the weapon is arming, leave it be
    //                //IF the weapon is a) not arming b) not charged and c) in-arc and range, charge it
    //                //IF the weapon is a) not arming b) charged c) overloadable and d) in overload range, overload it by 1 step

    //                if (TWS->MinEnergy <= ControlledShip->GetMaxEnergyAvailable()) ControlledShip->AllocateWeapon(i, TWS->MinEnergy);
    //            }
    //        }
    //    }
    //}

    TArray<AWeaponSystem*> ComeBackLater;
    for (FWeaponCapability& WeaponCapability : ShipWeapons) {
        if (AWeaponSystem* AWS = WeaponCapability.Weapon) {
            //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Weapon with lesser range found %d"), (int)AWS->MaxRange));
            if (AWS->MinEnergy > AWS->AllocatedEnergy) {
                if (WeaponCapability.bInRange) {
                    int32 AttemptedEnergy = AWS->MinEnergy - AWS->AllocatedEnergy;
                    int32 Success = AWS->AllocateEnergy(AttemptedEnergy);
                    if (Success < AttemptedEnergy) ComeBackLater.Add(AWS);
                }
                if (WeaponCapability.bCouldOverload) {
                    ComeBackLater.Add(AWS);
                }
            }
            else {
                if (!WeaponCapability.bInRange && !AWS->bIsArming) AWS->FreeEnergy(AWS->AllocatedEnergy);
            }
        }
    }
    
    for (AWeaponSystem* AWS : ComeBackLater) {
        int32 AttemptedEnergy = AWS->MinEnergy - AWS->AllocatedEnergy;
        int32 Success = AWS->AllocateEnergy(AttemptedEnergy);
        if (Success < AttemptedEnergy) break;
    }

    ControlledShip->AllocateMovement(ControlledShip->GetMaxEnergyAvailable());
    TimeSinceLastRealloc = 0.f;
}

//Attempts to free EnergyGoal energy, starting with least important systems and working up to more important ones
void ABaseTacticsAIController::FreeEnergyByPriority(int32 EnergyGoal) {
    int32 EnergyFreed = 0;
    if (CurrentTarget) {
        //first dealloc from shields not facing the target
        if (RangeToTarget > DangerZone) {
            //SHIELD FREE
            for (int32 i = 0; i < 6; i++) {
                //If not facing, free and add to the sum freed
                if (i != ShieldFacingIndex) { 
                    //Check if there is reinforcement
                    if (int32 ReinStrength = ControlledShip->GetCurrentShieldReinforcement(i)) {
                        //If there is reinforcement, check if it would be enough to free
                        if (EnergyFreed + ReinStrength > EnergyGoal) {
                            ReinStrength = EnergyGoal - EnergyFreed;
                        }
                        ControlledShip->FreeReinforceShield(ReinStrength, i);
                        EnergyFreed += ReinStrength;
                        if (EnergyFreed >= EnergyGoal) {
                            return;
                        }
                    }
                }
            }
            //WEAPON FREE: Start with weapons not facing the target
            

        }
    }
}



// x = -1 up, x = 1 down, y = 1 right, y = -1 left
void ABaseTacticsAIController::RotateToward(const FVector& TargetLocation)
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