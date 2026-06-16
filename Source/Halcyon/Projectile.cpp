// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "WeaponSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include <Kismet/GameplayStatics.h>
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

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

    Mesh->BodyInstance.bOverrideMass = true;
    Mesh->BodyInstance.SetMassOverride(0.f);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetupAttachment(Collision);

    // collision
    Collision->InitSphereRadius(12.f); // adjust size later?
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionResponseToAllChannels(ECR_Overlap); 
    //Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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
            FVector ImpactVector = this->GetActorForwardVector() * -1;
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
            EHitLayer ShieldOrHull = Ship->AllocateDamage(ImpactAngle, GetDamage(DistanceTraveled));
            FVector ProjectileLoc = GetActorLocation();
            FVector FXSpawnLoc = ProjectileLoc;

            // calculate fx spawn rotation
            FVector ShipCenter = OtherActor->GetActorLocation();
            FVector ImpactLoc = SweepResult.ImpactPoint;
            FVector ImpactNormal = SweepResult.ImpactNormal;

            FVector ProjectileDir = GetVelocity().GetSafeNormal();
            FRotator ImpactNormalRot = ImpactNormal.Rotation();
            // average of opposite projectile vector and normal vector
            FQuat NormalQuat = ImpactNormal.ToOrientationQuat();
            FQuat ProjectileQuat = (-ProjectileDir).ToOrientationQuat();
            FQuat AvgQuat = FQuat::Slerp(NormalQuat, ProjectileQuat, 0.5f);
            FRotator FXSpawnRot = AvgQuat.Rotator();
            switch (ShieldOrHull) {
                case EHitLayer::Shield:
                    // spawn shield FX
                    if (Ship->ShieldHitBPClass) {
                        float OffsetDistance = 50.f; // distance from hull
                        float LifeTime = 1.f; // how long FX plane lasts

                        //FVector ShieldFXSpawnLoc = ImpactLoc + ImpactNormal * OffsetDistance;
                        FVector ShieldFXSpawnLoc = ImpactLoc + AvgQuat.GetForwardVector() * OffsetDistance;                        FActorSpawnParameters SpawnParams;
                        SpawnParams.Owner = this;
                        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                        AActor* ShieldPlane = GetWorld()->SpawnActor<AActor>(Ship->ShieldHitBPClass, ShieldFXSpawnLoc, FXSpawnRot, SpawnParams);
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
                        FXSpawnLoc = ShieldFXSpawnLoc;
                    }
                    // shield hit audio
                    if (Ship->ShieldHitAudio) {
                        UGameplayStatics::PlaySoundAtLocation(this, Ship->ShieldHitAudio, ProjectileLoc);
                    }
                    /*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("test"));*/

                    break;
                case EHitLayer::Hull:
                    // hull hit audio
                    if (HullHitAudio) {
                        UGameplayStatics::PlaySoundAtLocation(this, HullHitAudio, ProjectileLoc);
                    }
                    // spawn hull hit fx
                    if (HullHitEffect) {
                        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                            GetWorld(),
                            HullHitEffect,
                            ProjectileLoc,
                            FXSpawnRot
                        );
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
                FXSpawnLoc,
                GetActorRotation()
            );
        }
            // Destroy projectile
            this->Destroy();
        }
        //LOGIC: ADD OBSTACLES THAT BLOCK PROJECTILES
        else if (Cast<AWeaponSystem>(OtherActor) || Cast<AProjectile>(OtherActor)) {
            return;
        }
        else {
            this->Destroy();
            return;
        }
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
            MaxRange = OwningWeapon->MaxRangeOverload;
            EnergyLevel = EnergyLevel > MaxEnergy ? MaxEnergy : EnergyLevel;
        }
        else bIsOverloaded = false;
        team = OwningWeapon->team;
    }
    /*if (AProjectile::IsHomingProjectile() && MissileSmokeSystem) {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("smoke enabled")));
        SmokeComp->SetAsset(MissileSmokeSystem);
        SmokeComp->Activate();
    }*/
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
        //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("not homing projectile"));
    }
}
void AProjectile::RemoveMissileTarget() {
    if (!Movement) return;
    Movement->HomingTargetComponent = nullptr;
    Movement->bIsHomingProjectile = false;

    FVector CurrentVel = Movement->Velocity;
    float Speed = CurrentVel.Size();

    FVector Forward = CurrentVel.GetSafeNormal();
    FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();
    // calculate random direction
    FVector RandomOffset =
        Right * FMath::FRandRange(-1.f, 1.f) +
        FVector::UpVector * FMath::FRandRange(-1.f, 1.f);
    float VeerStrength = .6f;
    FVector NewDir = (Forward + RandomOffset * VeerStrength).GetSafeNormal();
    // interp velocity to new direction in Tick()
    bVeering = true;
    DesiredVelocity = NewDir * Speed;



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
    if (bVeering && Movement) {
        Movement->Velocity = FMath::VInterpTo(
            Movement->Velocity,
            DesiredVelocity,
            DeltaTime,
            5.f
        );
    }
}

void AProjectile::FireInDirection(const FVector& ShootDirection)
{
    if (Movement)
    {
        Movement->Velocity = ShootDirection.GetSafeNormal() * Movement->InitialSpeed;
    }
}


//Default damage function; just decrease over range
int32 AProjectile::GetDamage(float Range) {
    int32 AdjustedBaseDamage = BaseDamage;
    if (DamageScaling) {
        float RangeThreshold = MaxRange / 3;
        int32 RangeBand = (int)(Range / RangeThreshold);
        /* GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
              FString::Printf(TEXT("Hit at range %f (Rangeband %d)"),
                  DistanceTraveled, RangeBand));*/
        AdjustedBaseDamage = (BaseDamage - RangeBand * (BaseDamage / DamageScaling));
    }
    if (bIsOverloaded) {
        if (EnergyStep == 0) return AdjustedBaseDamage;
        AdjustedBaseDamage += (int)(OverloadScaling * (float)AdjustedBaseDamage * (float)((EnergyLevel - MinEnergy) / EnergyStep));
    }
    return AdjustedBaseDamage;
    
}