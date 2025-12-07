// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShipPawn.h"
#include "ShipPawnMovementComponent.h"
#include "ShipAIController.generated.h"

/**
 * 
 */
UCLASS()
class HALCYON_API AShipAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY()
	AShipPawn* PlayerPawn;

	UPROPERTY()
	class AShipPawn* ControlledShip;

	// offset to aim at to create orbiting effect
	UPROPERTY(EditAnywhere)
	float SideOffset = 7500.0f;

	UPROPERTY(EditAnywhere)
	float HeightOffset = 0.f;

	// range to set target
	UPROPERTY(EditAnywhere)
	float FireRange = 7500.0f;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	void RotateToward(const FVector& TargetLocation);
	bool bIsTargeting = false;
private:
	void AllocateEnergy();
};
