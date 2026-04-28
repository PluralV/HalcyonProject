// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShipStatWidget.h"
#include "ShipTargetingStatWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UButton;
class AShipPawn;
class UBorder;
/**
 * 
 */
UCLASS()
class HALCYON_API UShipTargetingStatWidget : public UShipStatWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	//Shows either the name of the type of ship you're targeting or "No Target"
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetOrNoTarget;
	//Shows either the class of the ship you're targeting or "N/A"
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* CurrentTargetClass;

	//Shows speed of target
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetSpeed;
	//Shows range to target
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetRange;


	//Target hull integrity
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetHealthBar;

	//Text labels for integrity
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetHullCurr;

	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetHullMax;

	//Border containing shield diagram
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UBorder* TargetDiagramBorder;
	

	//Health bars to show the current power system health of the enemy
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetLeftEngine;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetRightEngine;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetCenterEngine;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetReactor;

	//Bars showing shield strength of enemy ship
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	TArray<UProgressBar*> TargetShields;
	//Bars showing shield strength of enemy ship
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	TArray<UTextBlock*> TargetShieldsText;

	//Actual bound shield bars
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetShield1;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetShield2;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetShield3;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetShield4;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetShield5;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TargetShield6;

	//Text blocks for shield health
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetShield1Curr;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetShield2Curr;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetShield3Curr;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetShield4Curr;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetShield5Curr;
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TargetShield6Curr;

	//Button used to allocate power to and activate targeting system
	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UButton* TurnOnTargeterButton;

private:
	AShipPawn* OwningShipPawn;
	AShipPawn* TargetShipPawn;

	UFUNCTION()
	void HandleActivateTargeter();
	
	UFUNCTION()
	void HandleControlShiftButton(int32 NewCtrl);

	UFUNCTION()
	void HandleOnShieldStrengthChanged(int32 Index, int32 Amt);

	UFUNCTION()
	void HandleOnHullIntegrityChanged(int32 Amt);

	UFUNCTION()
	void UpdateTargetShip();

	UFUNCTION()
	void ResetTargeterToNeutral();

	UFUNCTION()
	void InitializeTargeterToShip();

};
