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

    virtual void TickComponent(float DeltaTime, 
        enum ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    FVector CurrentVelocity;
    FRotator AngularVelocity;
	
};
