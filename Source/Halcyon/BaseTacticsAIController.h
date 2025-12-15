// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseTacticsAIController.generated.h"

UENUM()
enum class ECombatStatus {
	ArmedClose,
	ArmedFar,
	Arming
};

UENUM()
enum class ETargetStatus {
	Healthy,
	Unshielded,
	Crippled
};

USTRUCT()
struct FTargetScore {
	GENERATED_BODY()

	UPROPERTY()
	AActor* Target;

	UPROPERTY()
	int32 AppraisalScore;
};


class AShipPawn;
class AWeaponSystem;

//Stores a weapon, along with its expected damage at current range and the fraction of maximum damage this represents
USTRUCT()
struct FWeaponCapability {
	GENERATED_BODY()

	UPROPERTY()
	AWeaponSystem* Weapon;
	UPROPERTY()
	int32 Index;
	UPROPERTY()
	int32 ExpectedDamage;

	UPROPERTY()
	float DamageEfficiency;

	UPROPERTY()
	bool bCouldOverload;

	UPROPERTY()
	bool bInRange;

	UPROPERTY()
	bool bInArc;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class HALCYON_API ABaseTacticsAIController : public AAIController
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY()
	AActor* CurrentTarget;

	UPROPERTY()
	class AShipPawn* ControlledShip;

	

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	void RotateToward(const FVector& TargetLocation);
	//Choose which ship to target
	virtual void AcquireEligibleTargets();
	virtual void ReappraiseTargets();
	virtual void AcquireBestTarget();
	virtual void HandleTargetDestroyed(int32 CauseOfDeath, AShipPawn* DestroyedShip);
	virtual void HandleNewSpawn(AActor* SpawnedActor);
	//Assign a score to a target based on a number of factors
	virtual int32 AppraiseTarget(AActor* PossibleTarget);
	//Relative to that ship and current state, determine where to move next
	virtual FVector AcquireLookAtPoint();
	//Determine what to do with weapons - hold fire, shoot, or potentially free/allocate further energy
	virtual void EngageTarget();
	//Returns a pair of bools: [0]: weapon is in range, [1]: weapon is in arc of current target
	UFUNCTION()
	void CheckIsBearing(int32 WeaponIndex);

	//Allocate energy to systems - typically once per cycle, or sometimes immediately following an event
	virtual void AllocateEnergy();
	virtual void FreeEnergyByPriority(int32 EnergyGoal);
	virtual void InitializeAfterLoad();
	//VARIABLES
	//List of weapons with ancillary info modified as needed
	UPROPERTY()
	TArray<FWeaponCapability> ShipWeapons;

	//List of targetable ships
	UPROPERTY()
	TArray<FTargetScore> EligibleTargets;


	//Rotator pointing to the current target
	FRotator RotToTarget;
	
	// offset to aim at to create orbiting effect
	UPROPERTY(EditAnywhere)
	float SideOffset = 7500.0f;

	//Used to determine how far above/below to look relative to player target
	UPROPERTY(EditAnywhere)
	float HeightOffset = 0.f;

	//range to set target
	UPROPERTY(EditAnywhere)
	float AcquireRange = 12000.0f;

	//Current range to current target
	float RangeToTarget;
	//range at which this ship's longest-range weapons can be used
	UPROPERTY()
	float MaxRange = 7500.0f;

	//range at which this ship assumes its target can damage it significantly; calculated when a target is acquired
	//can be overridden
	UPROPERTY()
	float DangerZone;

	//time in seconds before each reallocation, barring exceptional circumstances
	UPROPERTY(EditAnywhere)
	float EnergyCycle = 10.f;

	//time in seconds since last allocation
	float TimeSinceLastRealloc = 0.f;

	//time in seconds before each target reappraisal, barring exceptional circumstances
	UPROPERTY(EditAnywhere)
	float TargetCycle = 6.f;

	float TimeSinceLastTarget = 0.f;

	//State of the ship - whether ready to shoot or not, basically
	ECombatStatus CurrentWeaponStatus;

	//State of the target - whether it is vulnerable or not, basically
	ETargetStatus CurrentTargetStatus;

	//the number of weapons this ship has
	int32 WeaponCount = 0;
	//The current shield facing to the target
	int32 ShieldFacingIndex = 0;

	bool bIsTargeting = false;
	
};
