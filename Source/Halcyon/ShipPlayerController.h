// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShipPlayerController.generated.h"

class AModularSystem;
class UInputMappingContext;
class UInputAction;
class AWeaponSystem;

UENUM()
enum class EStaticSystemType : int8 {
	Movement UMETA(DisplayName = "Movement"),
	Shield1 UMETA(DisplayName = "Reinforce #1 Shield"),
	Shield2 UMETA(DisplayName = "Reinforce #2 Shield"),
	Shield3 UMETA(DisplayName = "Reinforce #3 Shield"),
	Shield4 UMETA(DisplayName = "Reinforce #4 Shield"),
	Shield5 UMETA(DisplayName = "Reinforce #5 Shield"),
	Shield6 UMETA(DisplayName = "Reinforce #6 Shield")
};

UCLASS()
class HALCYON_API AShipPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	void AcquireTargetToHud(AActor* Target);
	void AddWeaponWidget();

	UFUNCTION(BlueprintCallable)
	void SetWeaponDetails(AWeaponSystem* NewOwningWeapon, int32 ItsIndex);
	
	UFUNCTION(BlueprintCallable)
	bool GetPauseStatus();

	UFUNCTION(BlueprintCallable)
	float GetPauseChargePercent();

	UFUNCTION(BlueprintCallable)
	void PauseRealtimeGame();

protected:
	void InitializeHUD();

	virtual void Tick(float DeltaTime) override;
	// Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* ControllerMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ToggleHUDAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PauseAction;

	//Widgets to appear on player HUD and give info about ship stats:
	/*
	MovementInfo: 
		- Current Speed
		- Current Top Speed
		- Turn rate
		- Energy allocated to movement
		- Engine health (L, R, C, based on maximums)
		- Buttons: Allocate to movement, deallocate from movement
	
	Energy: 
		- Maximum Energy
		- Current Available Energy
		- Progress bar to visualize this

	ShipIntegrity:
		- Shield strength on all facings
		- Number of hull points
		- Total integrity (HP?)
		- Buttons: Allocate energy to shield (boosts shield by 1 per power for <INTERVAL> before energy is freed)
	
	WeaponInfo:
		- List of weapons featuring name and status (active, inactive, destroyed)
		- All entries clickable
		- On Click: Display a secondary widget which shows: 
			Weapon name
			Weapon arc
			Weapon range
			Weapon damage range (could be more detailed stats but this depends on how complicated it is)
			Allocated energy - button to allocate energy or free energy (allocated energy is there for <INTERVAL> before
			freeing becomes possible)
			Draws arc/range on the screen
		- Buttons: Allocate/free energy from each weapon
		- Key binds: Group weapons so they shoot together (TODO, future feature)
	*/

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> ShipMovementWidget;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> ShipEnergyWidget;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> ShipIntegrityWidget;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> ShipHullWidget;
	
	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> ShipWeaponWidget;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> ShipTargetWidget;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> PauseWidget;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> EAWidget;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> WeaponDetailsWidget;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> PauseStatusWidget;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> ObjectiveListWidget;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Win Widget")
	TSubclassOf<UUserWidget> WinWidgetClass;

	UPROPERTY()
	UUserWidget* HUDMovement;
	UPROPERTY()
	UUserWidget* HUDEnergy;
	UPROPERTY()
	UUserWidget* HUDIntegrity;
	UPROPERTY()
	UUserWidget* HUDWeapons;
	UPROPERTY()
	UUserWidget* HUDHull;
	UPROPERTY()
	UUserWidget* HUDTarget;
	UPROPERTY()
	UUserWidget* HUDPaused;
	UPROPERTY()
	UUserWidget* HUDWeaponDetails;
	UPROPERTY()
	UUserWidget* HUDPauseStatus;
	UPROPERTY()
	UUserWidget* HUDObjectiveList;
	UPROPERTY()
	UUserWidget* WinWidget;

	/*Separate functions will exist for allocating energy to systems that are static / default
	(i.e. movement, shield reinforcement, <potential> repairs/electronic warfare) and those that
	are modular and specific to a ship (or specific to one module) (i.e. weapons)
	*/

	//UFUNCTION(BlueprintCallable)
	//void AllocateEnergyToModularSys(AModularSystem* TargetSystem, int32 amt);

	//UFUNCTION(BlueprintCallable)
	//void FreeEnergyFromModularSys(AModularSystem* TargetSystem, int32 amt);
	
	void ToggleHUDInteraction();

	

	

	UFUNCTION(BlueprintCallable)
	void ClearWeaponDetails();
	

	//For disabling/enabling mouse look; when disabled you can interact with HUD UI
	//currently bound to RightMouse
//	void DisableLook();
//	void EnableLook();
private:
	
	
	int32 GameModeIndex = -1;
	float TimeSinceLastPause = 32.f;
	const float PauseTimeCooldown = 32.f;
	bool bIsInHUDMode = false;
	bool bIsPaused = false;
	bool bIsOnFirstPause = true;
//	void OnRightMouseAxis(float Value);
//	bool bIsRightMouseDown = false;
	UFUNCTION()
	void HandleWin();
	
	UFUNCTION()
	void HandleLoss();


};
