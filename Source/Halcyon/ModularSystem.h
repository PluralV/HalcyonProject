// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModularSystem.generated.h"

UCLASS(Blueprintable, BlueprintType)
class HALCYON_API AModularSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AModularSystem();

	void AllocateEnergy(int32 amt);

	void FreeEnergy(int32 amt);

	void FreeEnergy();

	bool IsPowered();

	int32 GetPower();

	FName SystemName;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Static Mesh")
	UStaticMeshComponent* SystemMesh;

private:
	int32 EnergyAllocated = 0;
	bool Powered = false;
	bool Damageable = true;
};
