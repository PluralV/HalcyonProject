// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPawnMovementComponent.h"

UShipPawnMovementComponent::UShipPawnMovementComponent() {
    //IMPLEMENT
    //
}

void UShipPawnMovementComponent::TickComponent(float DeltaTime, 
    enum ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
    
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    //IMPLEMENT

}

void UShipPawnMovementComponent::AddThrustInput(float ThrottleValue) {

}

void UShipPawnMovementComponent::SetThrustInput(float ThrustValue) {
    CurrentVelocity = CurrentVelocity.GetSafeNormal() * ThrustValue;
}

void UShipPawnMovementComponent::AddRotationalInput(FVector RotationInput) {

}

void UShipPawnMovementComponent::SetRotationalInput(FRotator Rotator) {
    AngularVelocity = Rotator;
}

