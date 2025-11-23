// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponDetailedInfoWidget.generated.h"

class AWeaponSystem;
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
};
