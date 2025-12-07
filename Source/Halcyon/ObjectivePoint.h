// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "ObjectivePoint.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMarkerReached, int32, ObjectiveIndexReached);

UCLASS(Blueprintable, BlueprintType)
class HALCYON_API AObjectivePoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectivePoint();

	int32 GetMyObjIndex();

	void SetActiveObjective(bool IsActive);

	UPROPERTY(BlueprintReadOnly, Category = "Events")
	FOnMarkerReached OnMarkerReached;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void OnInteract(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget Component")
	UWidgetComponent* MapMarker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget Component")
	TSubclassOf<UUserWidget> MapMarkerClass;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* Mesh;

	UPROPERTY(BlueprintReadOnly, Category = "Collision")
	USphereComponent* TriggerSphere;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective Point Sorting")
	float InteractionDistance = 500.f;

	//Some sort of index to denote what it does - ideas - simple capture point/waypoint, repair station, etc
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective Point Sorting")
	int32 ObjectiveMarkType;
	//Index to denote its position in the list of objectives
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective Point Sorting")
	int32 MyObjectiveIndex;

	

	

private:
	bool bWasTriggered = false;
	bool bIsActive = false;

};
