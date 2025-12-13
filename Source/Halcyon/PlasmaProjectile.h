// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "PlasmaProjectile.generated.h"

USTRUCT(BlueprintType)
struct FRangePair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RangeThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DamageAtRange;
};

/**
 * 
 */
UCLASS()
class HALCYON_API APlasmaProjectile : public AProjectile
{
	GENERATED_BODY()
public:
	APlasmaProjectile();
	virtual void BeginPlay() override;

	virtual int32 GetDamage(float Range) override;
protected:
	//2D Array of pairs of floats; the first denotes a range threshold, the second denotes damage dealt by the projectile within that range-band
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	TArray<FRangePair> RangeThresholds;
};
