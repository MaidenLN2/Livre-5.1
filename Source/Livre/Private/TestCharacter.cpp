// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


//////////////////////////////////////////////////////////////////////////
// ADefaultTestCharacter

ADefaultTestCharacter::ADefaultTestCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

void ADefaultTestCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Custom Expansion
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ADefaultTestCharacter::CapsuleComponentHit);
	JumpReset(MaxJump);
	
	if (UpdateWallRunCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, TEXT("ProcessTimeline"));
		UpdateWallRunTimeline.AddInterpFloat(UpdateWallRunCurve, ProgressFunction);

		UpdateWallRunTimeline.SetTimelineLengthMode(TL_LastKeyFrame);
	}
}

void ADefaultTestCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ClampHorizontalVelocity();

	GetCharacterMovement()->GravityScale = (GetCharacterMovement()->Velocity.Z <= 0.0f) ? FallingGravity : InitialGravity;
	
	if (UpdateWallRunTimeline.IsPlaying())
	{
		UpdateWallRunTimeline.TickTimeline(DeltaSeconds);
	}
}

void ADefaultTestCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearTimer(WallRunDurationTimerHandle);
	GetWorldTimerManager().ClearTimer(EndSlideTimerHandle);
}

//////////////////////////////////////////////////////////////////////////// Input

void ADefaultTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ADefaultTestCharacter::CustomJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADefaultTestCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADefaultTestCharacter::Look);

		//Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ADefaultTestCharacter::CustomSprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ADefaultTestCharacter::CustomSprintEnd);

		//Sliding
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &ADefaultTestCharacter::CustomSlideStart);
	}
}

bool ADefaultTestCharacter::LineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit) const
{
	return UKismetSystemLibrary::LineTraceSingle(
		this,
		Start,
		End,
		static_cast<ETraceTypeQuery>(ECC_Visibility),
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::Persistent,
		OutHit,
		true
		);
}

bool ADefaultTestCharacter::JumpUsed()
{
	if (IsWallRunning) return true;
	if (JumpLeft <= 0) return false;

	JumpLeft--;
	return true;
}

void ADefaultTestCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ADefaultTestCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

std::tuple<FVector, EWallRunSide> ADefaultTestCharacter::FindRunDirectionAndSide(FVector WallNormal)
{
	EWallRunSide SideLocal = Left;
	
	if (UKismetMathLibrary::DotProduct2D(FVector2D(WallNormal), FVector2D(GetActorRightVector())) > 0.0f)
	{
		SideLocal = Right;
	}

	FVector OutDir = UKismetMathLibrary::Cross_VectorVector(WallNormal, FVector(0.0f, 0.0f, (SideLocal == Left ? -1 : 1)));
	
	return std::tuple(OutDir, SideLocal);
}

bool ADefaultTestCharacter::IsSurfaceWallRan(FVector SurfaceNormal)
{
	if (SurfaceNormal.Z < -0.05f) return false;

	FVector SurfaceNormalNoZ = FVector(SurfaceNormal.X, SurfaceNormal.Y, 0.0f);
	UKismetMathLibrary::Vector_Normalize(SurfaceNormalNoZ);

	const float ACosDotResult = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(SurfaceNormalNoZ, SurfaceNormal));

	return (ACosDotResult < GetCharacterMovement()->GetWalkableFloorAngle());
}

FVector ADefaultTestCharacter::LaunchVelocity()
{
	FVector LaunchDirection;
	
	// SEQUENCE NOTIFIER
	// 0
	if (IsWallRunning)
	{
		LaunchDirection = UKismetMathLibrary::Cross_VectorVector(WallRunDirection, FVector(0.0f, 0.0f, (WallRunSide_Global == Left ? 1 : -1)));
	}
	else if (GetCharacterMovement()->IsFalling())
	{
		LaunchDirection = (GetActorRightVector() * AxisRight) + (GetActorForwardVector() * AxisForward);
	}

	// 1
	return ((LaunchDirection + FVector(0.0f, 0.0f, 1.0f)) * GetCharacterMovement()->JumpZVelocity);
}

bool ADefaultTestCharacter::AreKeysRequired()
{
	if (AxisForward <= 0.1f) return false;

	return (WallRunSide_Global == Left ? (AxisRight > 0.1f) : (AxisRight < 0.1f));
}

FVector2D ADefaultTestCharacter::GetHorizontalVelocity()
{
	 return FVector2D(GetCharacterMovement()->Velocity);
}

void ADefaultTestCharacter::SetHorizontalVelocity(FVector2D HorizontalVelocity)
{
	GetCharacterMovement()->Velocity = FVector(HorizontalVelocity, GetCharacterMovement()->Velocity.Z);
}

