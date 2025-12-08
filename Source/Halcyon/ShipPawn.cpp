// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipPawn.h"
#include "ShipPawnMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ShipPlayerController.h"
#include "ShipAIController.h"
#include "HalcyonSimpleGameMode.h"
#include "HalcyonMissionGameMode.h"
#include "ShipSpawnPoint.h"

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

	


	// Set up mesh
	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->SetCollisionProfileName(TEXT("Vehicle"));
	ShipMesh->SetEnableGravity(false);

	// mesh collision
	ShipMesh->SetCollisionObjectType(ECC_Pawn);
	ShipMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ShipMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	ShipMesh->SetGenerateOverlapEvents(true);

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
	
	// Automatically spawn and possess with AI controller
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Specify which AI controller class to use
	AIControllerClass = AShipAIController::StaticClass();
	
	// Create movement component
	MovementComponent = CreateDefaultSubobject<UShipPawnMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->SetUpdatedComponent(ShipMesh);
}

// Called when the game starts or when spawned
void AShipPawn::BeginPlay()
{
	Super::BeginPlay();

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

	//Setup movement component stats
	if (MovementComponent) {
		MovementComponent->SpeedLimit = SpeedLimit;
		MovementComponent->PitchRate = PitchRate;
		MovementComponent->YawRate = YawRate;
		MovementComponent->SpeedConstant = SpeedConstant;
		MovementComponent->AccelRate = AccelRate;
	}

	//If this ship was spawned by a spawn point, set its team to the owner's team
	if (Owner) {
		if (AShipSpawnPoint* OSSP = Cast<AShipSpawnPoint>(Owner)) {
			Team = OSSP->GetTeam();
		}
	}

	// Add the Input Mapping Context to the player's input subsystem
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		Team = 0;
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
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Adding weaponsystem to array"));
			WeaponComponents.Add(Comp);
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Added weapon"));
			if (AWeaponSystem* WS = Cast<AWeaponSystem>(Comp->GetChildActor())) {
				WS->team = Team;
				WS->ControlGroup = 0;
			}
		}
	}

	if (AShipPlayerController* SPC = Cast<AShipPlayerController>(Controller)) {
		SPC->AddWeaponWidget();
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
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("child actor not a weapon"));

				}
			}
			else {
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("no wewpaoncomp"));
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
		//Fire action
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShipPawn::Fire);
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
		/*GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Turning"));
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			FColor::Yellow,
			FString::Printf(TEXT("steer values: X=%f, Y=%f"), MoveValue.X, MoveValue.Y)
		);*/
		MovementComponent->SetRotationalInput(FRotator(MoveValue.X, MoveValue.Y, 0));
	}
}

void AShipPawn::Target(const FInputActionValue& Value) {
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Targeting"));
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	// Get screen center
	int32 ViewX, ViewY;
	PC->GetViewportSize(ViewX, ViewY);
	FVector2D ScreenCenter(0.5, 0.5);

	// Find all enemies in world, loop after projecting to screen to find closest enemy to center
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShipPawn::StaticClass(), Enemies);

	float ClosestDist = TNumericLimits<float>::Max();
	AActor* ClosestEnemy = nullptr;

	for (AActor* Enemy : Enemies)
	{
		if (!Enemy || Enemy == this) continue;

		//Find out if the target is on the screen
		FVector2D ScreenLoc;
		bool bOnCamera = PC->ProjectWorldLocationToScreen(Enemy->GetActorLocation(), ScreenLoc);//whether it is on front of camera
		//Normalize the screenloc coordinates to determine if it falls on-screen
		ScreenLoc.X /= ViewX;
		ScreenLoc.Y /= ViewY;
		bool bIsWithinMargin = (ScreenLoc.X >= 0.f && ScreenLoc.X <= 1.0f) && (ScreenLoc.Y >= 0.f && ScreenLoc.Y <= 1.0f);

		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("bOnScreen: %d, ScreenLoc: %f %f, ergo bIsWithinMargin %d"), bOnCamera, ScreenLoc.X, ScreenLoc.Y, bIsWithinMargin));
		if (bOnCamera && bIsWithinMargin)
		{
			float Dist = FVector2D::Distance(ScreenLoc, ScreenCenter);
			//ADDED LOGIC: Set ClosestEnemy only if the current target is not already pointing at this enemy and it is less than halfway between center and edge
			if (Dist < ClosestDist && CurrentTarget != Enemy)
			{
				ClosestDist = Dist;
				ClosestEnemy = Enemy;
			}
		}
	}
	if (ClosestEnemy)
	{
		
		if (CurrentTarget) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Removing old current-target"));
			if (AShipPawn* EnemyShip = Cast<AShipPawn>(CurrentTarget)) {
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Unbinding from old target"));
				EnemyShip->SetIsTargeted(false);
				EnemyShip->OnShipDestroyed.RemoveDynamic(this, &AShipPawn::HandleShipDestroyed);
			}
		}

		CurrentTarget = ClosestEnemy;

		if (AShipPawn* EnemyShip = Cast<AShipPawn>(CurrentTarget)) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Binding the ship destroyed logic"));
			EnemyShip->SetIsTargeted(true);
			EnemyShip->OnShipDestroyed.AddDynamic(this, &AShipPawn::HandleShipDestroyed);
		}

		if (AShipPlayerController* SPC = Cast<AShipPlayerController>(Controller)) {
			SPC->AcquireTargetToHud(ClosestEnemy);
		}
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
			//FString::Printf(TEXT("Target locked: %s"), *ClosestEnemy->GetName()));
	}
	else
	{
		if (CurrentTarget) {
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Removing old current-target"));
			if (AShipPawn* EnemyShip = Cast<AShipPawn>(CurrentTarget)) {
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Unbinding from old target"));
				EnemyShip->SetIsTargeted(false);
				EnemyShip->OnShipDestroyed.RemoveDynamic(this, &AShipPawn::HandleShipDestroyed);
			}
		}
		CurrentTarget = nullptr;
		if (AShipPlayerController* SPC = Cast<AShipPlayerController>(Controller)) {
			SPC->AcquireTargetToHud(CurrentTarget);
		}
		/*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
			FString::Printf(TEXT("No target found")));*/
	}
}

