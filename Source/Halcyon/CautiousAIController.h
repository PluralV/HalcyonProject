// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTacticsAIController.h"
#include "CautiousAIController.generated.h"

/**
 * Cautious AI player - best for controlling ships with long-ranged weapons on the front
 */
UCLASS()
class HALCYON_API ACautiousAIController : public ABaseTacticsAIController
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;
	//Relative to that ship and current state, determine where to move next
	virtual FVector AcquireLookAtPoint() override;
	//Determine what to do with weapons - hold fire, shoot, or potentially free/allocate further energy
	virtual void EngageTarget() override;
	//Allocate energy to systems - typically once per cycle, or sometimes immediately following an event
	virtual void AllocateEnergy() override;

};
