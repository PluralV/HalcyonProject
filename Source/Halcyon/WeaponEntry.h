// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponEntry.generated.h"

/**
 * 
 */
UCLASS()
class HALCYON_API UWeaponEntry : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Owner")
	APawn* OwningShip;

	UPROPERTY(BlueprintReadOnly, Category = "Owner")
	AActor* OwningWeapon;

	//Based on bStaticSystem, either allocs/frees to concrete subsystem by index
	//Can be used more than once here
	UFUNCTION(BlueprintCallable, Category = "Pawn Action")
	void OnAllocButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Pawn Action")
	void OnFreeButtonClicked();

protected:
	virtual void NativeConstruct() override;

};
