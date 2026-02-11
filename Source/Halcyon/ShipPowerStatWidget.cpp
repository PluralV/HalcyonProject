// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPowerStatWidget.h"
#include "ShipPawn.h"
#include "ShipPlayerController.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"


//TODO: Bind events
void UShipPowerStatWidget::NativeConstruct() {
	Super::NativeConstruct();

	if (OwningShip) {
		if (AShipPawn* OSP = Cast<AShipPawn>(OwningShip)) {
			//Set max values for energy stats
			MaxOwningShipEnergy = OSP->GetMaxEnergy();
			MaxLeftEngineHealth = OSP->GetLeftEngMax();
			MaxCentEngineHealth = OSP->GetCentEngMax();
			MaxRightEngineHealth = OSP->GetRightEngMax();
			MaxReactorHealth = OSP->GetReactorMax();

			//Get default values for energy bar/stat initialization; these do not need to be saved
			// (they are only changed via methods that invoke signals)
			float DefaultAvailableEnergy = OSP->GetMaxEnergyCurr();
			int32 DefaultLeftHealth = OSP->GetLeftEngCurr();
			int32 DefaultCentHealth = OSP->GetCentEngCurr();
			int32 DefaultRightHealth = OSP->GetRightEngCurr();
			int32 DefaultReactorHealth = OSP->GetReactorCurr();

			//Fill out progress bar array
			AllProgressBars.Add(LEngBar);
			AllProgressBars.Add(CEngBar);
			AllProgressBars.Add(REngBar);
			AllProgressBars.Add(ReactorBar);
			AllProgressBars.Add(TotalEnergyBar);
			AllProgressBars.Add(TotalEnergyToBeReleasedBar);
			//Fill out textbox array
			AllTextBlocks.Add(TBLeftMax);
			AllTextBlocks.Add(TBCenterMax);
			AllTextBlocks.Add(TBRightMax);
			AllTextBlocks.Add(TBReactorMax);
			AllTextBlocks.Add(TBAvailableEnergy);
			AllTextBlocks.Add(TBEnergyToRelease);
			AllTextBlocks.Add(TBMaxEnergy);
			AllTextBlocks.Add(EnergyLossAmt);
			
			//Bind events for energy changes
			OSP->OnAvailableEnergyChanged.AddDynamic(this, &UShipPowerStatWidget::OnAvailableEnergyChange);
			OSP->OnTotalEnergyChanged.AddDynamic(this, &UShipPowerStatWidget::OnTotalEnergyChange);
			OSP->OnLeftEngChanged.AddDynamic(this, &UShipPowerStatWidget::OnLEngChange);
			OSP->OnCenterEngChanged.AddDynamic(this, &UShipPowerStatWidget::OnCEngChange);
			OSP->OnRightEngChanged.AddDynamic(this, &UShipPowerStatWidget::OnREngChange);
			OSP->OnReactorChanged.AddDynamic(this, &UShipPowerStatWidget::OnReactorChange);
			//Bind events for energy loss/energy regain
			OSP->OnEnergyToBeReleased.AddDynamic(this, &UShipPowerStatWidget::OnImminentEnergyRelease);
			OSP->OnEnergyToBeRestricted.AddDynamic(this, &UShipPowerStatWidget::OnImminentEnergyLoss);

			//Set defaults
			//For cases where we have a ship starting with damage/incomplete power, need to get actual values
			ModifyTextBox(TBAvailableEnergy, DefaultAvailableEnergy);
			ModifyTextBox(TBMaxEnergy, MaxOwningShipEnergy);
			ModifyProgressBar(TotalEnergyBar, DefaultAvailableEnergy, MaxOwningShipEnergy);

			//Engine defaults
			ModifyTextBox(TBLeftMax, DefaultLeftHealth);
			ModifyProgressBar(LEngBar, (float)DefaultLeftHealth, MaxLeftEngineHealth);
			
			ModifyTextBox(TBCenterMax, DefaultCentHealth);
			ModifyProgressBar(CEngBar, (float)DefaultCentHealth, MaxCentEngineHealth);
			
			ModifyTextBox(TBRightMax, DefaultRightHealth);
			ModifyProgressBar(REngBar, (float)DefaultRightHealth, MaxRightEngineHealth);

			ModifyTextBox(TBReactorMax, DefaultReactorHealth);
			ModifyProgressBar(ReactorBar, (float)DefaultReactorHealth, MaxReactorHealth);

		}
	}
	//if (OwningController) {
	//	// ????
	//}

}

//TODO: Implement power-cycle bar and logic for power-cycling once this happens
void UShipPowerStatWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);

}

void UShipPowerStatWidget::OnLEngChange(int32 Amt) {
	ModifyProgressBar(AllProgressBars[0], (float)Amt, MaxLeftEngineHealth);
	ModifyTextBox(AllTextBlocks[0], Amt);
}

void UShipPowerStatWidget::OnCEngChange(int32 Amt) {
	ModifyProgressBar(AllProgressBars[1], (float)Amt, MaxCentEngineHealth);
	ModifyTextBox(AllTextBlocks[1], Amt);
}

void UShipPowerStatWidget::OnREngChange(int32 Amt) {
	ModifyProgressBar(AllProgressBars[2], (float)Amt, MaxRightEngineHealth);
	ModifyTextBox(AllTextBlocks[2], Amt);
}

void UShipPowerStatWidget::OnReactorChange(int32 Amt) {
	ModifyProgressBar(AllProgressBars[3], (float)Amt, MaxReactorHealth);
	ModifyTextBox(AllTextBlocks[3], Amt);
}

//On available energy changed, modify energy bar and textual note
void UShipPowerStatWidget::OnAvailableEnergyChange(float Amt) {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Available energy changed - %f"),Amt));
	ModifyProgressBar(AllProgressBars[4], Amt, MaxOwningShipEnergy);
	ModifyTextBox(AllTextBlocks[4], Amt);
}

//When energy is released
void UShipPowerStatWidget::OnImminentEnergyRelease(float Amt) {
	ModifyProgressBar(AllProgressBars[5], Amt, MaxOwningShipEnergy);
	ModifyTextBox(AllTextBlocks[5], Amt);
}

//On max energy changed, ONLY adjust the textual note
void UShipPowerStatWidget::OnTotalEnergyChange(float Amt) {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Total energy changed - %f"), Amt));
	ModifyTextBox(AllTextBlocks[6], Amt);
}

void UShipPowerStatWidget::OnImminentEnergyLoss(float Amt) {
	ModifyTextBox(AllTextBlocks[7], Amt);
}



