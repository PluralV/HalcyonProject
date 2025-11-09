// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipPawn.h"
#include "ShipPawnMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ShipPlayerController.h"

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

	
}



// Called every frame
void AShipPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Inputs bound"));
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
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Throttling"));
		MovementComponent->SetThrustInput(AppliedThrottle);
	}
}

void AShipPawn::Decelerate(const FInputActionValue& Value) {
	const FVector2D MoveValue = Value.Get<FVector2D>();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Braking"));
	bool bPressed = Value.Get<bool>();
	float AppliedThrottle = bPressed ? -1.f : 0.f;
	if (AShipPlayerController* PC = Cast<AShipPlayerController>(GetController()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Throttling"));
		MovementComponent->SetThrustInput(AppliedThrottle);
	}
}

void AShipPawn::Steer(const FInputActionValue& Value) {
	const FVector2D MoveValue = Value.Get<FVector2D>();
	
	if (AShipPlayerController* PC = Cast<AShipPlayerController>(GetController()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Turning"));
		MovementComponent->SetRotationalInput(FRotator(MoveValue.X, MoveValue.Y, 0));
	}
}

//Power Allocation Functions
void AShipPawn::AllocateReinforceShield(int32 amt, int32 index) {

}

void AShipPawn::AllocateMovement(int32 amt) {

}

void AShipPawn::AllocateModularSystem(AModularSystem* TargetSystem, int32 amt) {

}

void AShipPawn::FreeReinforceShield(int32 amt, int32 index) {

}

void AShipPawn::FreeMovement(int32 amt) {

}

void AShipPawn::FreeModularSystem(AModularSystem* TargetSystem, int32 amt) {

}
