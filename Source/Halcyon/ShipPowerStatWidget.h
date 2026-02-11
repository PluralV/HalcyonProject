// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShipStatWidget.h"
#include "ShipPowerStatWidget.generated.h"

class UProgressBar;
class UTextBlock;
/**
 * 
 */
UCLASS()
class HALCYON_API UShipPowerStatWidget : public UShipStatWidget
{
	GENERATED_BODY()
public:
	//Bar showing total available energy as fraction of maximum energy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TotalEnergyBar;

	//Bar showing amount of energy that will be sent back into the system after next cycle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* TotalEnergyToBeReleasedBar;

	//Bar showing time before next cycle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* PowerCycleCooldownBar;

	//Bar showing health of left engine
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* LEngBar;

	//Bar showing health of center engine
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* CEngBar;

	//Bar showing health of right engine
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* REngBar;

	//Bar showing health of reactor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* ReactorBar;

	//Shows amount of energy to be released on next cycle as digit
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBEnergyToRelease;

	//Displays maximum energy of ship
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBMaxEnergy;

	//Displays total energy available for ship right now
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBAvailableEnergy;

	//Displays total energy produced by left engine
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBLeftMax;
	//Displays total energy produced by center engine
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBCenterMax;
	//Displays total energy produced by right engine
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBRightMax;
	//Displays total energy produced by reactor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* TBReactorMax;
	//Displays amount of energy to be lost on next power cycle
	//TODO: CAN PROBABLY MERGE THIS WITH ENERGY RELEASED ONCE POWER CYCLE IMPLEMENTED
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* EnergyLossAmt;

protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	//Owning ship max energy characteristics; assumed to remain constant
	float MaxOwningShipEnergy;
	float MaxLeftEngineHealth;
	float MaxRightEngineHealth;
	float MaxCentEngineHealth;
	float MaxReactorHealth;

private:
	//Handlers for when engine/powersystem health changes
	UFUNCTION()
	void OnReactorChange(int32 Amt);
	UFUNCTION()
	void OnLEngChange(int32 Amt);
	UFUNCTION()
	void OnREngChange(int32 Amt);
	UFUNCTION()
	void OnCEngChange(int32 Amt);

	//Handler for when total energy changes
	UFUNCTION()
	void OnTotalEnergyChange(float Amt);

	//Handler for when energy will be restricted
	UFUNCTION()
	void OnImminentEnergyLoss(float Amt);

	//Handler for when energy will be released
	UFUNCTION()
	void OnImminentEnergyRelease(float Amt);

	//Handler for when available energy changes
	UFUNCTION()
	void OnAvailableEnergyChange(float Amt);

	//Indexes progress bars; 0-3 power (L, C, R, Reactor) 4 TotalEnergy, 5 ToBeReleased
	UPROPERTY()
	TArray<UProgressBar*> AllProgressBars;
	//Indexes text boxes: 0-5 map to power (same as the progress bars), 6 max energy, 7 energy to be lost
	UPROPERTY()
	TArray<UTextBlock*> AllTextBlocks;

};
