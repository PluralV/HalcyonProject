// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPlayerController.h"
#include "WeaponSystem.h"
#include "ShipPawn.h"
#include "Blueprint/UserWidget.h"
#include "WeaponListWidget.h"
#include "HalcyonSimpleGameMode.h"
#include "HalcyonMissionGameMode.h"
#include "ShipStatWidget.h"
#include "WeaponDetailedInfoWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"


void AShipPlayerController::BeginPlay() {
	Super::BeginPlay();
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("CONTROLLER BEGIN"));
	check(GEngine);
	SetInputMode(FInputModeGameOnly());
	bShouldPerformFullTickWhenPaused = true;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// Add the default gameplay mapping context
		if (ControllerMappingContext)
		{
			Subsystem->AddMappingContext(ControllerMappingContext, 0);
		}

	}

	if (AGameModeBase* CurrentGameMode = GetWorld()->GetAuthGameMode()) {
		if (AHalcyonSimpleGameMode* HCSM = Cast<AHalcyonSimpleGameMode>(CurrentGameMode)) {
			GameModeIndex = 1;
			HCSM->OnLoss.AddDynamic(this, &AShipPlayerController::HandleLoss);
			HCSM->OnVVin.AddDynamic(this, &AShipPlayerController::HandleWin);
		}//else if else if....
		else if (AHalcyonMissionGameMode* HCMM = Cast<AHalcyonMissionGameMode>(CurrentGameMode)) {
			GameModeIndex = 2;//TODO CHANGE
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("Binding win conditions for le Halcyon Mission")));
			HCMM->OnMissionLoss.AddDynamic(this, &AShipPlayerController::HandleLoss);
			HCMM->OnMissionWin.AddDynamic(this, &AShipPlayerController::HandleWin);
		}
	}

	GetWorldTimerManager().SetTimerForNextTick(this, &AShipPlayerController::InitializeHUD);
}


void AShipPlayerController::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (TimeSinceLastPause < PauseTimeCooldown && !bIsPaused) {
		float TempTime = TimeSinceLastPause + DeltaTime;
		TimeSinceLastPause = TempTime > PauseTimeCooldown ? PauseTimeCooldown : TempTime;
	}

}

void AShipPlayerController::InitializeHUD() {
	//1. ADD MOVEMENT WIDGET
	if (ShipMovementWidget) {
		HUDMovement = CreateWidget<UUserWidget>(this, ShipMovementWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDMovement)) {
			StatWidget->OwningShip = GetPawn();
			StatWidget->SetIsFocusable(false);
			StatWidget->AddToViewport();
		}
	}

	//2. ADD ENERGY WIDGET
	if (ShipEnergyWidget) {
		HUDEnergy = CreateWidget<UUserWidget>(this, ShipEnergyWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDEnergy)) {
			StatWidget->OwningShip = GetPawn();
			StatWidget->SetIsFocusable(false);
			StatWidget->AddToViewport();
		}
	}

	//3. ADD INTEGRITY WIDGET
	if (ShipIntegrityWidget) {
		HUDIntegrity = CreateWidget<UUserWidget>(this, ShipIntegrityWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDIntegrity)) {
			StatWidget->OwningShip = GetPawn();
			StatWidget->SetIsFocusable(false);
			StatWidget->AddToViewport();
		}
	}

	//4. ADD HULL WIDGET
	if (ShipHullWidget) {
		HUDHull = CreateWidget<UUserWidget>(this, ShipHullWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDHull)) {
			StatWidget->OwningShip = GetPawn();
			StatWidget->SetIsFocusable(false);
			StatWidget->AddToViewport();
		}
	}

	//5. ADD TARGET WIDGET
	if (ShipTargetWidget) {
		HUDTarget = CreateWidget<UUserWidget>(this, ShipTargetWidget);
		if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDTarget)) {
			StatWidget->OwningShip = nullptr;
			StatWidget->OwningController = this;
			StatWidget->SetIsFocusable(false);
			StatWidget->AddToViewport();
		}
	}
	//6. ADD WEAPON DETAILS WIDGET
	if (WeaponDetailsWidget) {
		HUDWeaponDetails = CreateWidget<UUserWidget>(this, WeaponDetailsWidget);
		if (UWeaponDetailedInfoWidget* WDIW = Cast<UWeaponDetailedInfoWidget>(HUDWeaponDetails)) {
			//TODO: set its characteristics as possible
			WDIW->OwningWeapon = nullptr;
			WDIW->MyIndex = -1;
			WDIW->OnNewOwningWeapon();
			WDIW->SetIsFocusable(false);
			WDIW->SetVisibility(ESlateVisibility::Hidden);
			WDIW->AddToViewport();
		}
	}
	//7. ADD PAUSE STATUS WIDGET
	if (PauseStatusWidget) {
		HUDPauseStatus = CreateWidget<UUserWidget>(this, PauseStatusWidget);
		if (UShipStatWidget* SSW = Cast<UShipStatWidget>(HUDPauseStatus)) {
			//TODO: set its characteristics as possible
			SSW->OwningShip = GetPawn();
			SSW->OwningController = this;
			SSW->SetIsFocusable(false);
			SSW->AddToViewport();
		}
	}

	//8. ADD OBJECTIVE WIDGET
	if (ObjectiveListWidget) {
		HUDObjectiveList = CreateWidget<UUserWidget>(this, ObjectiveListWidget);
		if (UShipStatWidget* SSW = Cast<UShipStatWidget>(HUDObjectiveList)) {
			SSW->OwningShip = GetPawn();
			SSW->OwningController = this;
			SSW->SetIsFocusable(false);
			SSW->AddToViewport();
		}
	}

	if (WinWidgetClass) {
		WinWidget = CreateWidget<UUserWidget>(this, WinWidgetClass);
		WinWidget->SetVisibility(ESlateVisibility::Hidden);
		WinWidget->AddToViewport();
	}
}

void AShipPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();
	
	//InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AShipPlayerController::PauseRealtimeGame);
	//InputComponent->BindAction("PauseGame", IE_Pressed, this, &AShipPlayerController::PauseRealtimeGame);
	// Use BindAxis instead of BindAction for more reliable mouse button tracking
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {
		EnhancedInputComponent->BindAction(ToggleHUDAction, ETriggerEvent::Triggered, this, &AShipPlayerController::ToggleHUDInteraction);
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Triggered, this, &AShipPlayerController::PauseRealtimeGame);
		}
}
	
void AShipPlayerController::ToggleHUDInteraction() {
	if (bIsInHUDMode) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Setting input to GAME ONLY"));
		bIsInHUDMode = false;
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
	}
	else {
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Setting input to GAME AND UI"));
		bIsInHUDMode = true;
		SetInputMode(FInputModeGameAndUI());
		SetShowMouseCursor(true);
	}
}

////MAY NEED WORK: WILL PLAYER CONTROLLER KNOW ABOUT THIS??
//void AShipPlayerController::AllocateEnergyToModularSys(AModularSystem* TargetSystem, int32 amt) {
//	TargetSystem->AllocateEnergy(amt);
//	}
//
//void AShipPlayerController::FreeEnergyFromModularSys(AModularSystem* TargetSystem, int32 amt) {
//	TargetSystem->FreeEnergy(amt);
//}

void AShipPlayerController::AcquireTargetToHud(AActor* Target) {
	if (APawn* TargetShip = Cast<APawn>(Target)) {
		if (HUDTarget && ShipTargetWidget) {
			if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDTarget)) {
				StatWidget->SetOwningShip(TargetShip);
			}
		}
	}
	else if (!Target) {
		if (HUDTarget && ShipTargetWidget) {
			if (UShipStatWidget* StatWidget = Cast<UShipStatWidget>(HUDTarget)) {
				StatWidget->SetOwningShip(nullptr);
			}
		}
	}
}


//TODO (PREFERABLE): Better integrate the situation with the WinWidget
void AShipPlayerController::HandleWin() {
	switch (GameModeIndex) {
	case 1:
	case 2:
	default:
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("You vvin :)")));
		break;
	}
}

void AShipPlayerController::HandleLoss() {
	switch (GameModeIndex) {
	case 1:
	case 2:
	default:
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("You lost :("))); 
		break;
	}
}

void AShipPlayerController::PauseRealtimeGame() {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("PAUSE INPUT RECEIVED"));
	if (TimeSinceLastPause < PauseTimeCooldown){
		return;
	}
	bIsPaused = !bIsPaused;
	if (bIsPaused) {
		// Pausing
		bIsInHUDMode = true;
		SetInputMode(FInputModeGameAndUI());
		SetShowMouseCursor(true);
		if (PauseWidget) {
			HUDPaused = CreateWidget<UUserWidget>(this, PauseWidget);
			HUDPaused->SetIsFocusable(false);
			HUDPaused->AddToViewport();
		}
	}
	else {
		// Unpausing
		bIsInHUDMode = false;
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
		if (HUDPaused) {
			HUDPaused->RemoveFromParent();
		}
		TimeSinceLastPause = 0.f;
	}
	SetPause(bIsPaused, FCanUnpause());
	
}

void AShipPlayerController::AddWeaponWidget() {
	//ADD WEAPON WIDGET
	if (ShipWeaponWidget) {
		HUDWeapons = CreateWidget<UUserWidget>(this, ShipWeaponWidget);
		if (UWeaponListWidget* StatWidget = Cast<UWeaponListWidget>(HUDWeapons)) {
			StatWidget->OwningShip = GetPawn();
			StatWidget->SetIsFocusable(false);
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Adding the weapon hud element"));
			StatWidget->AddToViewport();
		}
	}
}

//Set the weapon detail widget to be visible with a specific owning weapon
void AShipPlayerController::SetWeaponDetails(AWeaponSystem* NewOwningWeapon, int32 ItsIndex) {
	if (UWeaponDetailedInfoWidget* WDIW = Cast<UWeaponDetailedInfoWidget>(HUDWeaponDetails)) {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Cast success"));
		WDIW->OwningWeapon = NewOwningWeapon;
		WDIW->MyIndex = ItsIndex;
		WDIW->OnNewOwningWeapon();
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Setting visibility?"));
		WDIW->SetVisibility(ESlateVisibility::Visible);
	}
}

//Remove the weapon detail widget from visibility
void AShipPlayerController::ClearWeaponDetails() {
	if (UWeaponDetailedInfoWidget* WDIW = Cast<UWeaponDetailedInfoWidget>(HUDWeaponDetails)) {
		WDIW->OwningWeapon = nullptr;
		WDIW->SetIsFocusable(false);
		WDIW->SetVisibility(ESlateVisibility::Hidden);
	}
}

float AShipPlayerController::GetPauseChargePercent() {
	return TimeSinceLastPause > PauseTimeCooldown ? 1.0 : TimeSinceLastPause / PauseTimeCooldown;
}

bool AShipPlayerController::GetPauseStatus() {
	return bIsPaused;
}