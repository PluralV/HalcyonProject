// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "WeaponSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include <Kismet/GameplayStatics.h>


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

    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);

    Collision->SetCollisionResponseToAllChannels(ECR_Overlap);
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
    if (OtherActor && OtherActor != this->GetOwner() && OtherActor->IsA(AShipPawn::StaticClass()))
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
            // Play audio depending on whether shield or hull is hit
            EHitLayer ShieldOrHull = Ship->AllocateDamage(ImpactAngle, GetDamage());
            FVector ImpactLocation = GetActorLocation();
            switch (ShieldOrHull) {
                case EHitLayer::Shield:
                    // spawn effect
                    if (ShieldHitEffect) {
                        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                            GetWorld(),
                            ShieldHitEffect,
                            ImpactLocation,
                            GetActorRotation()  // or use an impact normal if you have one
                        );
                    }
                    // audio
                    UGameplayStatics::PlaySoundAtLocation(this, ShieldHitAudio, ImpactLocation);
                    break;
                case EHitLayer::Hull:
                    // audio
                    UGameplayStatics::PlaySoundAtLocation(this, HullHitAudio, ImpactLocation);
                    break;
                default:
                    break;
            }
            // spawn explosion effect if exists
            if (ExplosionEffect) {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                ExplosionEffect,
                ImpactLocation,
                GetActorRotation()
            );
        }
            // Destroy projectile
            this->Destroy();
        }
    }
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

    if (AWeaponSystem* OwningWeapon = Cast<AWeaponSystem>(Owner)) {
        MaxRange = OwningWeapon->MaxRange;
        EnergyLevel = OwningWeapon->AllocatedEnergy;
        MaxEnergy = OwningWeapon->MaxEnergy;
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
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("not homing projectile"));
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
    if (!DamageScaling) return BaseDamage;
    float RangeThreshold = MaxRange / 3;
    int32 RangeBand = (int)(DistanceTraveled / RangeThreshold);
  /*  GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
        FString::Printf(TEXT("Hit at range %f (Rangeband %d)"),
            DistanceTraveled, RangeBand));*/
    return (BaseDamage - RangeBand * (BaseDamage / DamageScaling));
}