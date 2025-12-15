// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipSpawnPoint.h"
#include "AIController.h"

// Sets default values
AShipSpawnPoint::AShipSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);


}

// Called when the game starts or when spawned
void AShipSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnStart) {
		SpawnShips();
	}
}

// Called every frame
void AShipSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (RepeatSpawnTime > 0.0) {
		BeginningTimeSinceLastSpawn += DeltaTime;
		if (BeginningTimeSinceLastSpawn >= RepeatSpawnTime) {
			SpawnShips();
		}
	}

}

TArray<AShipPawn*> AShipSpawnPoint::SpawnShips() {
	bHasSpawned = true;

	//SPAWN
	TArray<AShipPawn*> SpawnedShips;

	for (int i = 0; i < SpawnedShipClasses.Num(); i++) {
		if (SpawnedShipClasses[i]) {
			//get random location to spawn the ship in
			FVector2D RandomCircle = FMath::RandPointInCircle(SpawnRadius);
			FVector SpawnLocation = GetActorLocation() +
				FVector(RandomCircle.X, RandomCircle.Y, 0.0f);
			FRotator SpawnRotation = GetActorRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			SpawnParams.Owner = this;

			AActor* SpawnedShip = GetWorld()->SpawnActor<AActor>(SpawnedShipClasses[i], SpawnLocation, SpawnRotation, SpawnParams);

			if (AShipPawn* SSP = Cast<AShipPawn>(SpawnedShip))
			{
				SpawnedShips.Add(SSP);

				// NEW: Spawn and attach AI controller
				if (AIControllerClass)
				{
					AAIController* AIController = GetWorld()->SpawnActor<AAIController>(
						AIControllerClass,
						SpawnLocation,
						SpawnRotation
					);

					if (AIController)
					{
						AIController->Possess(SSP);
					}
				}
				else
				{
					// No AIControllerClass set, try to use default from pawn
					SSP->SpawnDefaultController();
				}
			}

		}
	}

	if (RepeatSpawnTime > 0.0) {
		BeginningTimeSinceLastSpawn = 0.0;
	}
	return SpawnedShips;

}

int32 AShipSpawnPoint::GetTeam() {
	return SpawnOnTeam;
}



