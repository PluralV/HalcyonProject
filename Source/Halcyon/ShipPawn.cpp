// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipPawn.h"
#include "ShipPawnMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ShipPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "WeaponSystem.h"

// Sets default values
AShipPawn::AShipPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMesh;

	// Create movement component
	MovementComponent = CreateDefaultSubobject<UShipPawnMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->SetUpdatedComponent(ShipMesh);
	MovementComponent->SpeedLimit = SpeedLimit;
	MovementComponent->PitchRate = PitchRate;
	MovementComponent->YawRate = YawRate;
	MovementComponent->SpeedConstant = SpeedConstant;
	MovementComponent->AccelRate = AccelRate;

	// Set up mesh
	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->SetCollisionProfileName(TEXT("Vehicle"));
	ShipMesh->SetEnableGravity(false);

	//Body mesh
	HullMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	HullMesh->SetupAttachment(ShipMesh);
	HullMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Build spring-arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 500.0f;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraLagSpeed = 3.0f;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->CameraRotationLagSpeed = 5.0f;
	//could change if necessary
	SpringArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

	//Create camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	

	//Initialize stats
	LeftEngCurr = LeftEng;
	RightEngCurr = RightEng;
	CenterEngCurr = CenterEng;
	PowerReactorCurr = PowerReactor;

	ForwardHullCurr = ForwardHull;
	AftHullCurr = AftHull;
	CenterHullCurr = CenterHull;

	TotalEnergy = LeftEng + RightEng + CenterEng + PowerReactor;
	TotalEnergyCurr = TotalEnergy;
	TotalEnergyAvailable = TotalEnergy;

	for (int8 i = 0; i < 6; i++) {
		ShieldFacingsCurr[i] = ShieldFacings[i];
	}
}

// Called when the game starts or when spawned
void AShipPawn::BeginPlay()
{
	Super::BeginPlay();
	

	// Add the Input Mapping Context to the player's input subsystem
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// THIS is where the VehicleMappingContext property is USED
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Adding ship mapping context"));
			Subsystem->AddMappingContext(ShipMappingContext, 0);
		}
	}

	// add the weapon systems placed in bp to the array
	TArray<UChildActorComponent*> ChildActorComps;
	GetComponents<UChildActorComponent>(ChildActorComps);
	for (UChildActorComponent* Comp : ChildActorComps)
	{
		if (Comp && Comp->GetChildActor() && Comp->GetChildActor()->IsA(AWeaponSystem::StaticClass()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Adding weaponsystem to array"));
			WeaponComponents.Add(Comp);
		}
	}
	
}



// Called every frame
void AShipPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CurrentTarget) {
		for (UChildActorComponent* WeaponComp : WeaponComponents)
		{
			if (WeaponComp) {
				AActor* Child = WeaponComp->GetChildActor();
				if (AWeaponSystem* Weapon = Cast<AWeaponSystem>(Child))
				{
					Weapon->TrackTarget(DeltaTime, CurrentTarget);
				}
				else {
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("child actor not a weapon"));

				}
			}
			else {
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("no wewpaoncomp"));
			}
		}
	}
}

// Called to bind functionality to input
void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Binding inputs"));
		//throttling
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Started, this, &AShipPawn::Throttle);
		//EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Ongoing, this, &AShipPawn::Move);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AShipPawn::ZeroThrottle);
		//braking/reversing
		EnhancedInputComponent->BindAction(DecelerateAction, ETriggerEvent::Started, this, &AShipPawn::Decelerate);
		//EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Ongoing, this, &AShipPawn::Brake);
		EnhancedInputComponent->BindAction(DecelerateAction, ETriggerEvent::Completed, this, &AShipPawn::ZeroDecel);
		//steering
		EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Started, this, &AShipPawn::Steer);
		//EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Ongoing, this, &AShipPawn::Steer);
		EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Completed, this, &AShipPawn::Steer);
		//Look action
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShipPawn::Look);
		//Target action
		EnhancedInputComponent->BindAction(TargetAction, ETriggerEvent::Completed, this, &AShipPawn::Target);
		//Movement alloc/free of energy with keys
		EnhancedInputComponent->BindAction(AllocMovementAction, ETriggerEvent::Triggered, this, &AShipPawn::HandleArrowAlloc);

		EnhancedInputComponent->BindAction(FreeMovementAction, ETriggerEvent::Triggered, this, &AShipPawn::HandleArrowFree);

	}
}

//PLAYER INPUT HANDLER FUNCTIONS
/*
TODO: For movement component, create a new function to set thrust/rotation rather than simply adding/subtracting
*/
void AShipPawn::ZeroThrottle() {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Zeroing throttle"));
	CurrentThrottle = 0.f;
	if (AShipPlayerController* PC = Cast<AShipPlayerController>(GetController()))
	{
		MovementComponent->SetThrustInput(0.f);
	}
}

