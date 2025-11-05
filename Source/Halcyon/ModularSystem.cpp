// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularSystem.h"

// Sets default values
AModularSystem::AModularSystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AModularSystem::BeginPlay()
{
	Super::BeginPlay();
}

/*
Energy allocation: Powers the system to determine if it works or not. Has overloaded version for
systems which can receive variable energy.
*/
void AModularSystem::AllocateEnergy(int32 amt) {
	EnergyAllocated = amt;
	Powered = true;
}

void AModularSystem::FreeEnergy() {
	EnergyAllocated = 0;
	Powered = false;
}

void AModularSystem::FreeEnergy(int32 amt) {
	EnergyAllocated = amt >= EnergyAllocated ? 0 : EnergyAllocated - amt;
	Powered = EnergyAllocated > 0;
}

int32 AModularSystem::GetPower() {
	return EnergyAllocated;
}

bool AModularSystem::IsPowered() {
	return Powered;
}

// Called every frame
void AModularSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

