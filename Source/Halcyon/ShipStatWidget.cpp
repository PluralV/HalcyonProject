// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipStatWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UShipStatWidget::NativeConstruct() {
	Super::NativeConstruct();
}

void UShipStatWidget::OnAllocButtonClicked(float Amt, int32 TargetSystem) {
	if (OwningShip) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Owning ship passed"));
		//Check if it's a ship pawn
		if (AShipPawn* ShipPawn = Cast<AShipPawn>(OwningShip)) {
			//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Cast pass"));
			//Switch on Target System (STATIC)
			int32 index = -1;
			switch (TargetSystem) {
			case 0:
				ShipPawn->AllocateMovement(Amt);
				return;
			case 1:
				index = 0;
				break;
			case 2:
				index = 1;
				break;
			case 3:
				index = 2;
				break;
			case 4:
				index = 3;
				break;
			case 5:
				index = 4;
				break;
			case 6:
				index = 5;
				break;
				//More logic in future if additional static systems are added
			default:break;
			}
			//Reinforce shield on some facing
			ShipPawn->AllocateReinforceShield(index, Amt);
		}
	}
}

void UShipStatWidget::OnFreeButtonClicked(float Amt, int32 TargetSystem) {
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Free button clicked!:"));
	if (OwningShip) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Owning ship passed"));
		if (AShipPawn* ShipPawn = Cast<AShipPawn>(OwningShip)) {	
			//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Cast pass"));
			int32 index = -1;
			switch (TargetSystem) {
			case 0:
				ShipPawn->FreeMovement(Amt);
				return;
			case 1:
				index = 0;
				break;
			case 2:
				index = 1;
				break;
			case 3:
				index = 2;
				break;
			case 4:
				index = 3;
				break;
			case 5:
				index = 4;
				break;
			case 6:
				index = 5;
				break;
				//More logic in future if additional static systems are added
			default:break;
			}
			//Remove shield reinforcement
			ShipPawn->FreeReinforceShield(index, Amt);
		}
	}
}

void UShipStatWidget::SetOwningShip(AActor* NewOwningShip) {
	if (APawn* ShipPawn = Cast<APawn>(NewOwningShip)) {
		OwningShip = ShipPawn;
		OnOwningShipChanged(OwningShip);
	}
	else if (!NewOwningShip) {
		OnOwningShipChanged(nullptr);
	}
}

FReply UShipStatWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::SpaceBar ||
		InKeyEvent.GetKey() == EKeys::Escape)
	{
		return FReply::Unhandled(); // Pass to controller
	}
	// Handle other keys normally
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FText UShipStatWidget::FormatFloatTenths(float InFloat) {
	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = 0;
	Options.MaximumFractionalDigits = 2;

	return FText::AsNumber(InFloat, &Options);
}

//Alters progress bar based on parameters (Amt/MaxAmt) %
float UShipStatWidget::ModifyProgressBar(UProgressBar* ProgressBar, float Amt, float MaxAmt) {
	float Percent;
	if (Amt == 0) Percent = 0.f;
	else Percent = Amt / MaxAmt;

	if (Percent > 1.f) Percent = 1.f;
	
	ProgressBar->SetPercent(Percent);
	
	return Percent;
}

//Recolors progress bar to Color
void UShipStatWidget::RecolorProgressBar(UProgressBar* ProgressBar, FLinearColor Color) {
	ProgressBar->SetFillColorAndOpacity(Color);
}

//Writes in a text box (one version takes int, one float, one text)
void UShipStatWidget::ModifyTextBox(UTextBlock* TextBlock, int32 Number) {
	TextBlock->SetText(FText::FromString(FString::FromInt(Number)));
}
//Writes in a text box (one version takes int, one float, one text)
void UShipStatWidget::ModifyTextBox(UTextBlock* TextBlock, float NumberFloat) {
	TextBlock->SetText(FormatFloatTenths(NumberFloat));
}
//Writes in a text box (one version takes int, one float, one text)
void UShipStatWidget::ModifyTextBox(UTextBlock* TextBlock, FText Text) {
	TextBlock->SetText(Text);
}

//Recolors font to Color
void UShipStatWidget::RecolorTextBox(UTextBlock* TextBlock, FLinearColor Color) {
	TextBlock->SetColorAndOpacity(Color);
}

FLinearColor UShipStatWidget::GetStatusColorForBar(float Percent) {
	FLinearColor StatusColor = FLinearColor(0.084, 0.896, 0.8);;
	if (Percent <= 0.25) {
		StatusColor = FLinearColor(FLinearColor::Red);
	}
	else if (Percent <= 0.75) {
		StatusColor = FLinearColor(FLinearColor::Yellow);
	}
	return StatusColor;

}