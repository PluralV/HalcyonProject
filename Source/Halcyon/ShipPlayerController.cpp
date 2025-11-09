// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPlayerController.h"
#include "ModularSystem.h"
#include "ShipPawn.h"
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

}

void AShipPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	// Use BindAxis instead of BindAction for more reliable mouse button tracking
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {
		EnhancedInputComponent->BindAction(ToggleHUDAction, ETriggerEvent::Started, this, &AShipPlayerController::ToggleHUDInteraction);
	}
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

//void AShipPlayerController::OnRightMouseAxis(float Value) {
//	// Value is 1.0 when pressed, 0.0 when released
//	if (Value > 0.5f)
//	{
//		// Button is pressed
//		if (!bIsRightMouseDown)
//		{
//			bIsRightMouseDown = true;
//			DisableLook();
//		}
//	}
//	else
//	{
//		// Button is released
//		if (bIsRightMouseDown)
//		{
//			bIsRightMouseDown = false;
//			EnableLook();
//		}
//	}
//}

//void AShipPlayerController::DisableLook()
//{
//	// Right click pressed - show cursor, disable look
//	UE_LOG(LogTemp, Warning, TEXT("DisableLook called"));
//
//	FInputModeGameAndUI InputMode;
//	InputMode.SetHideCursorDuringCapture(false);
//	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
//
//	SetInputMode(InputMode);
//	SetShowMouseCursor(true);
//	SetIgnoreLookInput(true);
//}
//
//void AShipPlayerController::EnableLook()
//{
//	// Right click released - hide cursor, enable look
//	UE_LOG(LogTemp, Warning, TEXT("EnableLook called"));
//
//	FInputModeGameAndUI InputMode;
//	InputMode.SetHideCursorDuringCapture(true);
//	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);
//
//	SetInputMode(InputMode);
//	SetShowMouseCursor(false);
//	SetIgnoreLookInput(false);
//}

//MAY NEED WORK: WILL PLAYER CONTROLLER KNOW ABOUT THIS??
void AShipPlayerController::AllocateEnergyToModularSys(AModularSystem* TargetSystem, int32 amt) {
	TargetSystem->AllocateEnergy(amt);
}

void AShipPlayerController::FreeEnergyFromModularSys(AModularSystem* TargetSystem, int32 amt) {
	TargetSystem->FreeEnergy(amt);
}

void AShipPlayerController::AllocateEnergyToStaticSys(EStaticSystemType type, int32 amt) {
	APawn* CurrentPawn = GetPawn();
	if (AShipPawn* CurrentShipPawn = Cast<AShipPawn>(CurrentPawn)) {
		int32 index = -1;
		switch (type) {
		case EStaticSystemType::Movement:
			CurrentShipPawn->AllocateMovement(amt);
			return;
		case EStaticSystemType::Shield1:
			index = 0;
			break;
		case EStaticSystemType::Shield2:
			index = 1;
			break;
		case EStaticSystemType::Shield3:
			index = 2;
			break;
		case EStaticSystemType::Shield4:
			index = 3;
			break;
		case EStaticSystemType::Shield5:
			index = 4;
			break;
		case EStaticSystemType::Shield6:
			index = 5;
			break;
			//More logic in future if additional static systems are added
		default:break;
		}
		//Reinforce a shield
		CurrentShipPawn->AllocateReinforceShield(amt, index);
	}
	
}

void AShipPlayerController::FreeEnergyFromStaticSys(EStaticSystemType type, int32 amt) {
	APawn* CurrentPawn = GetPawn();
	if (AShipPawn* CurrentShipPawn = Cast<AShipPawn>(CurrentPawn)) {
		int32 index = -1;
		switch (type) {
		case EStaticSystemType::Movement:
			CurrentShipPawn->FreeMovement(amt);
			return;
		case EStaticSystemType::Shield1:
			index = 0;
			break;
		case EStaticSystemType::Shield2:
			index = 1;
			break;
		case EStaticSystemType::Shield3:
			index = 2;
			break;
		case EStaticSystemType::Shield4:
			index = 3;
			break;
		case EStaticSystemType::Shield5:
			index = 4;
			break;
		case EStaticSystemType::Shield6:
			index = 5;
			break;
			//More logic in future if additional static systems are added
		default:break;
		}
		//Reinforce a shield
		CurrentShipPawn->FreeReinforceShield(amt, index);
	}
}