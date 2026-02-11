// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponEntry.h"
#include "WeaponDetailedInfoWidget.h"
#include "ShipPawn.h"
#include "ShipPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "WeaponSystem.h"
#include "Kismet/GameplayStatics.h"

void UWeaponEntry::NativeConstruct() {
	Super::NativeConstruct();

	if (OwningWeapon) {
			if (LblWeaponName) LblWeaponName->SetText(FText::FromString(FString::Printf(TEXT("%s %d"), *(OwningWeapon->WeaponAbbreviatedName.ToString()), (MyIndex + 1))));
			if (LblWeaponArc) LblWeaponArc->SetText(OwningWeapon->WeaponArc);
			if (LblStatus) LblStatus->SetText(FText::FromString("INACTIVE"));
			if (EnergyLevelCurr) EnergyLevelCurr->SetText(FormatFloatTenths(OwningWeapon->AllocatedEnergy));
			OwningWeapon->OnEnergyChangedExternal.AddDynamic(this, &UWeaponEntry::OnEnergyChanged);
			TSoftObjectPtr<USoundBase> UpSoundRef;
			UpSoundRef = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/HalcyonBlueprints/Ships/Weapons/Audio/UISounds/SystemPowerUp.SystemPowerUp")));
			AllocSoundEffect = UpSoundRef.LoadSynchronous();

			TSoftObjectPtr<USoundBase> DownSoundRef;
			DownSoundRef = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/HalcyonBlueprints/Ships/Weapons/Audio/UISounds/SystemPowerDown.SystemPowerDown")));
			FreeSoundEffect = DownSoundRef.LoadSynchronous();

			TSoftObjectPtr<USoundBase> DeniedSoundRef;
			DeniedSoundRef = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/HalcyonBlueprints/Ships/Weapons/Audio/UISounds/Camera_Shutter1.Camera_Shutter1")));
			DenySoundEffect = DeniedSoundRef.LoadSynchronous();
	}
	if (OwningShip) {
		if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) OSP->OnWeaponDamaged.AddDynamic(this, &UWeaponEntry::RegisterDamage);
	}
}

void UWeaponEntry::OnAllocButtonClicked() {
	//Cycle energy - attempt to immediately allocate to minimum level
	if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) {
		if (OwningWeapon && !bIsDamaged && !OwningWeapon->bIsFiring) {
			float AmountAlloced = 0.f;
			if (OwningWeapon->AllocatedEnergy < OwningWeapon->MinEnergy) {//if less than minimum, allocate entirely up to min
				//ALLOCATE THIS AMOUNT OF ENERGY TO WEAPON W/SHIP PAWN
				AmountAlloced = OSP->AllocateWeapon(MyIndex, OwningWeapon->MinEnergy - OwningWeapon->AllocatedEnergy);
				if (AmountAlloced == 0.f) {
					float AmountFreed = OSP->FreeWeapon(MyIndex, OwningWeapon->AllocatedEnergy);
					if (AmountFreed && FreeSoundEffect) {
						UGameplayStatics::PlaySound2D(GetWorld(), FreeSoundEffect);
					}
					else {
						if (DenySoundEffect) {
							UGameplayStatics::PlaySound2D(GetWorld(), DenySoundEffect);
						}
					}
				}
				else {
					if (AllocSoundEffect) {
						UGameplayStatics::PlaySound2D(GetWorld(), AllocSoundEffect);
					}
				}
			}
			else if (OwningWeapon->AllocatedEnergy < OwningWeapon->MaxEnergy) {//If between minimum and maximum, step up by 1
				AmountAlloced = OSP->AllocateWeapon(MyIndex, OwningWeapon->EnergyStep);
				if (AmountAlloced == 0.f) {//If it failed, just free the energy
					float AmountFreed = OSP->FreeWeapon(MyIndex, OwningWeapon->AllocatedEnergy);
					//Determine what sound to play
					//If AmountFreed > 0, play the Freed sound
					if (AmountFreed > 0.f && FreeSoundEffect) {
						UGameplayStatics::PlaySound2D(GetWorld(), FreeSoundEffect);
					}//otherwise play the Denied sound
					else {
						if (DenySoundEffect) {
							UGameplayStatics::PlaySound2D(GetWorld(), DenySoundEffect);
						}
					}
				}
				else {//if AmountAlloced then play the Alloc sound
					if (AllocSoundEffect) {
						UGameplayStatics::PlaySound2D(GetWorld(), AllocSoundEffect, 
							1.0f, 
							1.0 + 
							(((float)(OwningWeapon->AllocatedEnergy - OwningWeapon->MinEnergy) 
								/ (float)((OwningWeapon->EnergyStep) ? OwningWeapon->EnergyStep : 1))) * 0.3f);
					}
				}
			}
			else {//If at maximum, simply free all the energy
				float AmountFreed = OSP->FreeWeapon(MyIndex, OwningWeapon->AllocatedEnergy);
				if (AmountFreed > 0.f) {
					if (FreeSoundEffect) {
						UGameplayStatics::PlaySound2D(GetWorld(), FreeSoundEffect);
					}
				}
				else if (DenySoundEffect) {
					UGameplayStatics::PlaySound2D(GetWorld(), DenySoundEffect);
				}
			}
			OnEnergyChanged();
		}
		else {
			if (DenySoundEffect)
				UGameplayStatics::PlaySound2D(GetWorld(), DenySoundEffect);
		}
	}
}

