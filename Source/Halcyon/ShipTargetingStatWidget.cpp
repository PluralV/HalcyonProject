// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipTargetingStatWidget.h"
#include "ShipPawn.h"
#include "Components/Button.h"
#include "Components/Border.h"

void UShipTargetingStatWidget::NativeConstruct() {
	Super::NativeConstruct();
	

}

void UShipTargetingStatWidget::NativeTick(const FGeometry &MyGeometry, float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UShipTargetingStatWidget::HandleActivateTargeter() {

}

void UShipTargetingStatWidget::HandleOnShieldStrengthChanged(int32 Index, int32 Amt) {
	
}

void UShipTargetingStatWidget::HandleOnHullIntegrityChanged(int32 Amt) {

}

void UShipTargetingStatWidget::HandleControlShiftButton(int32 NewCtrl) {
	if (OwningShipPawn) {
		OwningShipPawn->SetCurrentControlGroup(NewCtrl);
		UpdateTargetShip();
	}
}

void UShipTargetingStatWidget::UpdateTargetShip() {
	if (OwningShipPawn) {
		AActor* TargetActor = OwningShipPawn->GetCurrentTarget(OwningShipPawn->GetCurrentControlGroup());
		if (AShipPawn* TargetShip = Cast<AShipPawn>(TargetActor)) {
			TargetShipPawn = TargetShip;
			InitializeTargeterToShip();
		}
		else {
			TargetShipPawn = nullptr;
			ResetTargeterToNeutral();
		}
	}
}

void UShipTargetingStatWidget::InitializeTargeterToShip() {
	if (TargetShipPawn) {
		for (int32 i = 0; i < 6; i++) {
			//Set progress healthbar to relevant amount
			float Percent = ModifyProgressBar(TargetShields[i],
				(float)(TargetShipPawn->GetCurrentShieldFacing(i)
					+ TargetShipPawn->GetCurrentShieldReinforcement(i)),TargetShipPawn->GetShieldFacing(i));
			//Set color of bar depending on percent
			RecolorProgressBar(TargetShields[i], GetStatusColorForBar(Percent));
			//Set text box to shield health
			ModifyTextBox(TargetShieldsText[i],
				(TargetShipPawn->GetCurrentShieldFacing(i)
					+ TargetShipPawn->GetCurrentShieldReinforcement(i)));
		}
		TargetDiagramBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	
}

void UShipTargetingStatWidget::ResetTargeterToNeutral() {
	TargetDiagramBorder->SetVisibility(ESlateVisibility::Collapsed);
	ModifyTextBox(TargetOrNoTarget, FText::FromString("No Target"));
	ModifyTextBox(CurrentTargetClass, FText::FromString("N/A"));
	ModifyTextBox(TargetRange, FText::FromString("0"));
	ModifyTextBox(TargetSpeed, FText::FromString("0"));

}