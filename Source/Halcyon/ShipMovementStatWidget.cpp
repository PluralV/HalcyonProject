// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipMovementStatWidget.h"
#include "ShipPawn.h"
#include "Components/Button.h"

void UShipMovementStatWidget::NativeConstruct() {
	Super::NativeConstruct();

	if (OwningShip) {
		if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) {
			//initialize base numbers
			OwningShipPawn = OSP;
			ShipMovementStep = OwningShipPawn->GetMovementStep();
			ShipSpeedConstant = OwningShipPawn->GetSpeedConstant();
			MaxMovementEnergy = OwningShipPawn->GetMaxEngine();
			CurrentMovementEnergy = OwningShipPawn->GetMovementEnergy();
			
			//Initialize engine/movement energy
			OnMovementEnergyUpdate(CurrentMovementEnergy);
			OnMaxEngineUpdate(MaxMovementEnergy);
			
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Binding events to buttons."));
			//Bind events to alloc/dealloc button
			AllocButton->OnClicked.AddDynamic(this, &UShipMovementStatWidget::AllocButtonListener);
			DeallocButton->OnClicked.AddDynamic(this, &UShipMovementStatWidget::DeallocButtonListener);

			OwningShipPawn->OnMovementEnergyChanged.AddDynamic(this, &UShipMovementStatWidget::OnMovementEnergyUpdate);
			OwningShipPawn->OnTotalEngineChanged.AddDynamic(this, &UShipMovementStatWidget::OnMaxEngineUpdate);

			//Set timer for monitoring speed
			GetWorld()->GetTimerManager().SetTimer(SpedometerTimer, [this]()
				{
					CheckSpeed();
				}, 0.01f, true);
		}
	}
}



void UShipMovementStatWidget::AllocButtonListener() {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Clicked alloc button."));
	OnAllocButtonClicked(ShipMovementStep, 0);
}

void UShipMovementStatWidget::DeallocButtonListener() {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Clicked dealloc button."));
	OnFreeButtonClicked(ShipMovementStep, 0);
}

void UShipMovementStatWidget::OnMovementEnergyUpdate(float Amt) {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Movement energy changed."));
	if (OwningShipPawn) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("OwningShipPawn is valid."));
		//When movement energy changes, first alter bar/text block to show how much energy has been put into movement 
		CurrentMovementEnergy = Amt;
		ModifyTextBox(TBMovementEnergy, Amt);
		ModifyProgressBar(MovementEnergyBar, CurrentMovementEnergy, MaxMovementEnergy);
		//then adjust max speed display
		TopSpeed = CurrentMovementEnergy * ShipSpeedConstant;
		ModifyTextBox(TBMaxSpeed, TopSpeed);
	}
}
void UShipMovementStatWidget::OnMaxEngineUpdate(float Amt) {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Max engine changed."));
	if (OwningShipPawn) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("OwningShipPawn is valid."));
		//When movement energy changes, first alter bar/text block to show how much energy has been put into movement 
		MaxMovementEnergy = Amt;
		ModifyProgressBar(MovementEnergyBar, CurrentMovementEnergy, MaxMovementEnergy);
		//then adjust max speed display
		TopSpeed = CurrentMovementEnergy * ShipSpeedConstant;
		ModifyTextBox(TBMaxSpeed, TopSpeed);
	}
}
void UShipMovementStatWidget::CheckSpeed() {
	if (OwningShipPawn) {
		float Speed = OwningShipPawn->GetCurrentVelocity(false);
		ModifyTextBox(TBCurrSpeed, FormatFloatTenths(Speed));
		ModifyProgressBar(SpedometerBar, Speed, TopSpeed);
	}
}

