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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDestroyed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaticEnergyChanged, int32, which, int32, amt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldStrengthChanged, int32, index, int32, amt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvailableEnergyChanged, int32, amt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementEnergyChanged, int32, amt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTotalEnergyChanged, int32, amt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHullIntegrityChanged, int32, amt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShipDestroyed, int32, CauseOfDeath, AShipPawn*, DestroyedShip);




UCLASS(Blueprintable, BlueprintType)
class HALCYON_API AShipPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AShipPawn();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UShipPawnMovementComponent* MovementComponent;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStaticEnergyChanged OnStaticEnergyChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAvailableEnergyChanged OnAvailableEnergyChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTotalEnergyChanged OnTotalEnergyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMovementEnergyChanged OnMovementEnergyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShieldStrengthChanged OnShieldStrengthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHullIntegrityChanged OnHullIntegrityChanged;


	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShipDestroyed OnShipDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyDestroyed OnEnemyDestroyed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Name")
	FText ShipName;

	UFUNCTION()
	void SetTarget(AActor* Target) {
		CurrentTarget = Target;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AllocMovementAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FreeMovementAction;

	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	AActor* CurrentTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<UChildActorComponent*> WeaponComponents;

	//Handle player input
	void Look(const FInputActionValue& Value);
	void Throttle(const FInputActionValue& Value);
	void Decelerate(const FInputActionValue& Value);
	void Steer(const FInputActionValue& Value);
	void ZeroThrottle();
	void ZeroDecel();
	void ZeroSteering();
	void Target(const FInputActionValue& Value);
	void Fire();
	void HandleArrowAlloc();
	void HandleArrowFree();

	// Camera rotation speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraRotationSpeed = 1.f;

	//Ship base stats
	//Hull integrity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Base System Stats")
	int32 HullIntegrity = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base System Stats")
	int32 MaxHullIntegrity = 32;

	//Shield values
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Base System Stats")
	TArray<int32> ShieldFacings = { 30,24,18,18,18,24 };
	//ShieldReinforcements tracks any extra power put into a given shield and should be reduced FIRST by any external damage
	UPROPERTY(BlueprintReadOnly)
	TArray<int32> ShieldReinforcements = { 0,0,0,0,0,0 };
	//Stores current strength of each shield
	UPROPERTY(BlueprintReadOnly)
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

	UFUNCTION(BlueprintCallable)
	void AllocateDamage(float FromAngle, int32 DamageAmt);

	//GETTERS
	//Movement energy
	UFUNCTION(BlueprintCallable)
	int32 GetMovementEnergy();

	UFUNCTION(BlueprintCallable)
	int32 GetMaxEnergyCurr();

	UFUNCTION(BlueprintCallable)
	int32 GetMaxEnergy();

	UFUNCTION(BlueprintCallable)
	int32 GetMaxEnergyAvailable();

	UFUNCTION(BlueprintCallable)
	float GetSpeedConstant();

	//Shield getters
	UFUNCTION(BlueprintCallable)
	int32 GetShieldFacing(int32 index);

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentShieldFacing(int32 index);

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentShieldReinforcement(int32 index);

	//Static system numbers
	UFUNCTION(BlueprintCallable)
	int32 GetLeftEngMax();

	UFUNCTION(BlueprintCallable)
	int32 GetLeftEngCurr();

	UFUNCTION(BlueprintCallable)
	int32 GetRightEngMax();

	UFUNCTION(BlueprintCallable)
	int32 GetRightEngCurr();

	UFUNCTION(BlueprintCallable)
	int32 GetCentEngMax();

	UFUNCTION(BlueprintCallable)
	int32 GetCentEngCurr();

	UFUNCTION(BlueprintCallable)
	int32 GetReactorMax();

	UFUNCTION(BlueprintCallable)
	int32 GetReactorCurr();

	UFUNCTION(BlueprintCallable)
	int32 GetForwardHullMax();

	UFUNCTION(BlueprintCallable)
	int32 GetForwardHullCurr();

	UFUNCTION(BlueprintCallable)
	int32 GetAftHullMax();

	UFUNCTION(BlueprintCallable)
	int32 GetAftHullCurr();

	UFUNCTION(BlueprintCallable)
	int32 GetHullIntegrity();

	UFUNCTION(BlueprintCallable)
	int32 GetMaxHullIntegrity();

	//Velocity: Rounds in case it's for display, otherwise does not
	UFUNCTION(BlueprintCallable)
	float GetCurrentVelocity(bool bForDisplay);
	

private:
	UFUNCTION()
	void HandleShipDestroyed(int32 CauseOfDeath, AShipPawn* DestroyedShip);

	void DestroyShip(int32 CauseOfDeath);
	void UnlockTarget();
	
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
