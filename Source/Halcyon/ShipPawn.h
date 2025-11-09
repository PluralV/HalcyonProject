// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "ShipPawn.generated.h"

class UInputMappingContext;
class UInputAction;
class UShipPawnMovementComponent;
class AModularSystem;

UCLASS(Blueprintable, BlueprintType)
class HALCYON_API AShipPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AShipPawn();

	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UShipPawnMovementComponent* MovementComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* ShipMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ThrottleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SteerAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DecelerateAction;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Base System Stats")
	int32 HullIntegrity = 32;
	//Shield values
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Base System Stats")
	TArray<int32> ShieldFacings = { 30,24,18,18,18,24 };
	//ShieldReinforcements tracks any extra power put into a given shield and should be reduced FIRST by any external damage
	TArray<int32> ShieldReinforcements = { 0,0,0,0,0,0 };
	//Stores current strength of each shield
	TArray<int32> ShieldFacingsCurr = {0,0,0,0,0,0};
	
	//Hull damage: Internal damage that does nothing
	int32 CenterHull = 0;
	int32 CenterHullCurr;
	int32 ForwardHull = 8;
	int32 ForwardHullCurr;
	int32 AftHull = 12;
	int32 AftHullCurr;
	
	//Energy stats: The total amount of energy generated is the total integer sum of these
	//Curr values indicate the current number remaining (initialised to the same amount as the base)
	UPROPERTY(BlueprintReadWrite, Category = "Base System Stats")
	int32 LeftEng = 16;
	int32 LeftEngCurr = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Base System Stats")
	int32 RightEng = 16;
	int32 RightEngCurr = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Base System Stats")
	int32 CenterEng = 0;
	int32 CenterEngCurr = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Base System Stats")
	int32 PowerReactor = 4;
	int32 PowerReactorCurr = 0;

	//TotalEnergy: maximum possible energy based on above stats
	//TotalEnergyCurr: current maximum possible energy based on above stats/damage
	//TotalEnergyAvailable: current energy not allocated
	int32 TotalEnergy;
	int32 TotalEnergyCurr;
	int32 TotalEnergyAvailable;

	//ARRAYS FOR STORING SYSTEMS
	

	//Engine settings
	UPROPERTY(BlueprintReadWrite, Category = "Base Mobility Stats")
	float SpeedLimit = 1750.f;
	UPROPERTY(BlueprintReadWrite, Category = "Base Mobility Stats")
	float CurrentThrottle = 0.f;

	//Maneuverability
	UPROPERTY(BlueprintReadWrite, Category = "Base Mobility Stats")
	float PitchRate = 20.f;
	UPROPERTY(BlueprintReadWrite, Category = "Base Mobility Stats")
	float YawRate = 20.f;
	//Acceleration rate, how fast velocity increases up to current maximum
	UPROPERTY(BlueprintReadWrite, Category = "Base Mobility Stats")
	float AccelRate = 20.f;
	//Top speed, multiplied by amount of energy allocated to movement to get the maximum velocity (magnitude)
	UPROPERTY(BlueprintReadWrite, Category = "Base Mobility Stats")
	float SpeedConstant = 20.f;
	
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Functions for allocating energy to specific functions
	UFUNCTION(BlueprintCallable)
	void AllocateReinforceShield(int32 amt, int32 index);

	UFUNCTION(BlueprintCallable)
	void AllocateMovement(int32 amt);

	UFUNCTION(BlueprintCallable)
	void AllocateModularSystem(AModularSystem* TargetSystem, int32 amt);

	UFUNCTION(BlueprintCallable)
	void FreeReinforceShield(int32 amt, int32 index);

	UFUNCTION(BlueprintCallable)
	void FreeMovement(int32 amt);

	UFUNCTION(BlueprintCallable)
	void FreeModularSystem(AModularSystem* TargetSystem, int32 amt);
private:

	//Power allocated to base non-external systems
	int32 MovementEnergy = 0;

	//Used for ticking
	FVector CurrentVelocity;
	FRotator AngularVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ShipMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* HullMesh;


};
