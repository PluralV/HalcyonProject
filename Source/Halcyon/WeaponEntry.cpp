// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponEntry.h"
#include "WeaponDetailedInfoWidget.h"
#include "ShipPawn.h"
#include "ShipPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "WeaponSystem.h"

void UWeaponEntry::NativeConstruct() {
	Super::NativeConstruct();

	if (OwningWeapon) {
			if (LblWeaponName) LblWeaponName->SetText(FText::FromString(FString::Printf(TEXT("%s %d"), *(OwningWeapon->WeaponAbbreviatedName.ToString()), (MyIndex + 1))));
			if (LblWeaponArc) LblWeaponArc->SetText(OwningWeapon->WeaponArc);
			if (LblStatus) LblStatus->SetText(FText::FromString("INACTIVE"));
			if (EnergyLevelCurr) EnergyLevelCurr->SetText(FText::FromString(FString::Printf(TEXT("%d"), OwningWeapon->AllocatedEnergy)));
			OwningWeapon->OnEnergyChangedExternal.AddDynamic(this, &UWeaponEntry::OnEnergyChanged);
	}
	if (OwningShip) {
		if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) OSP->OnWeaponDamaged.AddDynamic(this, &UWeaponEntry::RegisterDamage);
	}
}

void UWeaponEntry::OnAllocButtonClicked() {
	//Cycle energy - attempt to immediately allocate to minimum level
	if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) {
		if (OwningWeapon && !bIsDamaged) {
			int32 AmountAlloced = 0;
			if (OwningWeapon->AllocatedEnergy < OwningWeapon->MinEnergy) {//if less than minimum, allocate entirely up to min
				//ALLOCATE THIS AMOUNT OF ENERGY TO WEAPON W/SHIP PAWN
				AmountAlloced = OSP->AllocateWeapon(MyIndex, OwningWeapon->MinEnergy - OwningWeapon->AllocatedEnergy);
				if (!AmountAlloced) {
					OSP->FreeWeapon(MyIndex, OwningWeapon->AllocatedEnergy);
				}
			}
			else if (OwningWeapon->AllocatedEnergy < OwningWeapon->MaxEnergy) {//If between minimum and maximum, step up by 1
				AmountAlloced = OSP->AllocateWeapon(MyIndex, OwningWeapon->EnergyStep);
				if (!AmountAlloced) {//If it failed, just free the energy
					OSP->FreeWeapon(MyIndex, OwningWeapon->AllocatedEnergy);
				}
			}
			else {//If at maximum, simply free all the energy
				OSP->FreeWeapon(MyIndex, OwningWeapon->AllocatedEnergy);
			}
			OnEnergyChanged();
		}
	}
}

//Displays the expanded detail menu for the current weapon
void UWeaponEntry::OnInfoButtonClicked() {
	if (OwningWeapon) {
		if (OwningShip) {
			if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) {
				if (AShipPlayerController* OPC = Cast<AShipPlayerController>(OSP->Controller)) {
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Clicked info button"));
					OPC->SetWeaponDetails(OwningWeapon,MyIndex);
				}
			}
		}
	}
}

//Assigns weapon to control group on click
void UWeaponEntry::OnCtrlGroupButtonClicked() {
	if (OwningWeapon && !bIsDamaged) {
		OwningWeapon->ControlGroup = (OwningWeapon->ControlGroup + 1) % 10;
		if (LblControlGroup) {
			LblControlGroup->SetText(FText::FromString(FString::Printf(TEXT("%d"), OwningWeapon->ControlGroup)));
		}
	}
}

