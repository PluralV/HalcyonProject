// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponListWidget.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "WeaponEntry.h"

void UWeaponListWidget::NativeConstruct() {
	Super::NativeConstruct();
	
	//Check if the owning pawn is valid
	if (OwningShip) {
		//Attempt to cast the owning pawn to a ShipPawn
		if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) {
			//Check if the Weapon Entry widget class is a valid widget class
			if (WeaponEntryClass && ContainerPanel) {
				//Get all the WeaponComponents of the owning ship from its array
				TArray<UChildActorComponent*> WeaponsToEnter = OSP->GetWeaponComponents();
				for (int i = 0; i < WeaponsToEnter.Num(); i++) {
					//Check if the weapon system is a valid weapon system
					if (AWeaponSystem* WS = Cast<AWeaponSystem>(WeaponsToEnter[i]->GetChildActor())) {
						//If valid, create a WeaponEntryWidget and push it to the list of entry widgets
						UUserWidget* Entry = CreateWidget<UUserWidget>(this, WeaponEntryClass);
						//Set WeaponEntry properties: add the widget, set its owning ship and weapon, store its index for easy reference
						//back to the original weapon
						if (UWeaponEntry* CWE = Cast<UWeaponEntry>(Entry)) {
							WeaponEntryList.Add(CWE);
							//Set owning ship and weapon in the widget to the current weapon and this widget's owning ship
							CWE->OwningShip = OSP;
							CWE->OwningWeapon = WS;
							CWE->MyIndex = i;
							CWE->SetDesiredSizeInViewport(FVector2D(200.f, 200.f));
							//then add to the viewport
							CWE->SetIsFocusable(false);
						}
					}
				}
			}
		}
	}

	//Add all the child widgets to the ContainerPanel child (scroll/vertical box)
	if (ContainerPanel) {
		
		for (int i = 0; i < WeaponEntryList.Num(); i++) {
			// Create a Size Box to control dimensions
			USizeBox* SizeBox = NewObject<USizeBox>(this);
			SizeBox->SetHeightOverride(160.0f);  // Overridden height of each cell
			SizeBox->SetWidthOverride(220.0f);
			//Add the child to the SizeBox
			SizeBox->AddChild(WeaponEntryList[i]);

			if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(ContainerPanel->AddChild(SizeBox))) {
				// Configure the slot
				//ScrollSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 0.f));
				ScrollSlot->SetHorizontalAlignment(HAlign_Left);
				ScrollSlot->SetVerticalAlignment(VAlign_Fill);
				ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
				// Spacing between items
				ScrollSlot->SetPadding(FMargin(5.0f, 0.0f));
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

