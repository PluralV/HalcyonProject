// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HalcyonMissionGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionWin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionLoss);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class HALCYON_API AHalcyonMissionGameMode : public AGameModeBase
{
	GENERATED_BODY()
protected:
	virtual void StartPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMissionWin OnMissionWin;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMissionLoss OnMissionLoss;

	//TODO CHANGE!!!
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Win Condition")
	uint8 EnemiesRemaining = 0;

	UFUNCTION()
	void DecrementEnemies();

	UFUNCTION()
	void IncrementEnemies();

	UFUNCTION()
	void CauseLoss();

private:
	void CheckEnemies();
};
