// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "WeaponSystem.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AProjectile::AProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

    // movement
    Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
    Movement->InitialSpeed = InitialSpeed;
    Movement->MaxSpeed = MaxSpeed;
    Movement->bRotationFollowsVelocity = true;
    Movement->bShouldBounce = false;
    Movement->ProjectileGravityScale = 0.f;

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    RootComponent = Collision;

    Mesh->SetMassOverrideInKg(NAME_None, 0.f, true);
    Mesh->SetupAttachment(Collision);

    // collision
    Collision->InitSphereRadius(12.f); // adjust size later?
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionResponseToAllChannels(ECR_Overlap); 
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetNotifyRigidBodyCollision(true);

    Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Collision->SetGenerateOverlapEvents(true);
    Collision->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnOverlapBegin);

}

UFUNCTION()
void AProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
        FString::Printf(TEXT("Hit Actor: %s"), OtherActor->GetName()));*/
    if (OtherActor && OtherActor != this->GetOwner())
    {
        /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
            FString::Printf(TEXT("Collision detected:")));*/
        if (AShipPawn* Ship = Cast<AShipPawn>(OtherActor)) {

            if (Ship->Team == team)
            {
               /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
                    FString::Printf(TEXT("SAME TEAM!!!!")));*/
                return;
            }
            // Apply damage or effects here
            FVector TargetVector = Ship->GetActorRightVector();
            FVector ImpactVector = this->GetActorForwardVector()*-1;
            FQuat ImpactQuat = FQuat::FindBetweenNormals(TargetVector, ImpactVector);
            FRotator ImpactRotator = ImpactQuat.Rotator();
            float ImpactAngle = ImpactRotator.Yaw + 30.f;
            if (ImpactAngle < 0) ImpactAngle += 360.f;
            /*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
                FString::Printf(TEXT("Target Vector: %f %f %f; ImpactVector: %f %f %f; Hit Angle: %f"), 
                    TargetVector.X, TargetVector.Y, TargetVector.Z,
                    ImpactVector.X, ImpactVector.Y, ImpactVector.Z,
                    ImpactAngle));*/
            Ship->AllocateDamage(ImpactAngle, GetDamage());

            // Destroy projectile
            this->Destroy();
        }
        //LOGIC: ADD OBSTACLES THAT BLOCK PROJECTILES
        /*else {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,FString::Printf(TEXT("ACKK!!!!!")));
            this->Destroy();
            return;
        }*/
    }
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
    if (AWeaponSystem* OwningWeapon = Cast<AWeaponSystem>(Owner)) {
        BaseDamage = OwningWeapon->BaseDamage;
        DamageScaling = OwningWeapon->DamageScaling;
        EnergyLevel = OwningWeapon->AllocatedEnergy;
        MaxRange = OwningWeapon->MaxRange;
        MaxEnergy = OwningWeapon->MaxEnergy;
        MinEnergy = OwningWeapon->MinEnergy;
        EnergyStep = OwningWeapon->EnergyStep;
        OverloadScaling = OwningWeapon->OverloadScaling;
        if (EnergyLevel > MinEnergy) {
            
            bIsOverloaded = true;
            EnergyLevel = EnergyLevel > MaxEnergy ? MaxEnergy : EnergyLevel;
        }
        else bIsOverloaded = false;
        team = OwningWeapon->team;
    }
}
void AProjectile::SetupHoming(AActor* InTarget)
{
    Target = InTarget;

    if (Movement->bIsHomingProjectile && Target)
    {
        Movement->HomingTargetComponent = Target->GetRootComponent();
    }
    else {
        //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("not homing projectile"));
    }
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    //Tracks how far the projectile has moved and destroys it if it exceeds maximum range
    DistanceTraveled += ((float)(Movement->Velocity.Length()) * DeltaTime);
    if (DistanceTraveled > MaxRange) this->Destroy();

}

void AProjectile::FireInDirection(const FVector& ShootDirection)
{
    if (Movement)
    {
        Movement->Velocity = ShootDirection.GetSafeNormal() * Movement->InitialSpeed;
    }
}


//Default damage function; just decrease over range
int32 AProjectile::GetDamage() {
    int32 AdjustedBaseDamage = BaseDamage;
    if (DamageScaling) {
        float RangeThreshold = MaxRange / 3;
        int32 RangeBand = (int)(DistanceTraveled / RangeThreshold);
        /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
              FString::Printf(TEXT("Hit at range %f (Rangeband %d)"),
                  DistanceTraveled, RangeBand));*/
        AdjustedBaseDamage = (BaseDamage - RangeBand * (BaseDamage / DamageScaling));
    }
    if (bIsOverloaded) {
        AdjustedBaseDamage += (int)(OverloadScaling * (float)AdjustedBaseDamage * (float)((EnergyLevel - MinEnergy) / EnergyStep));
    }
    return AdjustedBaseDamage;
    
}