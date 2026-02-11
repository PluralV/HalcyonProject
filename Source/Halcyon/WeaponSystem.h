// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.h"
#include "WeaponSystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnergyChangedExternal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFireAway);


UCLASS(Blueprintable, BlueprintType)
class HALCYON_API AWeaponSystem : public AActor
{
    GENERATED_BODY()

public:
    AWeaponSystem();

    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnEnergyChangedExternal OnEnergyChangedExternal;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnFireAway OnFireAway;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Name")
    FText WeaponName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Name")
    FText WeaponAbbreviatedName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Name")
    FText WeaponDesc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Name")
    FText WeaponArc; //To be added manually?
protected:
    virtual void BeginPlay() override;
    float BarrelCurrentPitch = 0.f;
    float TurretCurrentRot = 0.f;
    
    

public:
    virtual void Tick(float DeltaTime) override;
    
    /* Function to track target */
    
    void TrackTarget(float DeltaTime, AActor* CurrentTarget);

    //returns the actual amount of energy allocated/freed for owning ship calculation purposes
    UFUNCTION(BlueprintCallable)
    float AllocateEnergy(float amt); 

    //returns the actual amount of energy allocated/freed for owning ship calculation purposes
    UFUNCTION(BlueprintCallable)
    float FreeEnergy(float amt);

    UFUNCTION(BlueprintCallable)
    bool CauseDamage();

    //returns true if weapon is in arc of the target
    bool IsInArc(AActor* Target);

    //returns true if weapon is in arc of the target
    bool IsInRange(AActor* Target, bool bOverloadRange=false);

    void FireWeapon(AActor* Target);

    /* Root component of the weapon system */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    TSubclassOf<AProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float RotationSpeed = 60.0f;

    /* Fire weapon if within this angle*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float MaxFiringAngle = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float MaxTurretArc = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float MaxTurretArcNegative = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float MaxPitchArc = 90.0f;

    // seconds before next shot
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float FireRate = 1.5f; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* FireSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    // distance traveled (in-game cm) before loss
    float MaxRange = 2500; 
    
    // distance traveled (in-game cm) before loss if overloaded
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float MaxRangeOverload = 2500; 

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Energy")
    float MinEnergy;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Energy")
    float MaxEnergy;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Energy")
    float EnergyStep;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon Energy")
    float AllocatedEnergy;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Energy")
    int32 BaseDamage = 8;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Energy")
    int32 DamageScaling = 4; //At each successive rangeband (MaxRange/3 meters traveled), lose 1/DamageScaling * BaseDamage damage

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Energy")
    float OverloadScaling = 0.f; //For each additional EnergyStep energy, add OverloadScaling * the total calculated damage to the final value

    int32 team = -1;
    int32 ControlGroup = 0;
    float TimeSinceLastShot = 0.f;
private:
    //Amount of time it takes after firing before energy can be freed from the weapon (naturally)
    const float FireEnergyLock = 8.0f;
public:
    //repreesents whether weapon has taken a hit
    bool bIsDamaged = false;
    //represents whether current target is in arc or not
    bool bTargetInArc = false;
    //If this is true, the weapon has not cooled down yet (i.e. less than FireRate time has passed since last shot)
    bool bIsArming = false;
    //If this is true, further energy cannot be allocated/freed to this weapon
    bool bIsFiring = false;
};
