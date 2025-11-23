// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponEntry.generated.h"

class AWeaponSystem;
class UProgressBar;
class UTextBlock;
class UButton;
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
	AWeaponSystem* OwningWeapon;

	//Attempts to allocate/free energy from the weapon
	//Can be used more than once here
	UFUNCTION(BlueprintCallable, Category = "Pawn Action")
	void OnAllocButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Pawn Action")
	void OnCtrlGroupButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Pawn Action")
	void OnInfoButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Pawn Action")
	void OnEnergyChanged();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* EnergyLevelProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UProgressBar* Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblWeaponArc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblWeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblStatus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblCooldownStatus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblControlGroup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* EnergyLevelCurr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UButton* AllocButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UButton* InfoButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UButton* CtrlGroupButton;

	int32 MyIndex = -1;//Keeps index of weapon in the array stored by ship owner

protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	UFUNCTION(BlueprintImplementableEvent, Category = "Target")
	void OnEnergyUpdated(int32 NewCurrentEnergy);
	void AdjustButtonBackgroundColor(FLinearColor StatusColor);
private:
	bool bIsReady = false;
};
