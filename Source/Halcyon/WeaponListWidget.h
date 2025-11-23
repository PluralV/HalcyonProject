// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShipPawn.h"
#include "ShipPlayerController.h"
#include "WeaponSystem.h"
#include "WeaponListWidget.generated.h"


class UWeaponEntry;
/**
 * 
 */
UCLASS()
class HALCYON_API UWeaponListWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Owning Ship")
	APawn* OwningShip;

	UPROPERTY(BlueprintReadOnly, Category = "Owning Ship")
	APlayerController* OwningController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Widgets", meta = (BindWidget))
	class UPanelWidget* ContainerPanel;

	UPROPERTY(EditAnywhere, Category = "Child Widgets")
	TSubclassOf<UWeaponEntry> WeaponEntryClass;

	UPROPERTY(BlueprintReadOnly, Category = "Child Widgets")
	TArray<UUserWidget*> WeaponEntryList;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Target")
	void OnOwningShipChanged(APawn* NewOwningShip);
};
