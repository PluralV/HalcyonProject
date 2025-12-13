// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "WeaponSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include <Kismet/GameplayStatics.h>
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"


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
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetupAttachment(Collision);

    // collision
    Collision->InitSphereRadius(12.f); // adjust size later?
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionResponseToAllChannels(ECR_Overlap); 
    //Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetNotifyRigidBodyCollision(true);

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
            FVector ProjectileLoc = GetActorLocation();
            FVector ExplosionSpawnLoc = ProjectileLoc;
            switch (ShieldOrHull) {
                case EHitLayer::Shield:
                    // spawn projectile effect
                    if (ShieldHitEffect) {
                        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                            GetWorld(),
                            ShieldHitEffect,
                            ProjectileLoc,
                            GetActorRotation()  // or use an impact normal if you have one
                        );
                    }
                    // spawn shield FX
                    if (Ship->ShieldHitBPClass) {
                        float OffsetDistance = 50.f; // distance from hull
                        float LifeTime = 1.f; // how long FX plane lasts
                        FVector ShipCenter = OtherActor->GetActorLocation();
                        FVector ImpactLoc = SweepResult.ImpactPoint;
                        FVector ImpactNormal = SweepResult.ImpactNormal;
                        
                        FVector ProjectileDir = GetVelocity().GetSafeNormal();
                        FRotator ImpactNormalRot = ImpactNormal.Rotation();
                        // average of opposite projectile vector and normal vector
                        FQuat NormalQuat = ImpactNormal.ToOrientationQuat();
                        FQuat ProjectileQuat = (-ProjectileDir).ToOrientationQuat();
                        FQuat AvgQuat = FQuat::Slerp(NormalQuat, ProjectileQuat, 0.5f);
                        FRotator ShieldFXSpawnRot = AvgQuat.Rotator();

                        //FVector ShieldFXSpawnLoc = ImpactLoc + ImpactNormal * OffsetDistance;
                        FVector ShieldFXSpawnLoc = ImpactLoc + AvgQuat.GetForwardVector() * OffsetDistance;                        FActorSpawnParameters SpawnParams;
                        SpawnParams.Owner = this;
                        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                        AActor* ShieldPlane = GetWorld()->SpawnActor<AActor>(Ship->ShieldHitBPClass, ShieldFXSpawnLoc, ShieldFXSpawnRot, SpawnParams);
                        // attach to ship so it moves with it
                        if (ShieldPlane)
                        {
                            ShieldPlane->AttachToActor(OtherActor, FAttachmentTransformRules::KeepWorldTransform);

                            // destroy after LifeTime
                            FTimerHandle TimerHandle;
                            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [ShieldPlane]() {
                                    if (ShieldPlane)
                                    {
                                        ShieldPlane->Destroy();
                                    }
                                }, LifeTime, false); // lifespan in seconds
                        }
                        ExplosionSpawnLoc = ShieldFXSpawnLoc;
                    }
                    // shield hit audio
                    if (ShieldHitAudio) {
                        UGameplayStatics::PlaySoundAtLocation(this, ShieldHitAudio, ProjectileLoc);
                    }
                    /*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("test"));*/

                    // shield spawn audio
                    /*if (ShieldSpawnAudio) {
                        UGameplayStatics::PlaySoundAtLocation(this, ShieldSpawnAudio, ProjectileLoc);
                    }*/
                    break;
                case EHitLayer::Hull:
                    // hull hit audio
                    if (HullHitAudio) {
                        UGameplayStatics::PlaySoundAtLocation(this, HullHitAudio, ProjectileLoc);
                    }
                    break;
                default:
                    break;
            }
            // spawn explosion effect if exists
            if (ExplosionEffect) {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                ExplosionEffect,
                ExplosionSpawnLoc,
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
        // set target
        Movement->HomingTargetComponent = Target->GetRootComponent();
        // add to array of missiles currently locked on ship
        if (AShipPawn* ShipPawn = Cast<AShipPawn>(Target))
        {
            ShipPawn->AddIncomingMissile(this);
        }
    }
    else {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("not homing projectile"));
    }
}

void AProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AShipPawn* Ship = Cast<AShipPawn>(Target))
    {
        Ship->RemoveIncomingMissile(this);
    }

    Super::EndPlay(EndPlayReason);
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