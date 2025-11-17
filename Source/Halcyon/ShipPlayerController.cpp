// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPlayerController.h"
#include "ModularSystem.h"
#include "ShipPawn.h"
#include "Blueprint/UserWidget.h"
#include "HalcyonSimpleGameMode.h"
#include "ShipStatWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


void AShipPlayerController::BeginPlay() {
	Super::BeginPlay();

	check(GEngine);
	SetInputMode(FInputModeGameOnly());
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// Add the default gameplay mapping context
		if (ControllerMappingContext)
		{
			Subsystem->AddMappingContext(ControllerMappingContext, 1);
}
	}
	// Use Game and UI mode from the start
	/*EnableLook();*/
	//TODO: ADD WIDGETS TO HUD

	//1. ADD MOVEMENT WIDGET
	if (ShipMovementWidget) {
		HUDMovement = CreateWidget<UUserWidget>(this, ShipMovementWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDMovement)) {
			StatWidget->OwningShip = GetPawn();
			StatWidget->AddToViewport();
}
	}

	//2. ADD ENERGY WIDGET
	if (ShipEnergyWidget) {
		HUDEnergy = CreateWidget<UUserWidget>(this, ShipEnergyWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDEnergy)) {
			StatWidget->OwningShip = GetPawn();
			StatWidget->AddToViewport();
}
	}

	//3. ADD INTEGRITY WIDGET
	if (ShipIntegrityWidget) {
		HUDIntegrity = CreateWidget<UUserWidget>(this, ShipIntegrityWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDIntegrity)) {
			StatWidget->OwningShip = GetPawn();
			StatWidget->AddToViewport();
		}
	}

	//4. ADD HULL WIDGET
	if (ShipHullWidget) {
		HUDHull = CreateWidget<UUserWidget>(this, ShipHullWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDHull)) {
			StatWidget->OwningShip = GetPawn();
			StatWidget->AddToViewport();
		}
	}

	//5. ADD WEAPON WIDGET
	if (ShipWeaponWidget) {
		HUDWeapons = CreateWidget<UUserWidget>(this, ShipWeaponWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDWeapons)) {
			StatWidget->OwningShip = GetPawn();
			StatWidget->AddToViewport();
		}
	}

	//6. ADD TARGET WIDGET
	if (ShipTargetWidget) {
		HUDTarget = CreateWidget<UUserWidget>(this, ShipTargetWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDTarget)) {
			StatWidget->OwningShip = nullptr;
			StatWidget->OwningController = this;
			StatWidget->AddToViewport();
		}
	}

	if (AGameModeBase* CurrentGameMode = GetWorld()->GetAuthGameMode()) {
		if (AHalcyonSimpleGameMode* HCSM = Cast<AHalcyonSimpleGameMode>(CurrentGameMode)) {
			GameModeIndex = 1;
			HCSM->OnLoss.AddDynamic(this,&AShipPlayerController::HandleLoss);
			HCSM->OnVVin.AddDynamic(this,&AShipPlayerController::HandleWin);
		}//else if else if....
	}
}

void AShipPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	// Use BindAxis instead of BindAction for more reliable mouse button tracking
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {
		EnhancedInputComponent->BindAction(ToggleHUDAction, ETriggerEvent::Started, this, &AShipPlayerController::ToggleHUDInteraction);
		}
		//Reinforce a shield
		CurrentShipPawn->AllocateReinforceShield(amt, index);
	}
	
void AShipPlayerController::ToggleHUDInteraction() {
	if (bIsInHUDMode) {
		bIsInHUDMode = false;
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
	else {
		bIsInHUDMode = true;
		SetInputMode(FInputModeGameAndUI());
		SetShowMouseCursor(true);
	}
}

//MAY NEED WORK: WILL PLAYER CONTROLLER KNOW ABOUT THIS??
void AShipPlayerController::AllocateEnergyToModularSys(AModularSystem* TargetSystem, int32 amt) {
	TargetSystem->AllocateEnergy(amt);
	}

void AShipPlayerController::FreeEnergyFromModularSys(AModularSystem* TargetSystem, int32 amt) {
	TargetSystem->FreeEnergy(amt);
}

void AShipPlayerController::AcquireTargetToHud(AActor* Target) {
	if (APawn* TargetShip = Cast<APawn>(Target)) {
		if (HUDTarget && ShipTargetWidget) {
			if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDTarget)) {
				StatWidget->SetOwningShip(TargetShip);
			}
		}
	}
}

void AShipPlayerController::HandleWin() {
	switch (GameModeIndex) {
	case 1:
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("You vvin :)")));
		return;
	default:break;
	}
}

void AShipPlayerController::HandleLoss() {
	switch (GameModeIndex) {
	case 1:
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("You lost :(")));
		return;
	default:break;
	}
}

