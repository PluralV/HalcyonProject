// Fill out your copyright notice in the Description page of Project Settings.


#include "HalcyonMissionGameMode.h"
#include "ShipPawn.h"
#include "ShipPlayerController.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

void AHalcyonMissionGameMode::StartPlay() {
	Super::StartPlay();
	check(GEngine != nullptr);

    ObjectiveCount = ObjectiveList.Num();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShipPawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (AShipPawn* Ship = Cast<AShipPawn>(Actor))
        {
            if (Ship->Team == 1){
                IncrementEnemies(1);
            }
        }
    }

    //Register all objective markers to their corresponding index in the objective list
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AObjectivePoint::StaticClass(), FoundActors);

    //For each ObjectivePoint, bind it to the Objective at the index MyObjectiveIndex in the list, and then bind its completion to
    //the completion function
    for (AActor* Actor : FoundActors) {
        if (AObjectivePoint* AOP = Cast<AObjectivePoint>(Actor)) {
            int32 ObjIndex = AOP->GetMyObjIndex();
            //GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("Associating objective %d with point %s!"),ObjIndex, *AOP->GetName()));
            ObjectiveList[ObjIndex].AssociatedObjective = AOP;
            AOP->OnMarkerReached.AddDynamic(this, &AHalcyonMissionGameMode::HandleObjectiveCompletion);
        }
    }

    //Register all spawnpoints to their corresponding index in the objective list
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShipSpawnPoint::StaticClass(), FoundActors);

    //For each ShipSpawnPoint, if it is bound to an objective, bind it to that objective (will spawn when that objective is set to active)
    for (AActor* Actor : FoundActors) {
        if (AShipSpawnPoint* ASSP = Cast<AShipSpawnPoint>(Actor)) {
            if (ASSP->MyObjIndex < 0) continue;
            int32 ObjIndex = ASSP->MyObjIndex;
            ObjectiveList[ObjIndex].AssociatedSpawners.Add(ASSP);
        }
    }

    //Initialize all objective map widgets: only the first one should have a visible marker
    for (int32 i = 1; i < ObjectiveCount; i++) {
        switch (ObjectiveList[i].ObjectiveType) {
        case 0:
            if (ObjectiveList[i].AssociatedObjective) {
                ObjectiveList[i].AssociatedObjective->SetActiveObjective(false);
            }
            break;
        case 1: //TODO: add switch logic for other obj types - should KILL missions have widgets over the target ships? (probably?)
            break;
        case 2:
            break;
        default:break;
        }
        
        
    }

    //Set the first one to active
    CurrentObjective = ObjectiveList[0];
    CurrentObjectiveIndex = 0;
    ActivateCurrentObjective();

}

void AHalcyonMissionGameMode::DecrementEnemies() {
    EnemiesRemaining--;
    CheckEnemies();
}

void AHalcyonMissionGameMode::CauseLoss() {
    OnMissionLoss.Broadcast();
}


void AHalcyonMissionGameMode::IncrementEnemies(int32 ByAmt) {
    EnemiesRemaining+=ByAmt;
}

void AHalcyonMissionGameMode::CheckEnemies() {
    if (EnemiesRemaining <= 0 && CurrentObjective.ObjectiveType == 1) {
        HandleObjectiveCompletion(CurrentObjectiveIndex);
    }
}


void AHalcyonMissionGameMode::HandleObjectiveCompletion(int32 ObjectiveIndex) {
    if (ObjectiveIndex != CurrentObjectiveIndex) return;
    OnCurrentObjectiveComplete.Broadcast();
    if (CurrentObjective.ObjectiveType == 0) CurrentObjective.AssociatedObjective->SetActiveObjective(false);
    CurrentObjectiveIndex++;

    if (CurrentObjectiveIndex >= ObjectiveCount) {
        //GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("You vvon n shid")));
        OnMissionWin.Broadcast();
    }
    else ActivateCurrentObjective();
}

void AHalcyonMissionGameMode::ActivateCurrentObjective() {
    CurrentObjective = ObjectiveList[CurrentObjectiveIndex];
    OnCurrentObjectiveSet.Broadcast(CurrentObjective);
    if (CurrentObjective.AssociatedObjective) {
        CurrentObjective.AssociatedObjective->SetActiveObjective(true);
    }
    //IF this is a kill-objective (should have associated enemies), then spawn the enemies and register them
    if (CurrentObjective.ObjectiveType == 1) {
        if (!CurrentObjective.AssociatedSpawners.IsEmpty()) {
            for (AShipSpawnPoint* Spawner : CurrentObjective.AssociatedSpawners) {
                //Spawn all the ships from each associated spawn point for this objective
                TArray<AShipPawn*> Spawned = Spawner->SpawnShips();
                if (Spawner->GetTeam() == 1) {//If the ships are enemies, increment the enemy count for the mission
                    IncrementEnemies(Spawned.Num());
                }
            }
            //objective is done
            return;
        }
        //If it was empty for some reason, just complete it
        HandleObjectiveCompletion(CurrentObjectiveIndex);
    }
    return;
}

FObjectiveInfo AHalcyonMissionGameMode::GetCurrentObjective() {
    return CurrentObjective;
}