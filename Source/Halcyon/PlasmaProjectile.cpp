// Fill out your copyright notice in the Description page of Project Settings.


#include "PlasmaProjectile.h"

APlasmaProjectile::APlasmaProjectile() {

}

void APlasmaProjectile::BeginPlay() {
	Super::BeginPlay();

}

int32 APlasmaProjectile::GetDamage() {
	int32 AdjustedBaseDamage = BaseDamage;
	if (!RangeThresholds.IsEmpty()) {
		for (int32 i = 0; i < RangeThresholds.Num(); i++) {
			if (DistanceTraveled <= RangeThresholds[i].RangeThreshold) {
				AdjustedBaseDamage = (int)RangeThresholds[i].DamageAtRange;
				break;
			}
		}
	}
	if (bIsOverloaded) {
		if (EnergyStep == 0) return AdjustedBaseDamage;
		AdjustedBaseDamage += (int)(OverloadScaling * (float)AdjustedBaseDamage * (float)((EnergyLevel - MinEnergy) / EnergyStep));
	}
	return AdjustedBaseDamage;
}