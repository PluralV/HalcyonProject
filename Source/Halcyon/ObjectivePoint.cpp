// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectivePoint.h"
#include "ShipPlayerController.h"
#include "Components/SphereComponent.h"
#include "ShipPawn.h"

// Sets default values
AObjectivePoint::AObjectivePoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));


	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	RootComponent = TriggerSphere;

	Mesh->SetupAttachment(TriggerSphere);

	TriggerSphere->InitSphereRadius(InteractionDistance); // adjust size later?

	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerSphere->SetNotifyRigidBodyCollision(true);

	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AObjectivePoint::OnInteract);

	//Place widget marker on the screen
	MapMarker = CreateDefaultSubobject<UWidgetComponent>(TEXT("MapMarker"));
	MapMarker->SetupAttachment(TriggerSphere);
	MapMarker->SetWidgetSpace(EWidgetSpace::Screen);
	MapMarker->SetDrawSize(FVector2D(45.0f, 45.0f));
	MapMarker->SetPivot(FVector2D(0.5f, 0.5f));
}

// Called when the game starts or when spawned
void AObjectivePoint::BeginPlay()
{
	Super::BeginPlay();
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("BP: Binding objective interaction to distance %d"),(int32)InteractionDistance));
	TriggerSphere->SetSphereRadius(InteractionDistance);

	if (MapMarkerClass)
	{
		MapMarker->SetWidgetClass(MapMarkerClass);
		MapMarker->SetVisibility(bIsActive);
	}
}

void AObjectivePoint::OnInteract(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult) {
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("OnInteract called!")));
	if (bWasTriggered || !bIsActive) return;

	if (AShipPawn* ASP = Cast<AShipPawn>(OtherActor)) {
		if (AShipPlayerController* SPC = Cast<AShipPlayerController>(ASP->Controller)) {
			bWasTriggered = true;
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("INTERACTING WITH OBJECTIVE %d!"), MyObjectiveIndex));
			OnMarkerReached.Broadcast(MyObjectiveIndex);
		}
		return;
	}

}

void AObjectivePoint::SetActiveObjective(bool IsActive) {
	bIsActive = IsActive;
	MapMarker->SetVisibility(bIsActive);
}

int32 AObjectivePoint::GetMyObjIndex() {
	return MyObjectiveIndex;
}