// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponEntry.h"
#include "ShipPawn.h"
#include "WeaponSystem.h"

void UWeaponEntry::NativeConstruct() {
	Super::NativeConstruct();
}

void UWeaponEntry::OnAllocButtonClicked() {
	if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) {
		if (AWeaponSystem* OWS = Cast<AWeaponSystem>(OwningWeapon)) {
			if (OWS->AllocatedEnergy < OWS->MinEnergy) {
				int32 AmountTry = OWS->MinEnergy - OWS->AllocatedEnergy;
				//ALLOCATE THIS AMOUNT OF ENERGY TO WEAPON W/SHIP PAWN
			}
		}
	}
	
}

void UWeaponEntry::OnFreeButtonClicked() {

}