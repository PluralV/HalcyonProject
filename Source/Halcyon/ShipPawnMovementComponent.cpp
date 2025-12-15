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
        
        AngularThrust.Pitch = FMath::FInterpTo(
            AngularThrust.Pitch,
            TargetAngularThrust.Pitch,
            DeltaTime,
            1
        );
        AngularThrust.Yaw = FMath::FInterpTo(
            AngularThrust.Yaw,
            TargetAngularThrust.Yaw,
            DeltaTime,
            1
        );
        /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
            FString::Printf(TEXT("in tick  - Pitch: %f, Yaw: %f Roll: %f"), AngularThrust.Pitch, AngularThrust.Yaw, AngularThrust.Roll));*/

        FQuat PitchQuat = FQuat(Forward, FMath::DegreesToRadians(AngularThrust.Pitch * PitchRate * DeltaTime * -1.f));
        FQuat YawQuat = FQuat(WorldUp, FMath::DegreesToRadians(AngularThrust.Yaw * YawRate * DeltaTime));
        FQuat TargetQuat = YawQuat * PitchQuat * PrimComp->GetComponentQuat();
        FRotator TargetRot = TargetQuat.Rotator();
        TargetRot.Pitch = 0.f; // set roll to 0
        TargetQuat = TargetRot.Quaternion();
        float Alpha = 1.f; // tweak for smoothing
        FQuat NewQuat = FQuat::Slerp(PrimComp->GetComponentQuat(), TargetQuat, Alpha);
        PrimComp->SetWorldRotation(NewQuat);
        // set angular velocity to 0 to prevent ship rotating forever when colliding
        PrimComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);


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
        FVector ForceToApply = Right * CurrentThrust * ForceMultiplier * FMath::Sqrt((float)MovementEnergy);
        PrimComp->AddForce(ForceToApply, NAME_None,false);
        CurrentVelocity = PrimComp->GetPhysicsLinearVelocity();
        float NewSpeed = FVector::DotProduct(CurrentVelocity, Right);
       /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
            FString::Printf(TEXT("Before check: NewSpeed: %f SpeedConstant: %f MovementEnergy: %d"), NewSpeed, SpeedConstant, MovementEnergy));*/
        if (abs(NewSpeed) > SpeedConstant * (float)MovementEnergy) {
            NewSpeed = NewSpeed > 0 ? SpeedConstant * (float)MovementEnergy : -1.0 * SpeedConstant * (float)MovementEnergy;
           /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
                FString::Printf(TEXT("After check: NewSpeed: %f SpeedConstant: %f MovementEnergy: %d"), NewSpeed, SpeedConstant, MovementEnergy));*/
        }
        FVector NewVelocity = Right * NewSpeed;
        /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
            FString::Printf(TEXT("After check: NewVelocity: %f"), NewVelocity.Length()));*/
        PrimComp->SetPhysicsLinearVelocity(NewVelocity);
        CurrentVelocity = NewVelocity;
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
    TargetAngularThrust += FRotator(RotationInput.Y, RotationInput.X, RotationInput.Z);
}

void UShipPawnMovementComponent::SetRotationalInput(FRotator Rotator) {
    TargetAngularThrust = Rotator;
    /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
        FString::Printf(TEXT("set rotational input called - Pitch: %f, Yaw: %f Roll: %f"), AngularThrust.Pitch, AngularThrust.Yaw, AngularThrust.Roll));*/
}

void UShipPawnMovementComponent::SetMovementEnergy(int32 EnergyValue) {
    MovementEnergy = EnergyValue;
}

float UShipPawnMovementComponent::GetSpeed() {
    return CurrentVelocity.Length();
}

