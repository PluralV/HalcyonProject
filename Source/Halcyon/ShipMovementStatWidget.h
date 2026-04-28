// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShipStatWidget.h"
#include "ShipMovementStatWidget.generated.h"

class AShipPawn;
class UProgressBar;
class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class HALCYON_API UShipMovementStatWidget : public UShipStatWidget
{
	GENERATED_BODY()
public:
	//Bar showing total energy allocated to movement
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* MovementEnergyBar;

	//Bar showing how fast ship is moving relative to top speed at current energy level
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* SpedometerBar;

	//Text block displaying movement energy
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBMovementEnergy;

	//Text block displaying maximum movement energy
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBMovementEnergyMax;

	//Text block displaying movement speed
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBCurrSpeed;

	//Text block displaying max speed at current energy level
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBMaxSpeed;

	//Button that allocates 1 energy step to the engines
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UButton* AllocButton;

	//Button that frees 1 energy step from the engines
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UButton* DeallocButton;

	FTimerHandle SpedometerTimer;



protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnMovementEnergyUpdate(float Amt);
	UFUNCTION()
	void OnMaxEngineUpdate(float Amt);
	UFUNCTION()
	void CheckSpeed();
	UFUNCTION()
	void AllocButtonListener();
	UFUNCTION()
	void DeallocButtonListener();

	AShipPawn* OwningShipPawn;
	float ShipMovementStep;
	float ShipSpeedConstant;
	float MaxMovementEnergy;
	float CurrentMovementEnergy;
	float TopSpeed;
};
