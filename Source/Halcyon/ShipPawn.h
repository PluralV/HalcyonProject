// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "ShipPawn.generated.h"

class UInputMappingContext;
class UInputAction;


UCLASS(Blueprintable, BlueprintType)
class HALCYON_API AShipPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AShipPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* ShipMesh;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpaceshipMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpaceshipThrustComponent* ThrustComponent;*/

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* ShipMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ThrottleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PitchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DecelerateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* YawAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* TargetAction;

	//Handle player input
	void Look(const FInputActionValue& Value);
	void Throttle(const FInputActionValue& Value);
	void Decelerate(const FInputActionValue& Value);
	void Steer(const FInputActionValue& Value);
	void ZeroThrottle();
	void ZeroDecel();
	void ZeroSteering();
	void Target(const FInputActionValue& Value);

	// Camera rotation speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraRotationSpeed = 1.f;

	//Ship base stats
	//Hull integrity
	int32 HullIntegrity = 32;
	//Shield values
	TArray<int32> ShieldFacings = { 30,24,18,18,18,24 };
	
	//Hull damage: Internal damage that does nothing
	int32 CenterHull = 0;
	int32 ForwardHull = 8;
	int32 AftHull = 12;
	
	//Energy stats: The total amount of energy generated is the total integer sum of these
	int32 LeftEng = 16;
	int32 RightEng = 16;
	int32 CenterEng = 0;
	int32 PowerReactor = 4;

	//

	//ARRAYS FOR STORING SYSTEMS


	//Engine settings
	UPROPERTY()
	float SpeedLimit = 1750.f;
	float CurrentThrottle = 0.f;
	const float DefaultSpeedLimit = 1750.f;
	//Maneuverability
	float PitchRate = 20.f;
	float YawRate = 20.f;
	//Top speed, multiplied by amount of energy allocated to movement to get the maximum velocity (magnitude)
	float SpeedConstant = 20.f;
	
	


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:

	//Power allocated to base non-external systems
	int32 MovementEnergy = 0;

	FVector CurrentVelocity;
	FRotator AngularVelocity;
};
