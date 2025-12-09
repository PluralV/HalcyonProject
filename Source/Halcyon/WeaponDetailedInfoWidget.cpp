// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponDetailedInfoWidget.h"
#include "WeaponSystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UWeaponDetailedInfoWidget::NativeConstruct() {
	Super::NativeConstruct();
}

void UWeaponDetailedInfoWidget::OnCloseButtonClicked() {
	OwningWeapon = nullptr;
	OnNewOwningWeapon();
	SetVisibility(ESlateVisibility::Hidden);
}

void UWeaponDetailedInfoWidget::OnNewOwningWeapon() {
	if (OwningWeapon) {
		if (LblWeaponName) {
			LblWeaponName->SetText(FText::FromString(FString::Printf(TEXT("%s %d"), *(OwningWeapon->WeaponAbbreviatedName.ToString()), (MyIndex + 1))));
		}
		if (LblWeaponType) {
			LblWeaponType->SetText(FText::FromString(FString::Printf(TEXT("%s"), *(OwningWeapon->WeaponName.ToString()))));
		}
		if (LblWeaponDesc) {
			LblWeaponDesc->SetText(FText::FromString(FString::Printf(TEXT("%s"), *(OwningWeapon->WeaponDesc.ToString()))));
		}
		if (LblWeaponFacing) {
			LblWeaponFacing->SetText(FText::FromString(FString::Printf(TEXT("%s"), *(OwningWeapon->WeaponArc.ToString()))));
		}
		if (LblWeaponArcs) {
			LblWeaponArcs->SetText(FText::FromString(FString::Printf(TEXT("Turret Arc: %ddeg; Elevation: %ddeg"), (int)OwningWeapon->MaxTurretArc, (int)OwningWeapon->MaxPitchArc)));
		}
		if (LblWeaponFireRate) {
			LblWeaponFireRate->SetText(FText::FromString(FString::Printf(TEXT("Cooldown time: %.1fs"), roundf(OwningWeapon->FireRate * 10) / 10.f )));
		}
		if (LblEnergyCost && LblDamageScale) {
			if (OwningWeapon->MinEnergy != OwningWeapon->MaxEnergy) {
				LblEnergyCost->SetText(FText::FromString(FString::Printf(TEXT("%d energy to activate, %d energy max"), OwningWeapon->MinEnergy, OwningWeapon->MaxEnergy)));
				LblDamageScale->SetText(FText::FromString(FString::Printf(TEXT("+%d%% damage per %d additional energy"), (int)(OwningWeapon->OverloadScaling * 100.f),OwningWeapon->EnergyStep)));
				LblDamageScale->SetVisibility(ESlateVisibility::Visible);
			}
			else {
				LblEnergyCost->SetText(FText::FromString(FString::Printf(TEXT("%d energy to activate"), OwningWeapon->MinEnergy)));
				LblDamageScale->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		if (LblWeaponDamageAndRange) {
			if (!OwningWeapon->DamageScaling) {
				LblWeaponDamageAndRange->SetText(FText::FromString(FString::Printf(TEXT("Base Damage: %d | Range: %dm"), OwningWeapon->BaseDamage, (int)OwningWeapon->MaxRange)));
			}
			else {
				LblWeaponDamageAndRange->SetText(FText::FromString(FString::Printf(TEXT("Base Damage: %d | Range: %dm"), 
					OwningWeapon->BaseDamage,
					(int)OwningWeapon->MaxRange, 
					(int32)((1.f/(float)OwningWeapon->DamageScaling)*100.f),
					(int32)(OwningWeapon->MaxRange/3.f)
				)));
			}
			
		}

		if (LblWeaponRangeScale) {
			if (!OwningWeapon->DamageScaling) {
				LblWeaponRangeScale->SetText(FText::FromString(FString::Printf(TEXT(""))));
				LblWeaponRangeScale->SetVisibility(ESlateVisibility::Collapsed);
			}
			else {
				LblWeaponRangeScale->SetText(FText::FromString(FString::Printf(TEXT("-%d%% damage per %dm range"),
					(int32)((1.f / (float)OwningWeapon->DamageScaling) * 100.f),
					(int32)(OwningWeapon->MaxRange / 3.f)
				)));
				LblWeaponRangeScale->SetVisibility(ESlateVisibility::Visible);
			}

		}

	}
}