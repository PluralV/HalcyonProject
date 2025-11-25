// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSystem.h"
#include "Kismet/KismetMathLibrary.h"

AWeaponSystem::AWeaponSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    TurretBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretBase"));
    TurretBase->SetupAttachment(RootComponent);

    BarrelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot"));
    BarrelPivot->SetupAttachment(TurretBase);

    Barrel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrel"));
    Barrel->SetupAttachment(BarrelPivot);

    TurretBase->SetSimulatePhysics(false);
    TurretBase->SetMassOverrideInKg(NAME_None, 0.f, true);

    Barrel->SetSimulatePhysics(false);
    Barrel->SetMassOverrideInKg(NAME_None, 0.f, true);

    Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
    Muzzle->SetupAttachment(Barrel);

}

void AWeaponSystem::BeginPlay()
{
    Super::BeginPlay();
    Barrel->AttachToComponent(BarrelPivot, FAttachmentTransformRules::KeepRelativeTransform);
    Muzzle->AttachToComponent(Barrel, FAttachmentTransformRules::KeepRelativeTransform);
    TimeSinceLastShot = FireRate;
}

void AWeaponSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    //If the weapon is charged, set time since last shot to the arming time
    if (!bIsDamaged && AllocatedEnergy >= MinEnergy) {
        TimeSinceLastShot += DeltaTime;
        if (bIsArming) {
            if (TimeSinceLastShot >= FireRate) bIsArming = false;
        }
    }
}

//Attempts to allocate "amt" energy to this weapon. Returns the actual amount of energy allocated.
int32 AWeaponSystem::AllocateEnergy(int32 amt) {
    int32 GapToMax = MaxEnergy - AllocatedEnergy;
    if (amt <= GapToMax) {
        AllocatedEnergy += amt;
        return amt;
    }
    else {
        AllocatedEnergy = MaxEnergy;
        return GapToMax;
    }
}

int32 AWeaponSystem::FreeEnergy(int32 amt) {
    if (AllocatedEnergy - amt >= 0) {
        AllocatedEnergy -= amt;
        return amt;
    }
    else {
        int32 temp = AllocatedEnergy;
        AllocatedEnergy = 0;
        return temp;
    }
}

//Attempts to damage this weapon system. Returns true if it is not already damaged and false if it is.
bool AWeaponSystem::CauseDamage() {
    bIsDamaged = true;
    if (AllocatedEnergy) {
        return true;
    }
    return false;
    
}

