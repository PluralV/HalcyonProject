// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShipPawn.h"
#include "ShipPlayerController.h"
#include "ShipStatWidget.generated.h"

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

	//Based on bStaticSystem, either allocs/frees to concrete subsystem by index
	//Can be used more than once here
	UFUNCTION(BlueprintCallable, Category="Pawn Action")
	void OnAllocButtonClicked(int32 amt, int32 TargetSystem);

	UFUNCTION(BlueprintCallable, Category = "Pawn Action")
	void OnFreeButtonClicked(int32 amt, int32 TargetSystem);

protected:
	virtual void NativeConstruct() override;
};
