// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HalcyonSimpleGameMode.generated.h"

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
	//TODO CHANGE!!!
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Win Condition")
	uint8 EnemiesRemaining = 0;

	UFUNCTION()
	void DecrementEnemies();

	UFUNCTION()
	void IncrementEnemies();
};