void AShipPawn::Fire() {
	if (AShipPlayerController* ASPC = Cast<AShipPlayerController>(Controller)) {
		if (ASPC->GetPauseStatus()) return;
	}
	if (CurrentTarget) {
		if (AShipPawn* ShipTarget = Cast<AShipPawn>(CurrentTarget)) {
			if (ShipTarget->Team == Team) return;
		}
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Still tracking target"));
		for (UChildActorComponent* WeaponComp : WeaponComponents)
		{
			if (WeaponComp) {
				AActor* Child = WeaponComp->GetChildActor();
				if (AWeaponSystem* Weapon = Cast<AWeaponSystem>(Child))
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Firing!!!!!"));
					Weapon->FireWeapon(CurrentTarget);
				}
				else {
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("child actor not a weapon"));

				}
			}
			else {
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("no wewpaoncomp"));
			}
		}
	}
}

void AShipPawn::UnlockTarget() {
	/*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
		FString::Printf(TEXT("Unlocking target")));*/
	CurrentTarget = nullptr;
}

//Allocate damage hitting ship from some angle
void AShipPawn::AllocateDamage(float FromAngle, int32 DamageAmt) {
	//Possibility for random damage returning 0?

	if (!DamageAmt) return;

	//Determine shield facing based on angle of hit
	int32 ShieldBand = ((int)FromAngle % 360) / 60;
	//Total amount of energy being released as a result of the hit
	int32 ReinTotal = ShieldReinforcements[ShieldBand] >= DamageAmt ? DamageAmt : ShieldReinforcements[ShieldBand];

	//Allocate damage to shield reinforcement
	ShieldReinforcements[ShieldBand] -= DamageAmt;
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Prepping some energy for release!"));
	OnEnergyToBeReleased.Broadcast(ReinTotal);

	if (ReinTotal) {//If there is some amount of damage being taken on the reinforcement
		//Create cooldown timer before the energy is actually freed to be put in the system
		FTimerHandle ThrowAwayHandle;
		//TODO: Add logic to show that this is happening
		TWeakObjectPtr<AShipPawn> WeakThis(this);
		GetWorldTimerManager().SetTimer(ThrowAwayHandle, [WeakThis, ReinTotal, ShieldBand]() {
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Is we releasin the energy?"));
			if (WeakThis.IsValid()) {
				WeakThis->ReleaseEnergy(ReinTotal);
			}
			}, 24.f, false, -1);
	}

	if (ShieldReinforcements[ShieldBand] >= 0) {
		OnShieldStrengthChanged.Broadcast(ShieldBand, ShieldReinforcements[ShieldBand] + ShieldFacingsCurr[ShieldBand]);
		return;
	}
	else {
		DamageAmt = -1 * ShieldReinforcements[ShieldBand];
		ShieldReinforcements[ShieldBand] = 0;
	}



	//Allocate damage to shield itself
	ShieldFacingsCurr[ShieldBand] -= DamageAmt;
	if (ShieldFacingsCurr[ShieldBand] >= 0) {
		OnShieldStrengthChanged.Broadcast(ShieldBand, ShieldFacingsCurr[ShieldBand]);
		return;
	}
	else {
		DamageAmt = -1 * ShieldFacingsCurr[ShieldBand];
		ShieldFacingsCurr[ShieldBand] = 0;
		OnShieldStrengthChanged.Broadcast(ShieldBand, 0);
	}

	//Allocate damage to internals
	//CURRENT HACK: JUST DEAL TO HULL INTEGRITY
	bool HasHitWeapon = false;
	int32 PowerDamage = 0;
	bool HasHitLeft = false;
	bool HasHitRight = false;
	bool HasHitReactor = false;
	bool HasHitCenter = false;
	int32 StartingHull = HullIntegrity;
	for (int i = 0; i < DamageAmt; i++) {
		int32 TargetDamage = FMath::RandRange(0, 11);
		//1/12 chance per hit to damage a weapon
		if (TargetDamage < 1 && !HasHitWeapon) {
			///*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Is we hittin the weapon?")));*/
			//int32 Len = WeaponComponents.Num();
			//int32 IndexesCleared = 0;
			//int32 TargetIndex = FMath::RandRange(0, Len-1);
			////GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Is we hittin the RandRange %d"),TargetIndex));
			//while (!HasHitWeapon) {
			//	if (AWeaponSystem* WS = Cast<AWeaponSystem>(WeaponComponents[TargetIndex]->GetChildActor())) {
			//		if (WS->bIsDamaged) {
			//			TargetIndex = (TargetIndex + 1) % Len;
			//			IndexesCleared++;
			//			if (IndexesCleared > Len) {
			//				HullIntegrity--;
			//				if (!HullIntegrity) break;
			//				HasHitWeapon = true;
			//			}
			//			else {
			//				continue;
			//			}
			//		}
			//		else {
			//			//returns true if energy was freed
			//			if (WS->CauseDamage()) {
			//				//If energy was allocated, release it after 6s
			//				int32 Alloc = WS->AllocatedEnergy;
			//				OnEnergyToBeReleased.Broadcast(Alloc);
			//				FTimerHandle ThrowAwayHandle;
			//				//Release energy from weapon on a timer
			//				GetWorldTimerManager().SetTimer(ThrowAwayHandle, [this, WS, Alloc]() {
			//					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Is we releasin the energy?"));
			//					ReleaseEnergy(WS->FreeEnergy(Alloc));
			//					}, 6.f, false, -1);
			//			}
			//			//otherwise no need to release energy - just broadcast
			//			OnWeaponDamaged.Broadcast(TargetIndex);
			//		}
			//	}
			//}
			//HOLE INTEGRITY TODO: DELETE THIS DUMBASS THING
			HullIntegrity--;
			if (!HullIntegrity) break;
		}
		else if (TargetDamage < 3) {
			if (RightEngCurr) {
				HasHitRight = true;
				RightEngCurr--;
			}
			else if (PowerReactorCurr) {
				HasHitReactor = true;
				PowerReactorCurr--;
			}
			else {
				HullIntegrity--;
				if (!HullIntegrity) break;
				continue;
			}
			PowerDamage++;
		}
		else if (TargetDamage  == 3) {
			if (CenterEngCurr) {
				HasHitCenter = true;
				CenterEngCurr--;
			}
			else if (PowerReactorCurr) {
				HasHitReactor = true;
				PowerReactorCurr--;
			}
			else {
				HullIntegrity--;
				if (!HullIntegrity) break;
				continue;
			}
			PowerDamage++;
		}
		else if (TargetDamage < 6) {
			if (LeftEngCurr) {
				HasHitLeft = true;
				LeftEngCurr--;
			}
			else if (PowerReactorCurr) {
				HasHitReactor = true;
				PowerReactorCurr--;
			}
			else {
				HullIntegrity--;
				if (!HullIntegrity) break;
				continue;
			}
			PowerDamage++;
		}
		else {
			HullIntegrity--;
			if (!HullIntegrity) break;
		}
	}
	
	if (HasHitLeft) {
		OnLeftEngChanged.Broadcast(LeftEngCurr);
	}

	if (HasHitRight) {
		OnRightEngChanged.Broadcast(RightEngCurr);
	}

	if (HasHitCenter) {
		OnCenterEngChanged.Broadcast(CenterEngCurr);
	}
	if (HasHitReactor) {
		OnReactorChanged.Broadcast(PowerReactorCurr);
	}

	//sum total of all damage done to power systems
	if (PowerDamage) {
		OnEnergyToBeRestricted.Broadcast(PowerDamage);
		FTimerHandle ThrowAwayHandle;
		TWeakObjectPtr<AShipPawn> WeakThis(this);
		//Release energy from weapon on a timer
		GetWorldTimerManager().SetTimer(ThrowAwayHandle, [WeakThis, PowerDamage]() {
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Is we releasin the energy?"));
			if (WeakThis.IsValid()) WeakThis->EnergyLoss(PowerDamage);
			}, 18.f, false, -1);
	}

	if (HullIntegrity < StartingHull) {
		if (HullIntegrity <= 0) {
			//If you run out of hull, you are destroyed
			HullIntegrity = 0;
			DestroyShip(0);
		}
		OnHullIntegrityChanged.Broadcast(HullIntegrity);
	}
}

