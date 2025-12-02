// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include <Components/SphereComponent.h>
#include "ShipPawn.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"


#include "Projectile.generated.h"

class AWeaponSystem;
/*
* Generic class representing a kinetic projectile with constant speed and damage varying linearly over range.
*/
UCLASS(Blueprintable, BlueprintType)
class HALCYON_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();
	float GetProjectileSpeed() {
		return Movement->MaxSpeed;
	}
	bool IsHomingProjectile() {
		return Movement->bIsHomingProjectile;
	}
	AActor* Target;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Collision;

	UFUNCTION()
	int32 GetDamage();//returns damage amt for range

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* Movement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxRange = 3600.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float InitialSpeed = 2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxSpeed = 2000.f;

	float DistanceTraveled = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 BaseDamage = 8;//Maximum damage

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 DamageScaling = 4;//What fraction of the original damage (rounded) should be lost at each range band
	
	int32 EnergyLevel;
	int32 MaxEnergy;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* ShieldHitAudio;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* HullHitAudio;

	UPROPERTY(EditAnywhere, Category = "FX")
	UNiagaraSystem* ShieldHitEffect;

	UPROPERTY(EditAnywhere, Category = "FX")
	UNiagaraSystem* ExplosionEffect;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	int32 team = -1;

	void FireInDirection(const FVector& ProjectileDirection);
	void SetupHoming(AActor* InTarget);


};
