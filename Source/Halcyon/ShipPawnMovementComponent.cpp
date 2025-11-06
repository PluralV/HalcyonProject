// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPawnMovementComponent.h"

UShipPawnMovementComponent::UShipPawnMovementComponent(
    float PitchRate, 
    float YawRate, 
    float SpeedConstant, 
    float ThrustConstant) {
    //IMPLEMENT
    //
}

UShipPawnMovementComponent::UShipPawnMovementComponent() {
    Super();
}

void UShipPawnMovementComponent::TickComponent(float DeltaTime, 
    enum ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
    
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    //IMPLEMENT

}

void UShipPawnMovementComponent::AddThrustInput(float ThrottleValue) {

}

void UShipPawnMovementComponent::AddRotationalInput(FVector RotationInput) {

}

