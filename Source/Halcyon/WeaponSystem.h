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
    float BarrelCurrentPitch = 0.f;
    float TurretCurrentRot = 0.f;
    float TimeSinceLastShot = 0.f;
    

public:
    virtual void Tick(float DeltaTime) override;
    
    /* Function to track target */
    
    void TrackTarget(float DeltaTime, AActor* CurrentTarget);

    UFUNCTION(BlueprintCallable)
    int32 AllocateEnergy(int32 amt); //returns the actual amount of energy allocated/freed for owning ship calculation purposes

    UFUNCTION(BlueprintCallable)
    int32 FreeEnergy(int32 amt);

    UFUNCTION(BlueprintCallable)
    bool CauseDamage();

    void FireWeapon();

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    FVector TurretForwardAxis = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    FVector BarrelRotationAxis = FVector::RightVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
    FVector BarrelForwardAxis = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float RotationSpeed = 60.0f;

    /* Fire weapon if within this angle*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float MaxFiringAngle = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float MaxTurretArc = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float MaxPitchArc = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float FireRate = 1.5f; // shots per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float MaxRange = 2500; // distance traveled (in-game cm) before loss

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float CoolDownTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    int32 MinEnergy;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    int32 MaxEnergy;

    int32 AllocatedEnergy;

    

    bool bIsDamaged = false;
    bool bTargetInArc = false;
};
