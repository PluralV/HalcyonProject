// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShipPawn.h"
#include "ShipPlayerController.h"
#include "ShipStatWidget.generated.h"

class UProgressBar;
class UTextBlock;
/*
 SHIP SYSTEM STAT WIDGET
 - Used for showing ship stats and ship static system energy allocation. Not used for modular systems (which has to be more dynamic)
 */
UCLASS()
class HALCYON_API UShipStatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Owning Ship")
	APawn* OwningShip;

	UPROPERTY(BlueprintReadOnly, Category = "Owning Ship")
	APlayerController* OwningController;

	//Based on bStaticSystem, either allocs/frees to concrete subsystem by index
	//Can be used more than once here
	UFUNCTION(BlueprintCallable, Category="Pawn Action")
	void OnAllocButtonClicked(float amt, int32 TargetSystem);

	UFUNCTION(BlueprintCallable, Category = "Pawn Action")
	void OnFreeButtonClicked(float amt, int32 TargetSystem);

	UFUNCTION(BlueprintCallable, Category = "Setter")
	void SetOwningShip(AActor* NewOwningShip);

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Target")
	void OnOwningShipChanged(APawn* NewOwningShip);

	//Alters progress bar based on parameters (Amt/MaxAmt) % and returns the percentage
	float ModifyProgressBar(UProgressBar* ProgressBar, float Amt, float MaxAmt);

	//Recolors progress bar to Color
	void RecolorProgressBar(UProgressBar* ProgressBar, FLinearColor Color);

	//Writes in a text box (one version takes int, one float, one text)
	void ModifyTextBox(UTextBlock* TextBlock, int32 Number);
	//Writes in a text box (one version takes int, one float, one text)
	void ModifyTextBox(UTextBlock* TextBlock, float NumberFloat);
	//Writes in a text box (one version takes int, one float, one text)
	void ModifyTextBox(UTextBlock* TextBlock, FText Text);
	//Recolors text box font to Color
	void RecolorTextBox(UTextBlock* TextBlock, FLinearColor Color);
	//Formats floats into max-tenth digit decimals for displaying energy level
	FText FormatFloatTenths(float InFloat);

	FLinearColor GetStatusColorForBar(float Percent);
};
