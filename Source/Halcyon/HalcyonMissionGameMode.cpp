// Fill out your copyright notice in the Description page of Project Settings.


#include "HalcyonMissionGameMode.h"
#include "ShipPawn.h"
#include "ShipPlayerController.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

void AHalcyonMissionGameMode::StartPlay() {
	Super::StartPlay();
	check(GEngine != nullptr);
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShipPawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (AShipPawn* Ship = Cast<AShipPawn>(Actor))
        {
            if (AShipPlayerController* SPC = Cast<AShipPlayerController>(Ship->Controller)) {
                continue;
            }
            else {
                IncrementEnemies();
            }
        }
    }


}

void AHalcyonMissionGameMode::DecrementEnemies() {
    EnemiesRemaining--;
    CheckEnemies();
}

void AHalcyonMissionGameMode::CauseLoss() {
    OnMissionLoss.Broadcast();
}


void AHalcyonMissionGameMode::IncrementEnemies() {
    EnemiesRemaining++;
}

void AHalcyonMissionGameMode::CheckEnemies() {
    if (EnemiesRemaining <= 0) {
        OnMissionWin.Broadcast();
    }
}