//Power Allocation Functions
void AShipPawn::AllocateReinforceShield(int32 Amt, int32 Index) {
	if (TotalEnergyAvailable >= Amt) {
		//Reduce total available energy by Amt, then increase shield reinforcement in Index by Amt
		TotalEnergyAvailable -= Amt;
		ShieldReinforcements[Index] += Amt;
	}
	else {
		//Otherwise just use all remaining energy on reinforcing the shield
		int32 temp = TotalEnergyAvailable;
		TotalEnergyAvailable = 0;
		ShieldReinforcements[Index] += temp;
	}
	OnAvailableEnergyChanged.Broadcast(TotalEnergyAvailable);
	OnShieldStrengthChanged.Broadcast(Index, ShieldReinforcements[Index] + ShieldFacingsCurr[Index]);
}

void AShipPawn::FreeReinforceShield(int32 Amt, int32 Index) {
	if (ShieldReinforcements[Index] > 0) {
		if (TotalEnergyAvailable + Amt <= TotalEnergyCurr) {
			//Reduce movement energy by Amt, then increase available energy by Amt
			ShieldReinforcements[Index] -= Amt;
			TotalEnergyAvailable += Amt;
		}
		else {
			//Otherwise just remove all energy from movement and push to available
			int32 temp = ShieldReinforcements[Index];
			ShieldReinforcements[Index] = 0;
			TotalEnergyAvailable = TotalEnergyCurr;
		}
	}
	OnAvailableEnergyChanged.Broadcast(TotalEnergyAvailable);
	OnShieldStrengthChanged.Broadcast(Index, ShieldReinforcements[Index] + ShieldFacingsCurr[Index]);
}

