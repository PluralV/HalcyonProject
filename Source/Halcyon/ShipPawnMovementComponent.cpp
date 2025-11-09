// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPawnMovementComponent.h"

UShipPawnMovementComponent::UShipPawnMovementComponent() {
    //IMPLEMENT
    //
    /*CurrentThrust = 0;
    AngularVelocity = FRotator(0, 0, 0);*/
}

void UShipPawnMovementComponent::TickComponent(float DeltaTime, 
    enum ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
    
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    //IMPLEMENT
    if (!UpdatedComponent) return;

    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(UpdatedComponent))
    {
       
        FVector Forward = PrimComp->GetForwardVector();
        FVector Right = PrimComp->GetRightVector();
        FVector Up = PrimComp->GetUpVector();
        FVector WorldUp = FVector::UpVector;

        // Apply rotational torque
        /*FVector Torque = WorldUp * AngularThrust.Yaw + Right*AngularThrust.Pitch;
        PrimComp->AddTorqueInRadians(Torque*TorqueMultiplier);*/


        /*FVector AngularVel = PrimComp->GetPhysicsAngularVelocityInRadians();
        float Dampening = 0.98f;
        AngularVel *= Dampening;*/
        /*PrimComp->SetPhysicsAngularVelocityInRadians(AngularVel, false);
        */

        //????
        FQuat PitchQuat = FQuat(Forward, FMath::DegreesToRadians(AngularThrust.Pitch * PitchRate * DeltaTime * -1.f));
        FQuat YawQuat = FQuat(WorldUp, FMath::DegreesToRadians(AngularThrust.Yaw * YawRate * DeltaTime));
        FQuat TargetQuat = YawQuat * PitchQuat * PrimComp->GetComponentQuat();

        float Alpha = 1.f; // tweak for smoothing
        FQuat NewQuat = FQuat::Slerp(PrimComp->GetComponentQuat(), TargetQuat, Alpha);
        PrimComp->SetWorldRotation(NewQuat);


        //SIMPLE LINEAR ACCELERATION (drop if necessary)
        /*PrimComp->SetWorldLocation(GetLocation()+(Right*CurrentVelocity*DeltaTime));


        CurrentVelocity += CurrentThrust * DeltaTime * AccelRate;
        if (CurrentVelocity >= SpeedConstant * (float)MovementEnergy) {
            CurrentVelocity = SpeedConstant * (float)MovementEnergy;
        }
        else if (CurrentVelocity < 0.f) {
            CurrentVelocity = 0.f;
        }*/

        // Apply linear thrust UNCOMMENT TO ATTEMPT REIMPLEMENTING PHYSICS
       /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
            FString::Printf(TEXT("SetThrustInput called currentthrust: %f"), CurrentThrust));*/
        FVector ForceToApply = Right * CurrentThrust * ForceMultiplier;
        PrimComp->AddForce(ForceToApply, NAME_None,false);
        FVector CurrentVelocity2 = PrimComp->GetPhysicsLinearVelocity();
        float NewSpeed = FVector::DotProduct(CurrentVelocity2, Right);
        FVector NewVelocity = Right * NewSpeed;
        PrimComp->SetPhysicsLinearVelocity(NewVelocity);

    }

}

void UShipPawnMovementComponent::AddThrustInput(float ThrottleValue) {
   /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
        FString::Printf(TEXT("AddThrustInput called ThrottleValue: %f"), ThrottleValue));*/
    CurrentThrust += ThrottleValue;
}

void UShipPawnMovementComponent::SetThrustInput(float ThrustValue) {
    CurrentThrust = ThrustValue;
   /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
        FString::Printf(TEXT("SetThrustInput called currentthrust: %f"), CurrentThrust));*/
}

void UShipPawnMovementComponent::AddRotationalInput(FVector RotationInput) {
    /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
        FString::Printf(TEXT("add rotational input called - x: %f, y: %f z: %f"), RotationInput.X, RotationInput.Y, RotationInput.Z));*/
    AngularThrust += FRotator(RotationInput.Y, RotationInput.X, RotationInput.Z);
}

void UShipPawnMovementComponent::SetRotationalInput(FRotator Rotator) {
    AngularThrust = Rotator;
   /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
        FString::Printf(TEXT("set rotational input called - Pitch: %f, Yaw: %f Roll: %f"), AngularThrust.Pitch, AngularThrust.Yaw, AngularThrust.Roll));*/
}

