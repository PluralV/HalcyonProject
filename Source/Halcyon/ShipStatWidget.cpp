// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipStatWidget.h"


void UShipStatWidget::NativeConstruct() {
	Super::NativeConstruct();
}

void UShipStatWidget::OnAllocButtonClicked(int32 amt, int32 TargetSystem) {
	if (OwningShip) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Owning ship passed"));
		//Check if it's a ship pawn
		if (AShipPawn* ShipPawn = Cast<AShipPawn>(OwningShip)) {
			//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Cast pass"));
			//Switch on Target System (STATIC)
			int32 index = -1;
			switch (TargetSystem) {
			case 0:
				ShipPawn->AllocateMovement(amt);
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
			ShipPawn->AllocateReinforceShield(amt, index);
		}
	}
}

void UShipStatWidget::OnFreeButtonClicked(int32 amt, int32 TargetSystem) {
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Free button clicked!:"));
	if (OwningShip) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Owning ship passed"));
		if (AShipPawn* ShipPawn = Cast<AShipPawn>(OwningShip)) {	
			//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Cast pass"));
			int32 index = -1;
			switch (TargetSystem) {
			case 0:
				ShipPawn->FreeMovement(amt);
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
			ShipPawn->FreeReinforceShield(amt, index);
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
