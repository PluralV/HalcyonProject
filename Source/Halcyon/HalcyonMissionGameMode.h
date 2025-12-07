// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ObjectivePoint.h"
#include "ShipSpawnPoint.h"
#include "HalcyonMissionGameMode.generated.h"




USTRUCT(BlueprintType)
struct FObjectiveInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ObjectiveDesc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ObjectiveTitle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AObjectivePoint* AssociatedObjective;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AShipSpawnPoint*> AssociatedSpawners;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ObjectiveType; //0: interact, 1: kill, 2: protect (?)

	FObjectiveInfo() :
		ObjectiveDesc(FText::GetEmpty()),
		ObjectiveTitle(FText::GetEmpty()),
		AssociatedObjective(nullptr),
		AssociatedSpawners({}),
		ObjectiveType(0)
	{
	};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionWin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionLoss);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCurrentObjectiveComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentObjectiveSet, FObjectiveInfo, NewCurrentObjective);

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

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCurrentObjectiveComplete OnCurrentObjectiveComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCurrentObjectiveSet OnCurrentObjectiveSet;

	//TODO CHANGE!!!
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Win Condition")
	uint8 EnemiesRemaining = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission Details")
	FText MissionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Win Condition")
	TArray<FObjectiveInfo> ObjectiveList;

	FObjectiveInfo CurrentObjective = FObjectiveInfo();

	UFUNCTION(BlueprintCallable)
	FObjectiveInfo GetCurrentObjective();

	UFUNCTION()
	void DecrementEnemies();

	UFUNCTION()
	void IncrementEnemies(int32 ByAmt);

	UFUNCTION()
	void CauseLoss();

	UFUNCTION()
	virtual void HandleObjectiveCompletion(int32 ObjectiveIndex);

	UFUNCTION()
	virtual void ActivateCurrentObjective();

private:
	void CheckEnemies();
	int32 ObjectiveCount;
	int32 CurrentObjectiveIndex;
};