void UWeaponEntry::OnEnergyChanged() {
	if (OwningWeapon && !bIsDamaged) {
		int32 Energy = OwningWeapon->AllocatedEnergy;
		int32 Min = OwningWeapon->MinEnergy;
		int32 Max = OwningWeapon->MaxEnergy;

		//Determine color of UI elements that change w/energy level
		FLinearColor StatusColor = FLinearColor(0.084, 0.896, 0.8);
		if (Energy < Min) {
			StatusColor = FLinearColor(0.25, 0.25, 0.25, 0.9);
		}
		else if (Energy > Min) {
			if (Energy < Max) StatusColor = FLinearColor(0.f, 1.0, 0.5, 0.8);
			else StatusColor = FLinearColor(0.f, 1.0, 0.2, 0.8);
		}


		if (EnergyLevelProgressBar) {
			EnergyLevelProgressBar->SetPercent((float)Energy / (float)Max);
			//Set color of the progress bar to match either GREEN (overloaded) or HALCYON BLUE (standard charge)
			EnergyLevelProgressBar->SetFillColorAndOpacity(StatusColor);
		}
		
		//Update the text counter
		if (EnergyLevelCurr) {
			EnergyLevelCurr->SetText(FText::FromString(FString::Printf(TEXT("%d"), Energy)));
		}

		//Update the charge status widget
		if (LblStatus) {
			if (Energy == Min) {
				LblStatus->SetText(FText::FromString("ACTIVE (STD)"));
			}
			else if (Energy > Min) {
				if (Energy < Max) LblStatus->SetText(FText::FromString("ACTIVE (OL)"));
				else LblStatus->SetText(FText::FromString("ACTIVE (MAX OL)"));
			}
			else {
				LblStatus->SetText(FText::FromString("INACTIVE"));
			}
		}

		//Update cooldown status widget
		if (LblCooldownStatus) {
			if (Energy < Min) {
				LblCooldownStatus->SetVisibility(ESlateVisibility::Hidden);
			}
			else {
				LblCooldownStatus->SetVisibility(ESlateVisibility::Visible);
			}
		}
		//Update cooldown bar (just remove the cooldown bar entirely if depowered)
		if (Cooldown) {
			if (Energy < Min) Cooldown->SetPercent(0.0);
			else Cooldown->SetPercent(OwningWeapon->TimeSinceLastShot / OwningWeapon->FireRate);
		}

		//Update the button color to match status
		if (AllocButton) {
			AdjustButtonBackgroundColor(StatusColor);
		}
	}
	
}

void UWeaponEntry::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (OwningWeapon && !bIsDamaged) {
		if (OwningWeapon->AllocatedEnergy >= OwningWeapon->MinEnergy) {
			if (OwningWeapon->bIsArming) {
				bIsReady = false;
				if (OwningWeapon->TimeSinceLastShot + InDeltaTime <= OwningWeapon->FireRate) {
					Cooldown->SetPercent((OwningWeapon->TimeSinceLastShot + InDeltaTime) / OwningWeapon->FireRate);
					Cooldown->SetFillColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.9));
					LblCooldownStatus->SetText(FText::FromString("ARMING"));
				}	
				else {
					Cooldown->SetPercent(1.0);
					Cooldown->SetFillColorAndOpacity(FLinearColor(0.f,1.f,0.f,0.9));
					LblCooldownStatus->SetText(FText::FromString("ARMED"));
				}
			}
			else {
				if (!bIsReady) {
					bIsReady = true;
					Cooldown->SetPercent(1.0);
					Cooldown->SetFillColorAndOpacity(FLinearColor(0.f, 1.f, 0.f, 0.9));
					LblCooldownStatus->SetText(FText::FromString("ARMED"));
				}
			}
		}
	}
}

void UWeaponEntry::AdjustButtonBackgroundColor(FLinearColor StatusColor) {
	if (AllocButton) {
		// Get the current button style
		FButtonStyle ButtonStyle = AllocButton->GetStyle();

		// Create a new slate brush with the color
		ButtonStyle.Normal.TintColor = FSlateColor(StatusColor);

		ButtonStyle.Hovered.TintColor = FSlateColor(StatusColor * 1.2f); // Slightly brighter on hover

		ButtonStyle.Pressed.TintColor = FSlateColor(StatusColor * 0.8f); // Slightly darker when pressed

		// Apply the style to the button
		AllocButton->SetStyle(ButtonStyle);
	}
}

//if it matches this weapon's index, set the weapon's background color to red and show it as damaged
//damaged buttons do not respond to clicks
void UWeaponEntry::RegisterDamage(int32 Index) {
	if (Index == MyIndex) {
		bIsDamaged = true;
		if (AllocButton) {
			AdjustButtonBackgroundColor(FLinearColor(1.f, 0.f, 0.f));
		}
		if (LblStatus) {
			LblStatus->SetText(FText::FromString("WEAPON DOWN"));
		}
		if (LblCooldownStatus) {
			LblCooldownStatus->SetText(FText::FromString("CRITICAL"));
		}
		if (EnergyLevelCurr) {
			EnergyLevelCurr->SetText(FText::FromString("N/A"));
		}
	}
}