void AShipPawn::ZeroDecel() {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Zeroing brake"));
	if (AShipPlayerController* PC = Cast<AShipPlayerController>(GetController()))
	{
		MovementComponent->SetThrustInput(0.f);
	}
}

void AShipPawn::ZeroSteering() {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Zeroing steering"));
	if (AShipPlayerController* PC = Cast<AShipPlayerController>(GetController()))
	{
		//TODO: ADD NEW MOVEMENT COMPONENT FUNCTION TO 
		MovementComponent->AddRotationalInput(FVector(0.0,0.0,0.0));
	}
}

void AShipPawn::Look(const FInputActionValue& Value) {
	const FVector2D LookValue = Value.Get<FVector2D>();
	/*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("LOOK called"));*/
	if (SpringArm) {
		FRotator CurrentRotator = SpringArm->GetRelativeRotation();
		CurrentRotator.Yaw += LookValue.X * CameraRotationSpeed;
		CurrentRotator.Pitch = FMath::Clamp(CurrentRotator.Pitch + (LookValue.Y * CameraRotationSpeed), -360,360);//FMath::Clamp(CurrentRotator.Pitch + (LookValue.Y * CameraRotationSpeed), -80.0f, 0.0f);
		SpringArm->SetRelativeRotation(CurrentRotator);
	}
}


void AShipPawn::Throttle(const FInputActionValue& Value) {
	/*const FVector2D MoveValue = Value.Get<FVector2D>();
	float AppliedThrottle = MoveValue.Y;*/
	bool bPressed = Value.Get<bool>();
	float AppliedThrottle = bPressed ? 1.f : 0.f;
	CurrentThrottle = AppliedThrottle;
	
	if (AShipPlayerController* PC = Cast<AShipPlayerController>(GetController()))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Throttling"));
		MovementComponent->SetThrustInput(AppliedThrottle);
	}
}

void AShipPawn::Decelerate(const FInputActionValue& Value) {
	const FVector2D MoveValue = Value.Get<FVector2D>();
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Braking"));
	bool bPressed = Value.Get<bool>();
	float AppliedThrottle = bPressed ? -1.f : 0.f;
	if (AShipPlayerController* PC = Cast<AShipPlayerController>(GetController()))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Throttling"));
		MovementComponent->SetThrustInput(AppliedThrottle);
	}
}

void AShipPawn::Steer(const FInputActionValue& Value) {
	const FVector2D MoveValue = Value.Get<FVector2D>();
	
	if (AShipPlayerController* PC = Cast<AShipPlayerController>(GetController()))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Turning"));
		MovementComponent->SetRotationalInput(FRotator(MoveValue.X, MoveValue.Y, 0));
	}
}

void AShipPawn::Target(const FInputActionValue& Value) {
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Targeting"));
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	// Get screen center
	int32 ViewX, ViewY;
	PC->GetViewportSize(ViewX, ViewY);
	FVector2D ScreenCenter(ViewX / 2.0f, ViewY / 2.0f);

	// Find all enemies in world, loop after projecting to screen to find closest enemy to center
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShipPawn::StaticClass(), Enemies);

	float ClosestDist = TNumericLimits<float>::Max();
	AActor* ClosestEnemy = nullptr;

	for (AActor* Enemy : Enemies)
	{
		if (!Enemy || Enemy == this) continue;

		FVector2D ScreenLoc;
		bool bOnScreen = PC->ProjectWorldLocationToScreen(Enemy->GetActorLocation(), ScreenLoc);

		if (bOnScreen)
		{
			float Dist = FVector2D::Distance(ScreenLoc, ScreenCenter);
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				ClosestEnemy = Enemy;
			}
		}
	}
	if (ClosestEnemy)
	{
		CurrentTarget = ClosestEnemy;
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
			FString::Printf(TEXT("Target locked: %s"), *ClosestEnemy->GetName()));
	}
	else
	{
		CurrentTarget = nullptr;
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
			FString::Printf(TEXT("No target found")));
	}
}

//Power Allocation Functions
void AShipPawn::AllocateReinforceShield(int32 amt, int32 index) {
	if (TotalEnergyAvailable >= amt) {
		//Reduce total available energy by amt, then increase shield reinforcement in index by amt
		TotalEnergyAvailable -= amt;
		ShieldReinforcements[index] += amt;
	}
	else {
		//Otherwise just use all remaining energy on reinforcing the shield
		int32 temp = TotalEnergyAvailable;
		TotalEnergyAvailable = 0;
		ShieldReinforcements[index] += temp;
	}
	OnAvailableEnergyChanged.Broadcast(TotalEnergyAvailable);
}

