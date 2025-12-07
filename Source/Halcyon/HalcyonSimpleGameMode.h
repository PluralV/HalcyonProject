// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "HalcyonSimpleGameMode.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVVin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoss);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyKilled, int32, RemainingEnemies);

/**
 * 
 */
UCLASS()
class HALCYON_API AHalcyonSimpleGameMode : public AGameModeBase
{
	GENERATED_BODY()
protected:
	virtual void StartPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnVVin OnVVin;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoss OnLoss;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyKilled OnEnemyKilled;

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
