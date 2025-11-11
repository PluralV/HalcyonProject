// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPlayerController.h"
#include "ModularSystem.h"
#include "ShipPawn.h"


void AShipPlayerController::BeginPlay() {
	Super::BeginPlay();
}

//MAY NEED WORK: WILL PLAYER CONTROLLER KNOW ABOUT THIS??
void AShipPlayerController::AllocateEnergyToModularSys(AModularSystem* TargetSystem, int32 amt) {
	TargetSystem->AllocateEnergy(amt);
}

void AShipPlayerController::FreeEnergyFromModularSys(AModularSystem* TargetSystem, int32 amt) {
	TargetSystem->FreeEnergy(amt);
}

void AShipPlayerController::AllocateEnergyToStaticSys(EStaticSystemType type, int32 amt) {
	APawn* CurrentPawn = GetPawn();
	if (AShipPawn* CurrentShipPawn = Cast<AShipPawn>(CurrentPawn)) {
		int32 index = -1;
		switch (type) {
		case EStaticSystemType::Movement:
			CurrentShipPawn->AllocateMovement(amt);
			return;
		case EStaticSystemType::Shield1:
			index = 0;
			break;
		case EStaticSystemType::Shield2:
			index = 1;
			break;
		case EStaticSystemType::Shield3:
			index = 2;
			break;
		case EStaticSystemType::Shield4:
			index = 3;
			break;
		case EStaticSystemType::Shield5:
			index = 4;
			break;
		case EStaticSystemType::Shield6:
			index = 5;
			break;
			//More logic in future if additional static systems are added
		default:break;
		}
		//Reinforce a shield
		CurrentShipPawn->AllocateReinforceShield(amt, index);
	}
	
}

void AShipPlayerController::FreeEnergyFromStaticSys(EStaticSystemType type, int32 amt) {
	APawn* CurrentPawn = GetPawn();
	if (AShipPawn* CurrentShipPawn = Cast<AShipPawn>(CurrentPawn)) {
		int32 index = -1;
		switch (type) {
		case EStaticSystemType::Movement:
			CurrentShipPawn->FreeMovement(amt);
			return;
		case EStaticSystemType::Shield1:
			index = 0;
			break;
		case EStaticSystemType::Shield2:
			index = 1;
			break;
		case EStaticSystemType::Shield3:
			index = 2;
			break;
		case EStaticSystemType::Shield4:
			index = 3;
			break;
		case EStaticSystemType::Shield5:
			index = 4;
			break;
		case EStaticSystemType::Shield6:
			index = 5;
			break;
			//More logic in future if additional static systems are added
		default:break;
		}
		//Reinforce a shield
		CurrentShipPawn->FreeReinforceShield(amt, index);
	}
}