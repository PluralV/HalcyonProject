// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipPawn.h"
#include "ShipSpawnPoint.generated.h"


class AAIController;
UCLASS()
class HALCYON_API AShipSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShipSpawnPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawned Ships")
	TArray<TSubclassOf<AShipPawn>> SpawnedShipClasses;

	// NEW: AI Controller class to use for spawned ships
	UPROPERTY(EditAnywhere, Category = "Spawned Ships")
	TSubclassOf<AAIController> AIControllerClass;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawned Ships")
	float SpawnRadius = 100.0f;
	
	UFUNCTION(BlueprintCallable)
	TArray<AShipPawn*> SpawnShips();

	UFUNCTION(BlueprintCallable)
	int32 GetTeam();

	UPROPERTY(EditAnywhere, Category = "Objective")
	int32 MyObjIndex;
private: 
	
	UPROPERTY(EditAnywhere, Category = "Spawned Ships")
	int32 SpawnOnTeam;
	UPROPERTY(EditAnywhere, Category = "Spawned Ships")
	bool bSpawnOnStart;

	UPROPERTY(EditAnywhere, Category = "Spawned Ships")
	bool bHasSpawned;

	UPROPERTY(EditAnywhere, Category = "Spawned Ships")
	float RepeatSpawnTime;//-1.0 for no repeat

	UPROPERTY(EditAnywhere, Category = "Spawned Ships")
	float BeginningTimeSinceLastSpawn = 0.0;

	
};
