// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.h"
#include "WeaponSystem.generated.h"

UCLASS()
class HALCYON_API AWeaponSystem : public AActor
{
    GENERATED_BODY()

public:
    AWeaponSystem();

protected:
    virtual void BeginPlay() override;
    float BarrelCurrentPitch = -90.f;
    float TimeSinceLastShot = 0.f;
    

public:
    virtual void Tick(float DeltaTime) override;

    /* Root component of the weapon system */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TurretBase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* Barrel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* BarrelPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Muzzle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    FVector TurretRotationAxis = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float RotationSpeed = 60.0f;

    /* Fire weapon if within this angle*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float MaxFiringAngle = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float FireRate = 1.5f; // shots per second

    /* Function to track target */
    void TrackTarget(float DeltaTime, AActor* CurrentTarget);
};
