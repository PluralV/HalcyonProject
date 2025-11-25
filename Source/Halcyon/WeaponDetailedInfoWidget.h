// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponDetailedInfoWidget.generated.h"

class AWeaponSystem;
class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class HALCYON_API UWeaponDetailedInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly, Category = "Owner")
	AWeaponSystem* OwningWeapon;

	int32 MyIndex = 0;

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Pawn Action")
	void OnNewOwningWeapon();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblWeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblWeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblWeaponDesc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblWeaponArcs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblWeaponFacing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblWeaponDamageAndRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblWeaponRangeScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblWeaponFireRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblEnergyCost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UTextBlock* LblDamageScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	UButton* CloseButton;

	UFUNCTION(BlueprintCallable, Category = "Pawn Action")
	void OnCloseButtonClicked();
	
};
