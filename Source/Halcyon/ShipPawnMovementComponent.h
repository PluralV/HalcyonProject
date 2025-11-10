// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "ShipPawnMovementComponent.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class HALCYON_API UShipPawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
    UShipPawnMovementComponent();
    UFUNCTION(BlueprintCallable)
    void AddThrustInput(float ThrottleValue);

    UFUNCTION(BlueprintCallable)
    void AddRotationalInput(FVector RotationInput);

    UFUNCTION()
    void SetRotationalInput(FRotator Rotator);

    UFUNCTION()
    void SetThrustInput(float ThrottleValue);

    UFUNCTION()
    void SetMovementEnergy(int32 EnergyValue);

    UFUNCTION(BlueprintCallable)
    float GetSpeed();

    virtual void TickComponent(float DeltaTime, 
        enum ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;

    // Amount of force applied for full throttle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float ForceMultiplier = 200000.f;

    // Amount of torque applied for rotational input
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float TorqueMultiplier = 50000.f;

    //Engine settings
    UPROPERTY(BlueprintReadWrite, Category = "Base Mobility Stats")
    float SpeedLimit;
    //Maneuverability
    UPROPERTY(BlueprintReadWrite, Category = "Base Mobility Stats")
    float PitchRate;
    UPROPERTY(BlueprintReadWrite, Category = "Base Mobility Stats")
    float YawRate;
    //Top speed, multiplied by amount of energy allocated to movement to get the maximum velocity (magnitude)
    UPROPERTY(BlueprintReadWrite, Category = "Base Mobility Stats")
    float SpeedConstant;

    float AccelRate;

private:
    float CurrentThrust=0.f;
    FRotator AngularThrust;
    float AngularAccel = 5.f;
    FVector CurrentVelocity;
    //TODO: CHANGE TO 0 ONCE IMPLEMENTING ENERGY ALLOCATION FOR REAL AND SET ONLY W/ENERGY ALLOCATION
    int32 MovementEnergy = 0;
};