void ADefaultTestCharacter::UpdateWallRun()
{
	if (!AreKeysRequired()) return;

	const FVector TraceStart = GetActorLocation();
	const FVector TraceEnd = GetActorLocation() + (GetActorRightVector() * 250 * (WallRunSide_Global == Left ? 1.0f : -1.0f));
	FHitResult Storage;

	bool DidHit = LineTrace(TraceStart, TraceEnd, Storage);

	if (!DidHit)
	{
		EndWallRun(FallOff);
		return;
	}

	auto [Direction, SideLocal] = FindRunDirectionAndSide(Storage.ImpactNormal);

	if (!WallRunSide_Global == SideLocal)
	{
		EndWallRun(FallOff);
		return;
	}

	WallRunDirection = Direction;
	GetCharacterMovement()->Velocity = FVector(FVector2D(WallRunDirection * GetCharacterMovement()->GetMaxSpeed()), 0.0f);
}

void ADefaultTestCharacter::ClampHorizontalVelocity()
{
	if (!GetCharacterMovement()->IsFalling()) return;

	const float HVelocityByMaxSpeed = (GetHorizontalVelocity().Length() / GetCharacterMovement()->GetMaxSpeed());

	if (HVelocityByMaxSpeed <= 1.0f) return;

	SetHorizontalVelocity(GetHorizontalVelocity() / HVelocityByMaxSpeed);
}

void ADefaultTestCharacter::JumpReset(int NewJumpsValue)
{
	JumpLeft = UKismetMathLibrary::Clamp(NewJumpsValue, 0, MaxJump);
}

void ADefaultTestCharacter::BeginWallRun()
{
	// SEQUENCE NOTIFIER
	// 0 - Start Timer
	GetWorldTimerManager().SetTimer(WallRunDurationTimerHandle, [&]()
	{
		EndWallRun(FallOff);
	}, 2.0f, false);
	
	// 1
	GetCharacterMovement()->AirControl = 1.0f;
	GetCharacterMovement()->GravityScale = 0.0f;

	IsWallRunning = true;
	
	// Begin Camera Tilt - Once implemented, it goes here

	UpdateWallRunTimeline.PlayFromStart();
}

void ADefaultTestCharacter::EndWallRun(EWallRunEnd Why)
{
	JumpReset((Why == FallOff) ? 1 : MaxJump);

	GetCharacterMovement()->AirControl = 0.05f;
	GetCharacterMovement()->GravityScale = 1.0f;

	IsWallRunning = false;

	// End Camera Tilt - Once implemented, it goes here

	UpdateWallRunTimeline.Stop();
}

void ADefaultTestCharacter::CustomJumpStart()
{
	if (!JumpUsed()) return;

	LaunchCharacter(LaunchVelocity(), false, true);

	if (IsWallRunning)
	{
		EndWallRun(JumpOff);
	}
}

void ADefaultTestCharacter::CustomSlideStart()
{
	if (!IsSprinting) return;
	if (GetCharacterMovement()->IsFalling()) return;

	GetCapsuleComponent()->SetCapsuleHalfHeight(48.0f);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);

	AddMovementInput(FVector(), 200.0f);

	// Play Anim Montage here, record float output for Timer below

	GetWorldTimerManager().SetTimer(EndSlideTimerHandle, [&]()
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		// Brief delay of 0.5f seconds not included here, suspect it will be fine but noting for
		// the sake of accuracy
		GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	}, /* Duration of Anim Montage */ 2.0f, false);
}

void ADefaultTestCharacter::CustomSprintStart()
{
	IsSprinting = true;
	StartSprint();
}

void ADefaultTestCharacter::CustomSprintEnd()
{
	IsSprinting = false;
	StopSprint();
}

void ADefaultTestCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	JumpReset(MaxJump);

	GetCharacterMovement()->GravityScale = InitialGravity;
	
	UGameplayStatics::PlayWorldCameraShake(
		this,
		TSubclassOf<UCameraShakeBase>(),
		GetActorLocation(),
		0.0f,
		100.0f
		);
}

void ADefaultTestCharacter::CapsuleComponentHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (IsWallRunning) return;

	if (IsSurfaceWallRan(Hit.ImpactNormal))
	{
		if (GetCharacterMovement()->IsFalling())
		{
			auto [Direction, SideLocal] = FindRunDirectionAndSide(Hit.ImpactNormal);

			WallRunDirection = Direction;
			WallRunSide_Global = SideLocal;

   			AreKeysRequired() ? BeginWallRun() : EndWallRun(FallOff);
		}
	}
}

/**
 * @brief 
 * @param TimelineProgress 
 */
void ADefaultTestCharacter::ProcessTimeline(float TimelineProgress)
{
	UpdateWallRun();
}

// CUSTOM FUNCTIONALITY ABOVE

/**
 * @brief Default standard move implementation for UE 5.1
 * @param Value 
 */
void ADefaultTestCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	AxisForward = MovementVector.Y;
	AxisRight = MovementVector.X;

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ADefaultTestCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ADefaultTestCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool ADefaultTestCharacter::GetHasRifle()
{
	return bHasRifle;
}
