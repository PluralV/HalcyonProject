// Fill out your copyright notice in the Description page of Project Settings.


#include "HalcyonSimpleGameMode.h"
#include "ShipPawn.h"
#include "ShipPlayerController.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"


void AHalcyonSimpleGameMode::StartPlay() {
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
    GEngine->AddOnScreenDebugMessage(-1, 5.f,FColor::Red,FString::Printf(TEXT("%d Enemies Have Been Registered"),EnemiesRemaining));
}

void AHalcyonSimpleGameMode::DecrementEnemies() {
	EnemiesRemaining--;
	CheckEnemies();
}

void AHalcyonSimpleGameMode::CauseLoss() {
    OnLoss.Broadcast();
}


void AHalcyonSimpleGameMode::IncrementEnemies() {
	EnemiesRemaining++;
}

void AHalcyonSimpleGameMode::CheckEnemies() {
	if (EnemiesRemaining <= 0) {
		OnVVin.Broadcast();
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("You VVon")));
	}
}