//ReleaseEnergy: releases Amt energy and triggers cosmetic events. Used for the case where
//a system is damaged or otherwise disabled and releases its stored energy.
void AShipPawn::ReleaseEnergy(int32 Amt) {
	TotalEnergyAvailable += Amt;
	int32 Temp = TotalEnergyAvailable - TotalEnergyCurr;
	if (Temp > 0) {
		Amt -= Temp;
		TotalEnergyAvailable = TotalEnergyCurr;
	}
	OnEnergyToBeReleased.Broadcast(-1*Amt);//Reduce the to-be-released energy bar on the HUD
	OnAvailableEnergyChanged.Broadcast(TotalEnergyAvailable);//Set the energy bar to the new available total
}

void AShipPawn::EnergyLoss(int32 Amt) {
	TotalEnergyCurr -= Amt;
	TotalEnergyAvailable -= Amt;
	
	if (TotalEnergyAvailable < 0) {
		int32 RemainingDamage = -1 * TotalEnergyAvailable;
		TotalEnergyAvailable = 0;

		//FIRST: Strip power from shield reinforcements
		for (int i = 5; i >= 0; i--) {//Take off from all shields
			if (int32 ReinEnergy = ShieldReinforcements[i]) {
				ReinEnergy -= RemainingDamage;
				if (ReinEnergy < 0) {
					RemainingDamage = -1 * ReinEnergy;
					ReinEnergy = 0;
				}
				ShieldReinforcements[i] = ReinEnergy;
				OnShieldStrengthChanged.Broadcast(i, ShieldFacingsCurr[i] + ReinEnergy);
				if (ReinEnergy) break;
			}
		}

		if (RemainingDamage && MovementEnergy) {
			MovementEnergy -= RemainingDamage;
			if (MovementEnergy < 0) {
				RemainingDamage = -1 * MovementEnergy;
				MovementEnergy = 0;
			}
			if (UShipPawnMovementComponent* MC = Cast<UShipPawnMovementComponent>(MovementComponent)) {
				MC->SetMovementEnergy(MovementEnergy);
				OnMovementEnergyChanged.Broadcast(MovementEnergy);
			}
			if (MovementEnergy) {
				RemainingDamage = 0;
			}
		}

		if (RemainingDamage) {

			//ITERATE OVER ALL WEAPONS TO STRIP ENERGY
			for (int i = 0; i < WeaponComponents.Num(); i++) {
				//
				if (AWeaponSystem* WS = Cast<AWeaponSystem>(WeaponComponents[i]->GetChildActor())) {
					//if the weapon has energy in it, wipe it out
					if (WS->AllocatedEnergy) {
						RemainingDamage -= WS->AllocatedEnergy;
						if (RemainingDamage >= 0) FreeWeapon(i, WS->AllocatedEnergy);
						else {
							FreeWeapon(i, WS->AllocatedEnergy + RemainingDamage);
						}
						if (!RemainingDamage) break;
					}
				}
			}
		}
	}

	if (TotalEnergyCurr <= 0) {//if depowered, kill the ship
		DestroyShip(1);
	}

	//Broadcast new energy
	OnEnergyToBeRestricted.Broadcast(-1 * Amt);
	OnTotalEnergyChanged.Broadcast(TotalEnergyCurr);
	OnAvailableEnergyChanged.Broadcast(TotalEnergyAvailable);
}

