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
    if (ControlledShip) {
        //update records on whether target is in arc
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

        if (TimeSinceLastRealloc >= EnergyCycle) {
            AllocateEnergy();
        }
        if (TimeSinceLastTarget >= TargetCycle && EligibleTargets.Num() > 1) {
            ReappraiseTargets();
        }



        FVector LookAtPoint = AcquireLookAtPoint();

        RotateToward(LookAtPoint);

        // thrust forwards
        ControlledShip->MovementComponent->SetThrustInput(1);

        //Choose whether to hold or use weapons
        EngageTarget();
    }
}

void ABaseTacticsAIController::BeginPlay() {
    Super::BeginPlay();
    HeightOffset = FMath::RandRange(-1000.f, 1000.f);
    TimeSinceLastRealloc = EnergyCycle;
    
    CurrentWeaponStatus = ECombatStatus::ArmedClose;
}

//Sets up things like the weapon list/targets; has to be delayed in order to handle out-of-order initialization
void ABaseTacticsAIController::InitializeAfterLoad() {
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) {
        //GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow,FString::Printf(TEXT("Delaying InitializeAfterLoad")));
        GetWorldTimerManager().SetTimerForNextTick(this, &ABaseTacticsAIController::InitializeAfterLoad);
        return;
    }
    //GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, FString::Printf(TEXT("Made it here, for some reason")));
    ControlledShip = Cast<AShipPawn>(ControlledPawn);
    //Initialize weapons list
    TArray<UChildActorComponent*> WeaponComps = ControlledShip->GetWeaponComponents();
    WeaponCount = WeaponComps.Num();
    for (int32 i = 0; i < WeaponCount; i++) {
        if (AWeaponSystem* AWS = Cast<AWeaponSystem>(WeaponComps[i]->GetChildActor())) {
            //DEBUG DEBUG
            FText WeaponName = AWS->WeaponAbbreviatedName;
            UE_LOG(LogTemp, Warning, TEXT("Found weapon: %s"), *WeaponName.ToString());
            //GET RID OF ABOVE

            //Adds weaponcapability to the list
            int32 AtIndex = ShipWeapons.Add(FWeaponCapability());
            ShipWeapons[AtIndex].Weapon = AWS;
            ShipWeapons[AtIndex].ExpectedDamage = AWS->BaseDamage;
            ShipWeapons[AtIndex].Index = i;
            ShipWeapons[AtIndex].bInRange = false;
            ShipWeapons[AtIndex].bInArc = false;
            if (SideOffset > AWS->MaxRange / 2) SideOffset = FMath::Max(AWS->MaxRange/2, 1600.f);
           // GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, FString::Printf(TEXT("SideOffset: %d"),(int)SideOffset));
        }
        else {
            //DEBUG DEBUG
            UE_LOG(LogTemp, Warning, TEXT("Did not find weapon."));
        }
    }

    AcquireEligibleTargets();

    //Update whether weapons are bearing
    if (CurrentTarget) {
        //UE_LOG(LogTemp, Warning, TEXT("FOUND TARGET."));
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

    AllocateEnergy();
    CurrentWeaponStatus = ECombatStatus::ArmedClose;

}

void ABaseTacticsAIController::OnPossess(APawn* InPawn) {
    Super::OnPossess(InPawn);
    ControlledShip = Cast<AShipPawn>(GetPawn());
    //Acquire all targets on first tick (once all initialized)
    InitializeAfterLoad();
}

void ABaseTacticsAIController::AcquireEligibleTargets() {
    TArray<AActor*> PotentialTargetShips;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShipPawn::StaticClass(), PotentialTargetShips);
    int32 MaxScore = 0;
    for (AActor* PotentialTarget : PotentialTargetShips) {
        if (AShipPawn* TgShip = Cast<AShipPawn>(PotentialTarget)) {
            if (TgShip->Team != ControlledShip->Team) {
                //GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, FString::Printf(TEXT("Found a target")));
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
    ControlledShip->SetTarget(EligibleTargets[0].Target);
    if (AShipPawn* TargetAsShip = Cast<AShipPawn>(CurrentTarget)) {
        TargetAsShip->OnShipDestroyed.AddDynamic(this, &ABaseTacticsAIController::HandleTargetDestroyed);
    }
    TimeSinceLastTarget = 0.f;
}

void ABaseTacticsAIController::HandleTargetDestroyed(int32 CauseOfDeath, AShipPawn* DestroyedShip)
{
    CurrentTarget = nullptr;
    ControlledShip->SetTarget(nullptr);
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
            return Score * 3;
        }
    }
    
    return Score;
}