//Displays the expanded detail menu for the current weapon
void UWeaponEntry::OnInfoButtonClicked() {
	if (OwningWeapon) {
		if (OwningShip) {
			if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) {
				if (AShipPlayerController* OPC = Cast<AShipPlayerController>(OSP->Controller)) {
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Clicked info button"));
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
		float Energy = OwningWeapon->AllocatedEnergy;
		float Min = OwningWeapon->MinEnergy;
		float Max = OwningWeapon->MaxEnergy;

		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Energy, min, max %f %f %f"), Energy, Min, Max));

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
			EnergyLevelProgressBar->SetPercent(Energy / Max);
			//Set color of the progress bar to match either GREEN (overloaded) or HALCYON BLUE (standard charge)
			EnergyLevelProgressBar->SetFillColorAndOpacity(StatusColor);
		}
		
		//Update the text counter
		if (EnergyLevelCurr) {
			EnergyLevelCurr->SetText(FormatFloatTenths(Energy));
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
		AdjustButtonBackgroundColor(StatusColor);
	}
	
}

void UWeaponEntry::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);
	//Determine color/look of entry main panel and cooldown bars of various kinds
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
			if (OwningWeapon->bIsFiring) {
				if (!bIsFiringSet) {
					bIsFiringSet = true;
					DarkenButtonBackgroundColor();
				}
				LblCooldownStatus->SetText(FText::FromString("FIRING"));
				
			}
			else {
				if (bIsFiringSet) {
					bIsFiringSet = false;
					LightenButtonBackgroundColor();
				}
			}
		}
	}
}

void UWeaponEntry::DarkenButtonBackgroundColor() {
	if (AllocButton) {
		// Get current button style
		FButtonStyle ButtonStyle = AllocButton->GetStyle();

		ButtonStyle.Normal.TintColor = FSlateColor(ButtonStyle.Normal.TintColor.GetSpecifiedColor() * 0.75f);
		ButtonStyle.Hovered.TintColor = FSlateColor(ButtonStyle.Hovered.TintColor.GetSpecifiedColor() * 0.75f);
		ButtonStyle.Pressed.TintColor = FSlateColor(ButtonStyle.Pressed.TintColor.GetSpecifiedColor() * 0.75f);

		AllocButton->SetStyle(ButtonStyle);
	}
}

void UWeaponEntry::LightenButtonBackgroundColor() {
	if (AllocButton) {
		// Get current button style
		FButtonStyle ButtonStyle = AllocButton->GetStyle();

		ButtonStyle.Normal.TintColor = FSlateColor(ButtonStyle.Normal.TintColor.GetSpecifiedColor() / 0.75f);
		ButtonStyle.Hovered.TintColor = FSlateColor(ButtonStyle.Hovered.TintColor.GetSpecifiedColor() / 0.75f);
		ButtonStyle.Pressed.TintColor = FSlateColor(ButtonStyle.Pressed.TintColor.GetSpecifiedColor() / 0.75f);

		AllocButton->SetStyle(ButtonStyle);
	}
}

void UWeaponEntry::AdjustButtonBackgroundColor(FLinearColor StatusColor) {
	if (AllocButton) {
		// Get the current button style
		FButtonStyle ButtonStyle = AllocButton->GetStyle();

		//Change the normal, hovered, pressed colors to statuscolor (brighter on hover, darker on pressed)
		ButtonStyle.Normal.TintColor = FSlateColor(StatusColor);

		ButtonStyle.Hovered.TintColor = FSlateColor(StatusColor * 1.2f);

		ButtonStyle.Pressed.TintColor = FSlateColor(StatusColor * 0.8f);

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

FText UWeaponEntry::FormatFloatTenths(float InFloat) {
	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = 0;
	Options.MaximumFractionalDigits = 1;

	return FText::AsNumber(InFloat, &Options);
}