//handlers for input
void AShipPawn::HandleArrowAlloc() {
	AllocateMovement(1);
}

void AShipPawn::HandleArrowFree() {
	FreeMovement(1);
}

void AShipPawn::AllocateMovement(int32 Amt) {
	if (TotalEnergyAvailable >= Amt) {
		//Reduce total available energy by Amt, then increase movement energy by Amt
		TotalEnergyAvailable -= Amt;
		MovementEnergy += Amt;
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

void AShipPawn::FreeMovement(int32 Amt) {
	if (MovementEnergy >= Amt) {
		//Reduce movement energy by Amt, then increase available energy by Amt
		TotalEnergyAvailable += Amt;
		MovementEnergy -= Amt;
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

/*
Allocate/free from the weapon at Index Index by amount Amt
*/
int32 AShipPawn::AllocateWeapon(int32 Index, int32 Amt) {
	AActor* TargetWeapon = WeaponComponents[Index]->GetChildActor();
	if (AWeaponSystem* TW = Cast<AWeaponSystem>(TargetWeapon)) {
		if (TotalEnergyAvailable <= Amt) {
			Amt = TotalEnergyAvailable;
			if (!Amt) return 0;
		}
		TotalEnergyAvailable -= Amt;
		int32 AmountAllocated = TW->AllocateEnergy(Amt);
		TotalEnergyAvailable += Amt - AmountAllocated;
		OnAvailableEnergyChanged.Broadcast(TotalEnergyAvailable);
		return AmountAllocated;
	}
	return 0;
}

int32 AShipPawn::FreeWeapon(int32 Index, int32 Amt) {
	AActor* TargetWeapon = WeaponComponents[Index]->GetChildActor();
	if (AWeaponSystem* TW = Cast<AWeaponSystem>(TargetWeapon)) {
		int32 AmountFreed = TW->FreeEnergy(Amt);
		TW->OnEnergyChangedExternal.Broadcast();
		if (TotalEnergyAvailable + AmountFreed > TotalEnergyCurr) {
			AmountFreed = TotalEnergyCurr - TotalEnergyAvailable;
			if (!AmountFreed) return 0;
		}
		TotalEnergyAvailable += AmountFreed;
		OnAvailableEnergyChanged.Broadcast(TotalEnergyAvailable);
		return AmountFreed;
	}
	return 0;
}

void AShipPawn::DestroyShip(int32 CauseOfDeath) {
	//CauseOfDeath: records cause of destruction of the ship
	/*
	0: Hull integrity exhausted (just kill the ship)
	1: Energy exhausted (it drifts?)
	*/
	OnShipDestroyed.Broadcast(CauseOfDeath, this);
	AGameModeBase* CurrentGameMode = GetWorld()->GetAuthGameMode();

	//Branching ifs depending on gamemode
	/*************************************** HALCYON SIMPLE ***************************************/
	if (AHalcyonSimpleGameMode* HSGM = Cast<AHalcyonSimpleGameMode>(CurrentGameMode)) {
		//TODO: Logic for Halcyon Simple
		//Discern player/enemy by controller type
		//POTENTIAL TODO: Add "team" flag; check based on team rather than controller type
		if (AShipPlayerController* SPC = Cast<AShipPlayerController>(Controller)) {
			//Player death event: Cause a loss state in the gamemode (all Halcyon gamemodes should have one)
			HSGM->OnLoss.Broadcast();
		}
		else {
			//Enemy death event
			HSGM->DecrementEnemies();
		}
	}
	else if (AHalcyonMissionGameMode* HMGM = Cast<AHalcyonMissionGameMode>(CurrentGameMode)) {
		if (AShipPlayerController* SPC = Cast<AShipPlayerController>(Controller)) {
			//Player death event: Cause a loss state in the gamemode (all Halcyon gamemodes should have one)
			HMGM->OnMissionLoss.Broadcast();
		}
		else {
			//Enemy death event
			if (Team == 1)
				HMGM->DecrementEnemies();
		}
	}
	//else if else if...
	
	switch (CauseOfDeath) {//Ideally in the end TODO: we add some kind of death animation prior to vaporizing them
	case 0://Currently: just destroy
		this->Destroy();
		return;
	case 1:
		this->Destroy();
		return;
	default:return;
	}
	
}

void AShipPawn::HandleShipDestroyed(int32 CauseOfDeath, AShipPawn* DestroyedShip) {
	if (CurrentTarget) {
		if (AShipPawn* EnemyShip = Cast<AShipPawn>(CurrentTarget)) {
			if (DestroyedShip == CurrentTarget) {
				UnlockTarget();
			}
		}
	}
	
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

int32 AShipPawn::GetShieldFacing(int32 Index) {
	return ShieldFacings[Index];
}

int32 AShipPawn::GetCurrentShieldFacing(int32 Index) {
	return ShieldFacingsCurr[Index];
}

int32 AShipPawn::GetCurrentShieldReinforcement(int32 Index) {
	return ShieldReinforcements[Index];
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

int32 AShipPawn::GetHullIntegrity() {
	return HullIntegrity;
}

int32 AShipPawn::GetMaxHullIntegrity() {
	return MaxHullIntegrity;
}

TArray<UChildActorComponent*> AShipPawn::GetWeaponComponents() {
	return WeaponComponents;
}

void AShipPawn::SetIsTargeted(bool bIsTargeted) {
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,FString::Printf(TEXT("SetIsTargeted called with %d"),bIsTargeted));
	if (HullMesh)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,FString::Printf(TEXT("Setting rendercustomdepth and stencil value")));
		// Enable custom depth rendering
		HullMesh->SetRenderCustomDepth(bIsTargeted);
		// Get all mesh components attached to this actor
		TArray<UMeshComponent*> MeshComponents;
		GetComponents<UMeshComponent>(MeshComponents);

		for (UMeshComponent* MeshComp : MeshComponents)
		{
			if (MeshComp)
			{
				MeshComp->SetRenderCustomDepth(bIsTargeted);
				MeshComp->SetCustomDepthStencilValue(bIsTargeted ? (Team == 1 ? 255 : 250) : 0);
			}
		}
		// Set stencil value (255 for red outline, you can use different values for different colors)
		//HullMesh->SetCustomDepthStencilValue(bIsTargeted ? 255 : 0);
	}
}

float AShipPawn::GetCurrentVelocity(bool bForDisplay) {
	if (UShipPawnMovementComponent* MC = Cast<UShipPawnMovementComponent>(MovementComponent)) {
		float Velocity = MC->GetSpeed();
		return bForDisplay ? roundf(Velocity * 100) / 100.f : Velocity;
	}
	return -1.f;
}

void AShipPawn::BeginDestroy() {
	
	if (ShipMesh && ShipMesh->IsValidLowLevel())
	{
		//Get all children and unweld from the parent mesh to prevent welded component errors
		TArray<USceneComponent*> ChildComponents;
		ShipMesh->GetChildrenComponents(true, ChildComponents);

		for (USceneComponent* Child : ChildComponents)
		{
			if (UPrimitiveComponent* PrimChild = Cast<UPrimitiveComponent>(Child))
			{
				if (PrimChild->IsWelded())
				{
					PrimChild->UnWeldFromParent();
				}
			}
		}
	}

	Super::BeginDestroy();
}