void AShipPawn::FreeReinforceShield(int32 amt, int32 index) {
	if (MovementEnergy >= amt) {
		//Reduce movement energy by amt, then increase available energy by amt
		ShieldReinforcements[index] -= amt;
		TotalEnergyAvailable += amt;
	}
	else {
		//Otherwise just remove all energy from movement and push to available
		int32 temp = ShieldReinforcements[index];
		ShieldReinforcements[index] = 0;
		TotalEnergyAvailable += temp;
	}
	OnAvailableEnergyChanged.Broadcast(TotalEnergyAvailable);
}

//handlers for input
void AShipPawn::HandleArrowAlloc() {
	AllocateMovement(1);
}

void AShipPawn::HandleArrowFree() {
	FreeMovement(1);
}

void AShipPawn::AllocateMovement(int32 amt) {
	if (TotalEnergyAvailable >= amt) {
		//Reduce total available energy by amt, then increase movement energy by amt
		TotalEnergyAvailable -= amt;
		MovementEnergy += amt;
	}
	else {
		//Otherwise just use all remaining energy on movement
		int32 temp = TotalEnergyAvailable;
		TotalEnergyAvailable = 0;
		MovementEnergy += temp;
	}
	OnAvailableEnergyChanged.Broadcast(TotalEnergyAvailable);
	//Pass final movement energy down to the movement component to determine max speed
	if (UShipPawnMovementComponent* MC = Cast<UShipPawnMovementComponent>(MovementComponent)) {
		MC->SetMovementEnergy(MovementEnergy);
		OnMovementEnergyChanged.Broadcast(MovementEnergy);
	}
}

void AShipPawn::FreeMovement(int32 amt) {
	if (MovementEnergy >= amt) {
		//Reduce movement energy by amt, then increase available energy by amt
		TotalEnergyAvailable += amt;
		MovementEnergy -= amt;
	}
	else {
		//Otherwise just remove all energy from movement and push to available
		int32 temp = MovementEnergy;
		MovementEnergy = 0;
		TotalEnergyAvailable += temp;
		
	}
	OnAvailableEnergyChanged.Broadcast(TotalEnergyAvailable);
	//Pass final movement energy down to the movement component to determine max speed
	if (UShipPawnMovementComponent* MC = Cast<UShipPawnMovementComponent>(MovementComponent)) {
		MC->SetMovementEnergy(MovementEnergy);
		OnMovementEnergyChanged.Broadcast(MovementEnergy);
	}
}

void AShipPawn::AllocateModularSystem(AModularSystem* TargetSystem, int32 amt) {

}

void AShipPawn::FreeModularSystem(AModularSystem* TargetSystem, int32 amt) {

}

//GETTERS FOR STATS

int32 AShipPawn::GetMovementEnergy() {
	return MovementEnergy;
}

int32 AShipPawn::GetMaxEnergy() {
	return TotalEnergy;
}

int32 AShipPawn::GetMaxEnergyAvailable() {
	return TotalEnergyAvailable;
}

int32 AShipPawn::GetMaxEnergyCurr() {
	return TotalEnergyCurr;
}

int32 AShipPawn::GetShieldFacing(int32 index) {
	return ShieldFacings[index];
}

int32 AShipPawn::GetCurrentShieldFacing(int32 index) {
	return ShieldFacingsCurr[index];
}

int32 AShipPawn::GetCurrentShieldReinforcement(int32 index) {
	return ShieldReinforcements[index];
}

int32 AShipPawn::GetLeftEngMax() {
	return LeftEng;
}

int32 AShipPawn::GetLeftEngCurr() {
	return LeftEngCurr;
}

int32 AShipPawn::GetRightEngMax() {
	return RightEng;
}

int32 AShipPawn::GetRightEngCurr() {
	return RightEngCurr;
}

int32 AShipPawn::GetCentEngMax() {
	return CenterEng;
}

int32 AShipPawn::GetCentEngCurr() {
	return CenterEngCurr;
}

int32 AShipPawn::GetReactorMax() {
	return PowerReactor;
}

int32 AShipPawn::GetReactorCurr() {
	return PowerReactorCurr;
}

int32 AShipPawn::GetForwardHullMax() {
	return ForwardHull;
}

int32 AShipPawn::GetForwardHullCurr() {
	return ForwardHullCurr;
}

int32 AShipPawn::GetAftHullMax() {
	return AftHull;
}

int32 AShipPawn::GetAftHullCurr() {
	return AftHullCurr;
}

float AShipPawn::GetSpeedConstant() {
	return SpeedConstant;
}

float AShipPawn::GetCurrentVelocity(bool bForDisplay) {
	if (UShipPawnMovementComponent* MC = Cast<UShipPawnMovementComponent>(MovementComponent)) {
		float Velocity = MC->GetSpeed();
		return bForDisplay ? roundf(Velocity * 100) / 100.f : Velocity;
	}
	return -1.f;
}