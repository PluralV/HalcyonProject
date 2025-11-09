// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPlayerController.h"
#include "ModularSystem.h"


void AShipPlayerController::BeginPlay() {
	Super::BeginPlay();
}

void AShipPlayerController::AllocateEnergyToModularSys(AModularSystem* TargetSystem, int32 amt) {

}

void AShipPlayerController::FreeEnergyFromModularSys(AModularSystem* TargetSystem, int32 amt) {

}

void AShipPlayerController::AllocateEnergyToStaticSys(EStaticSystemType type, int32 amt) {

}

void AShipPlayerController::FreeEnergyFromStaticSys(EStaticSystemType type, int32 amt) {

}