//Go back through the list of targets and calculate an appraisal score for each
void ABaseTacticsAIController::ReappraiseTargets() {
    if (EligibleTargets.IsEmpty()) CurrentTarget = nullptr;
    for (int32 i = 0; i < EligibleTargets.Num(); i++) {
        FTargetScore& EligibleTarget = EligibleTargets[i];
        if (EligibleTarget.Target) {
            EligibleTarget.AppraisalScore = AppraiseTarget(EligibleTarget.Target);
            if (EligibleTarget.AppraisalScore > EligibleTargets[0].AppraisalScore) {
                FTargetScore Temp = EligibleTargets[0];
                EligibleTargets[0].AppraisalScore = EligibleTarget.AppraisalScore;
                EligibleTargets[0].Target = EligibleTarget.Target;
                EligibleTarget.AppraisalScore = Temp.AppraisalScore;
                EligibleTarget.Target = Temp.Target;
            }
        }
        else {
            EligibleTargets.RemoveAt(i);
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
        //if (CurrentWeaponStatus == ECombatStatus::Arming) {
        //    LookAtPoint = MyLoc + (- 1.f * (PlayerLoc - MyLoc));
        //}
        //else {
            LookAtPoint = PlayerLoc;
        //}
        
    }
    else // if closer, approach diagonally by pointing at player offset to the side
    {
        FVector FlankDirection = (MyLoc - PlayerLoc).GetSafeNormal();
        FlankDirection = FVector::CrossProduct(FlankDirection, FVector::UpVector).GetSafeNormal();
        LookAtPoint = PlayerLoc + FlankDirection * SideOffset + FVector::UpVector * HeightOffset;
    }

    return LookAtPoint;
}

//Determine what to do with weapons - hold fire, shoot, or potentially free/allocate further energy
void ABaseTacticsAIController::EngageTarget() {
    int32 FiredWeapons = 0;
    
    for (FWeaponCapability& WPEntry : ShipWeapons) {
        if (WPEntry.Weapon->bIsArming) {
            FiredWeapons++;
            continue;
        }
        else if (WPEntry.bInRange && WPEntry.bInArc && (WPEntry.Weapon->DamageScaling == 0 || WPEntry.Weapon->MaxRange / 2 >= RangeToTarget || (WPEntry.Weapon->MaxRange < 2000 && 1200 >= RangeToTarget))) {
            WPEntry.Weapon->FireWeapon(CurrentTarget);
            if (WPEntry.Weapon->bIsArming)
                FiredWeapons++;
        }
        
    }
    if (FiredWeapons >= WeaponCount) {
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
    if (ControlledShip && ControlledShip->HasActorBegunPlay()) {
        //GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, FString::Printf(TEXT("Total Energy: %d"),ControlledShip->GetMaxEnergyAvailable()));
        ControlledShip->FreeMovement(ControlledShip->GetMovementEnergy() / 2);
        TArray<FWeaponCapability*> ComeBackLater;
        //UE_LOG(LogTemp, Warning, TEXT("ALLOCATING ENERGY TO WEAPONS:"));
        for (FWeaponCapability& WeaponCapability : ShipWeapons) {
            if (AWeaponSystem* AWS = WeaponCapability.Weapon) {
                //UE_LOG(LogTemp, Warning, TEXT("ALLOCATING ENERGY TO WEAPON %s?"), *AWS->WeaponAbbreviatedName.ToString());
                //If weapon has not been alloced yet, check if in range
                if (AWS->MinEnergy > AWS->AllocatedEnergy) {
                    //If weapon is in range, then allocate minimum energy to it
                    UE_LOG(LogTemp, Warning, TEXT("Weapon not already allocated."));
                    if (WeaponCapability.bInRange) {
                        UE_LOG(LogTemp, Warning, TEXT("Weapon in range."))
                            int32 AttemptedEnergy = AWS->MinEnergy - AWS->AllocatedEnergy;
                        int32 Success = ControlledShip->AllocateWeapon(WeaponCapability.Index, AttemptedEnergy);
                        UE_LOG(LogTemp, Warning, TEXT("Allocated %d/%d energy to weapon %s."), Success, AttemptedEnergy, *AWS->WeaponAbbreviatedName.ToString());
                        if (Success < AttemptedEnergy) ComeBackLater.Add(&WeaponCapability);
                    }

                }
                else {
                    UE_LOG(LogTemp, Warning, TEXT("Weapon already allocated."));
                    if (!WeaponCapability.bInRange && !AWS->bIsArming)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Weapon not in range and not arming - FREE NOW"));
                        ControlledShip->FreeWeapon(WeaponCapability.Index, AWS->AllocatedEnergy);
                    }
                    //handle this later
                    //if (WeaponCapability.bCouldOverload) {
                    //    ComeBackLater.Add(AWS);
                    //}
                }
            }
        }

        for (FWeaponCapability* WC : ComeBackLater) {
            int32 AttemptedEnergy = WC->Weapon->MinEnergy - WC->Weapon->AllocatedEnergy;
            int32 Success = ControlledShip->AllocateWeapon(WC->Index, AttemptedEnergy);
            if (Success < AttemptedEnergy) break;
        }

        ControlledShip->AllocateMovement(ControlledShip->GetMaxEnergyAvailable());
        TimeSinceLastRealloc = 0.f;
    }
    else {
        //GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow,FString::Printf(TEXT("Delaying AllocateEnergy")));
        GetWorldTimerManager().SetTimerForNextTick(this, &ABaseTacticsAIController::AllocateEnergy);
        return;
    }
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