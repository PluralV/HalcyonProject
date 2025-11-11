// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSystem.h"

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

}

void AWeaponSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TimeSinceLastShot += DeltaTime;
}


void AWeaponSystem::TrackTarget(float DeltaTime, AActor* CurrentTarget)
{
    FVector TargetLoc = CurrentTarget->GetActorLocation();
    FVector TurretLoc = TurretBase->GetComponentLocation();
    FVector TargetDir = (TargetLoc - TurretLoc).GetSafeNormal();

    // Convert the turretrotationaxis (barrels default facing upwards) to world space
    FVector WorldTurretAxis = TurretBase->GetComponentTransform().TransformVectorNoScale(TurretRotationAxis).GetSafeNormal();
    FVector TurretForward = TurretBase->GetForwardVector();

    // Project target direction and turret forward direction onto plane perpendicular to rotation axis
    FVector ProjectedTargetDir = FVector::VectorPlaneProject(TargetDir, WorldTurretAxis).GetSafeNormal();
    FVector ProjectedForward = FVector::VectorPlaneProject(TurretForward, WorldTurretAxis).GetSafeNormal();

    if (!ProjectedTargetDir.IsNearlyZero() && !ProjectedForward.IsNearlyZero())
    {
        // Angle between current forward and target in plane
        float Angle = FMath::Acos(FVector::DotProduct(ProjectedForward, ProjectedTargetDir));

        // Axis of rotation (sign) along WorldAxis
        float Sign = FVector::DotProduct(FVector::CrossProduct(ProjectedForward, ProjectedTargetDir), WorldTurretAxis) < 0.f ? -1.f : 1.f;

        // Clamp rotation speed
        float DeltaAngle = FMath::Min(RotationSpeed * DeltaTime, FMath::RadiansToDegrees(Angle));

        // Apply rotation
        FQuat DeltaQuat = FQuat(WorldTurretAxis, FMath::DegreesToRadians(DeltaAngle * Sign));
        FQuat NewRot = DeltaQuat * TurretBase->GetComponentQuat();
        TurretBase->SetWorldRotation(NewRot);
    }

    // Rotate barrel (pitch only)
    
    // COMPUTE DELTA ANGLE

    // axis to pitch around
    FVector PitchPlaneNormal = Barrel->GetRightVector();
    // Direction to target in world space
    FVector BarrelLoc = Barrel->GetComponentLocation();
    FVector BarrelToTarget = -(TargetLoc - BarrelLoc).GetSafeNormal();

    // Project target onto plane perpendicular to pitch axis
    FVector ProjectedDir = FVector::VectorPlaneProject(BarrelToTarget, PitchPlaneNormal).GetSafeNormal();

    // Project barrel direction onto plane perpendicular to pitch axis
    FVector BarrelForward = FVector::VectorPlaneProject(Barrel->GetForwardVector(), PitchPlaneNormal).RotateAngleAxis(90.f, PitchPlaneNormal).GetSafeNormal()   ;

    // Angle between current barrel direction and target in pitch plane
    float Dot = FVector::DotProduct(BarrelForward, ProjectedDir);
    float Angle = FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f));

    // Determine rotation direction (sign)
    float Sign = FVector::DotProduct(FVector::CrossProduct(BarrelForward, ProjectedDir), PitchPlaneNormal) < 0.f ? -1.f : 1.f;

    // Incremental rotation in degrees (clamp by max rotation speed)
    float DeltaAngle = FMath::Min(RotationSpeed * DeltaTime, FMath::RadiansToDegrees(Angle)) * Sign;
    float NextPitch = DeltaAngle + BarrelCurrentPitch;
    if (NextPitch > 0 || NextPitch < -90)
    {
        return;
    }
    BarrelCurrentPitch += DeltaAngle;
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

    // Fire weapon if weapon is within certain angle
    if (FMath::RadiansToDegrees(Angle) < MaxFiringAngle && TimeSinceLastShot > FireRate)
    {
        if (ProjectileClass) {
            FVector SpawnLocation = Muzzle->GetComponentLocation();
            FRotator SpawnRotation = Barrel->GetComponentRotation();
            // Spawn projectile
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = GetInstigator();
            AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
            TimeSinceLastShot = 0;
            if (Projectile)
            {
                /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
                    FString::Printf(TEXT("Calling FireInDirection")));*/
                Projectile->FireInDirection(-BarrelForward);
                TimeSinceLastShot = 0;
            }
        }
    }
    

}
