// Fill out your copyright notice in the Description page of Project Settings.


#include "HalcyonSimpleGameMode.h"

void AHalcyonSimpleGameMode::StartPlay() {
	Super::StartPlay();

	check(GEngine != nullptr);
}

void AHalcyonSimpleGameMode::DecrementEnemies() {
	EnemiesRemaining--;
}


void AHalcyonSimpleGameMode::IncrementEnemies() {
	EnemiesRemaining++;
}