void AWeaponSystem::TrackTarget(float DeltaTime, AActor* CurrentTarget)
{
    if (!bIsDamaged && AllocatedEnergy >= MinEnergy) {
        FVector ShipLoc = CurrentTarget->GetActorLocation();
        FVector TargetVel = CurrentTarget->GetVelocity();
        FVector TurretLoc = TurretBase->GetComponentLocation();
        FVector ToShip = ShipLoc - TurretLoc;
        float Distance = ToShip.Size();
        FVector TargetLoc = ShipLoc;

        // Calculate lead only if not a missile
        if (!ProjectileClass) { return; }
        AProjectile* DefaultProj = ProjectileClass->GetDefaultObject<AProjectile>();
        if (!(DefaultProj->IsHomingProjectile())) {
            // Calculate lead using max speed
            float TimeToTarget = Distance / DefaultProj->GetProjectileSpeed();
            TargetLoc = ShipLoc + TargetVel * TimeToTarget;
        }
        FVector TargetDir = (TargetLoc - TurretLoc).GetSafeNormal();

        // Rotate turret (not barrel)
        // Convert the turretrotationaxis (barrels default facing x axis) to world space
        FVector WorldTurretAxis = TurretBase->GetComponentTransform().TransformVectorNoScale(TurretRotationAxis).GetSafeNormal();
        FVector TurretForward = TurretBase->GetComponentTransform().TransformVectorNoScale(TurretForwardAxis).GetSafeNormal();

        // Project target direction and turret forward direction onto plane perpendicular to turret rotation axis
        FVector ProjectedTargetDir = FVector::VectorPlaneProject(TargetDir, WorldTurretAxis).GetSafeNormal();
        FVector ProjectedForward = FVector::VectorPlaneProject(TurretForward, WorldTurretAxis).GetSafeNormal();
        // Angle between current forward and target in plane
        float TurretAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ProjectedForward, ProjectedTargetDir)));

        // Axis of rotation (sign) along WorldAxis
        float Sign = FVector::DotProduct(FVector::CrossProduct(ProjectedForward, ProjectedTargetDir), WorldTurretAxis) < 0.f ? -1.f : 1.f;

        // Clamp rotation speed
        float DeltaAngle = FMath::Min(RotationSpeed * DeltaTime, TurretAngle);
        float WouldBeRot = TurretCurrentRot + (DeltaAngle * Sign);
        /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
            FString::Printf(TEXT("DeltaAngle %f CurrentRot %f"), DeltaAngle, WouldBeRot));*/
        if (abs(WouldBeRot) <= MaxTurretArc) {
            // Apply rotation
            TurretCurrentRot += (DeltaAngle * Sign);
            FQuat DeltaQuat = FQuat(WorldTurretAxis, FMath::DegreesToRadians(DeltaAngle * Sign));
            FQuat NewRot = DeltaQuat * TurretBase->GetComponentQuat();
            TurretBase->SetWorldRotation(NewRot);
        }
        
       /* else {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
                FString::Printf(TEXT("NEAR ZERO")));
        }*/

        // Rotate barrel (pitch only)
        // axis to pitch around
        FVector PitchPlaneNormal = Barrel->GetComponentTransform().TransformVectorNoScale(BarrelRotationAxis).GetSafeNormal();
        // Direction to target in world space
        FVector BarrelLoc = Barrel->GetComponentLocation();
        FVector BarrelToTarget = (TargetLoc - BarrelLoc).GetSafeNormal();

        // Project target onto plane perpendicular to pitch axis
        FVector ProjectedDir = FVector::VectorPlaneProject(BarrelToTarget, PitchPlaneNormal).GetSafeNormal();

        // Project barrel direction onto plane perpendicular to pitch axis
        FVector MuzzleLoc = Muzzle->GetComponentLocation();
        FVector BarrelForward = (MuzzleLoc - BarrelLoc);
        FVector ProjectedBarrelDir = FVector::VectorPlaneProject(BarrelForward, PitchPlaneNormal).GetSafeNormal();
        //FVector BarrelForward = FVector::VectorPlaneProject(Barrel->GetForwardVector(), PitchPlaneNormal).RotateAngleAxis(90.f, PitchPlaneNormal).GetSafeNormal();


        // Angle between current barrel direction and target in pitch plane
        float Dot = FVector::DotProduct(ProjectedBarrelDir, ProjectedDir);
        float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));
        


        // Determine rotation direction (sign)
        Sign = FVector::DotProduct(FVector::CrossProduct(ProjectedBarrelDir, ProjectedDir), PitchPlaneNormal) < 0.f ? -1.f : 1.f;
        //Sign = FVector::DotProduct(FVector::CrossProduct(ProjectedBarrelDir, ProjectedDir), PitchPlaneNormal) < 0.f ? 1.f : -1.f;

       /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
            FString::Printf(TEXT("Dot is %f, destination angle %f, sign %f"), Dot, Angle, Sign));*/
        // Incremental rotation in degrees (clamp by max rotation speed)
        DeltaAngle = FMath::Min(RotationSpeed * DeltaTime, Angle) * Sign;
        float NextPitch = DeltaAngle + BarrelCurrentPitch;
        /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
            FString::Printf(TEXT("DeltaAngle %f NextPitch %f"), DeltaAngle, NextPitch));*/
       /*if (!(NextPitch > 0 || NextPitch < -90))*/
        if (abs(NextPitch) <= MaxPitchArc)
        {
           BarrelCurrentPitch = NextPitch;
           // Compute rotation+transaltion around pivot 
           FVector PivotLoc = BarrelPivot->GetComponentLocation();
           FVector ToBarrel = Barrel->GetComponentLocation() - PivotLoc;

           // Rotation delta
           FQuat DeltaQuat = FQuat(PitchPlaneNormal, FMath::DegreesToRadians(DeltaAngle));

           // New world location
           FVector NewLoc = PivotLoc + DeltaQuat.RotateVector(ToBarrel);
           // New world rotation
           FQuat NewRot = DeltaQuat * Barrel->GetComponentQuat();
           Barrel->SetWorldLocationAndRotation(NewLoc, NewRot);

           // clamp pitch between 0–90
           /*BarrelLocalRot.Pitch = FMath::Clamp(BarrelLocalRot.Pitch + DeltaAngle, -90.f, 0.f);
           Barrel->SetRelativeRotation(BarrelLocalRot);*/
        }

        // Fire weapon if weapon is within certain angle
        /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
            FString::Printf(TEXT("TurretAngle %f BarrelAngle %f"), TurretAngle, Angle))*/;
        if (Angle < MaxFiringAngle && TurretAngle < MaxFiringAngle)
        {
            /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
                FString::Printf(TEXT("Attempting to fire weapon: %s"),*this->GetName()));*/
            bTargetInArc = true;
            //if (ProjectileClass) {
            //    FVector SpawnLocation = Muzzle->GetComponentLocation();
            //    FRotator SpawnRotation = Barrel->GetComponentRotation();
            //    // Spawn projectile
            //    FActorSpawnParameters SpawnParams;
            //    SpawnParams.Owner = this;
            //    SpawnParams.Instigator = GetInstigator();
            //    AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
            //    TimeSinceLastShot = 0;
            //    if (Projectile)
            //    {
            //        /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
            //            FString::Printf(TEXT("Calling FireInDirection")));*/
            //        Projectile->FireInDirection(-BarrelForward);
            //        TimeSinceLastShot = 0;
            //    }
            //}
        }
        else {
           /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
                FString::Printf(TEXT("Weapon %s is not in arc."), *this->GetName()));*/
            bTargetInArc = false;
        }
    }
}

void AWeaponSystem::FireWeapon(AActor* Target) {
    if (!bIsDamaged && bTargetInArc && TimeSinceLastShot >= FireRate) {
        //get barrel right again
        FVector PitchPlaneNormal = Barrel->GetRightVector();
        // Project barrel direction onto plane perpendicular to pitch axis
        FVector SpawnLocation = Muzzle->GetComponentLocation();
        FVector BarrelLoc = Barrel->GetComponentLocation();
        FVector BarrelForward = (SpawnLocation - BarrelLoc);
        FVector ProjectedBarrelDir = FVector::VectorPlaneProject(BarrelForward, PitchPlaneNormal).GetSafeNormal();
        if (ProjectileClass) {
            FRotator SpawnRotation = Barrel->GetComponentRotation();
            // Spawn projectile
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = GetInstigator();
            AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
            if (Projectile)
            {
                /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
                    FString::Printf(TEXT("Calling FireInDirection")));*/
                Projectile->SetupHoming(Target);
                Projectile->FireInDirection(ProjectedBarrelDir);
                TimeSinceLastShot = 0.f;
                bIsArming = true;
            }
        }
    }
    /*else {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
            FString::Printf(TEXT("Weapon %s is not in arc."), *this->GetName()));
    }*/
}
