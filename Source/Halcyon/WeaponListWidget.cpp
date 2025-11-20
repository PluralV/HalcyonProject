// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponListWidget.h"
#include "WeaponEntry.h"

void UWeaponListWidget::NativeConstruct() {
	Super::NativeConstruct();

	//Check if the owning pawn is valid
	if (OwningShip) {
		//Attempt to cast the owning pawn to a ShipPawn
		if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) {
			//Check if the Weapon Entry widget class is a valid widget class
			if (WeaponEntryClass) {
				//Get all the WeaponComponents of the owning ship from its array
				TArray<UChildActorComponent*> WeaponsToEnter = OSP->GetWeaponComponents();
				for (int i = 0; i < WeaponsToEnter.Num(); i++) {
					//Check if the weapon system is a valid weapon system
					if (AWeaponSystem* WS = Cast<AWeaponSystem>(WeaponsToEnter[i]->GetChildActor())) {
						//If valid, create a WeaponEntryWidget and push it to the list of entry widgets
						UUserWidget* Entry = CreateWidget<UUserWidget>(this, WeaponEntryClass);
						//TODO set some properties of the weapon entry........
						if (UWeaponEntry* CWE = Cast<UWeaponEntry>(Entry)) {
							WeaponEntryList.Add(CWE);
							//Set owning ship and weapon in the widget to the current weapon and this widget's owning ship
							CWE->OwningShip = OSP;
							CWE->OwningWeapon = WS;
							//then add to the viewport
							CWE->SetIsFocusable(false);
							CWE->AddToViewport();
						}
					}
				}
			}
		}
	}
}


//ignore unnecessary textual input
FReply UWeaponListWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::SpaceBar ||
		InKeyEvent.GetKey() == EKeys::Escape)
	{
		return FReply::Unhandled(); // Pass to controller
	}
	// Handle other keys normally
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

