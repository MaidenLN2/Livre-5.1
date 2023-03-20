// Copyright Epic Games, Inc. All Rights Reserved.

#include "LivreCharacter.h"
#include "LivreProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InterpToMovementComponent.h" //the one for min tick time
#include "CharacterMovementComponentAsync.h" // for current floor
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h" // for update component etc
#include "CoreFwd.h" // for some vectors
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SimModule/SimulationModuleBase.h"
#include "InputMappingContext.h"
#include "Components/TimelineComponent.h"


//////////////////////////////////////////////////////////////////////////
// ALivreCharacter is LUIZZZZZZ

ALivreCharacter::ALivreCharacter()
{
	// Character doesnt have a rifle at start
	//bHasRifle = false;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
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
	
	//tUpdateWallRun = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline UpdaterWallRun"));
	//tUpdateWallRun->SetTimelineLength(5.0f);
	
	// Connecting Collision Detection Functions
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ALivreCharacter::CapsuleTouched);	// might work?
}

void ALivreCharacter::BeginPlay()
{
	// call the base class  
	Super::BeginPlay();
	printf("BeginPlay()");

	//add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Event Graph Functionality:
	EventJumpReset(maxJump);
	printf("jump reset");
	GetCharacterMovement()->SetPlaneConstraintEnabled(false);
	printf("GetCharacterMovement()->SetPlaneConstraintEnabled(false)");


	
	//USkinnedMeshComponent::HideBoneByName(Neck, PBO_None); // might need to be a BP specific function

}

//////////////////////////////////////////////////////////////////////////// Input

 void ALivreCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
 {
 	// set up action bindings
 	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
 	{
 		//Jumping
 		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);	// Original for Storage
 		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ALivreCharacter::CustomJump);
 		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ALivreCharacter::CustomJumpEnded);	// This was just an update to what the function did, it now allows for jumps to reset
 		UE_LOG(LogTemp, Warning, TEXT("jump custom"));

// 		//Moving
 		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &ALivreCharacter::Move);
 		UE_LOG(LogTemp, Warning, TEXT("moving"));

// 		//Looking
 		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Started, this, &ALivreCharacter::Look);
 		UE_LOG(LogTemp, Warning, TEXT("looking"));

		//Sprinting
 		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ALivreCharacter::CustomSprintPressed);
 		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ALivreCharacter::CustomSprintReleased);	// Stops the player from sprinting
 		UE_LOG(LogTemp, Warning, TEXT("sprint custom"));

 		//Sliding
 		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &ALivreCharacter::CustomSlidePressed);
 		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, this, &ALivreCharacter::CustomSlideReleased);	// Stops the player from sliding and resets their values
 		UE_LOG(LogTemp, Warning, TEXT("slide custom"));

 		//Wallrunning
 		EnhancedInputComponent->BindAction(WallrunAction, ETriggerEvent::Started, this, &ALivreCharacter::BeginWallRun);
 		EnhancedInputComponent->BindAction(WallrunAction, ETriggerEvent::Ongoing, this, &ALivreCharacter::UpdateWallRun);
 		EnhancedInputComponent->BindAction(WallrunAction, ETriggerEvent::Completed, this, &ALivreCharacter::CallEndWallRun);
 		// ^Calls EndWallRun early so that the wallrun button has to be held in order for the player to continue wallrunning
 		UE_LOG(LogTemp, Warning, TEXT("wallrun custom"));

 		PlayerInputComponent->BindAxis("Forward", this, &ALivreCharacter::MoveForward);
 		PlayerInputComponent->BindAxis("Right", this, &ALivreCharacter::MoveLateral);
 		PlayerInputComponent->BindAxis("MouseX", this, &ALivreCharacter::LookHorizontal);
 		PlayerInputComponent->BindAxis("MouseY", this, &ALivreCharacter::LookVertical);
 		//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ALivreCharacter::CustomJump);
 		//PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ALivreCharacter::CustomSprintPressed);
 		//PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ALivreCharacter::CustomSprintReleased);
 	}
 }

// Moving/looking functionality 
void ALivreCharacter::MoveForward(float value)
{
	AddMovementInput(RootComponent->GetForwardVector(), FMath::Clamp(value, -1.0f, 1.0f));
}

void ALivreCharacter::MoveLateral(float value)
{
	AddMovementInput(RootComponent->GetRightVector(), FMath::Clamp(value, -1.0f, 1.0f));
}

void ALivreCharacter::LookHorizontal(float value)
{
	AddControllerYawInput(value * sensitivity);
}

void ALivreCharacter::LookVertical(float value)
{
	AddControllerPitchInput(-value * sensitivity);
}

// Custom jump for wall running
void ALivreCharacter::CustomJump()
{
	UE_LOG(LogTemp, Warning, TEXT("Calling CustomJump()"));
	
	if (JumpUsed())
	{
		UE_LOG(LogTemp, Warning, TEXT("Calling Jump()"));
		Jump();

		if (isWallRunning)
		{
			EndWallRun(JumpOff);
		}
	}
	printf("CustomJump()");
}

// What happens after jump ends
void ALivreCharacter::CustomJumpEnded()
{
	if (jumpLeft == 0)
	{
		FTimerHandle ResetJumps;
		GetWorld()->GetTimerManager().SetTimer(ResetJumps, [&]()
		{
			EventJumpReset(maxJump);
		}, 0.25f, false);
	}
	StopJumping();
}
// Sprinting functionality
void ALivreCharacter::CustomSprintPressed()
{
	if (!isSprinting)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartSprint from CustomSprintPressed"));
	}
	isSprinting = true;
	StartSprint();
}

void ALivreCharacter::CustomSprintReleased()
{
	if (isSprinting)
	{
		UE_LOG(LogTemp, Warning, TEXT("StopSprint from CustomSprintReleased"));
	}
	isSprinting = false;
	StopSprint();
}
//Sliding functionality
void ALivreCharacter::CustomSlidePressed()
{
	if (isSprinting && !GetCharacterMovement()->IsFalling())
	{
		GetCapsuleComponent()->SetCapsuleHalfHeight(48.0f);
		// GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	LaunchCharacter(GetActorForwardVector() * 2000.0f, true, false);
		// AddMovementInput(GetActorForwardVector(), 200.0f, false);


		// Sliding Anim Goes Here
		//float Duration = PlayAnimMontage(/*Insert Anim Montage Here*/);
		
		// float Duration = 5.0f;
		//
		// FTimerHandle CSP_TimerHandle;
		// GetWorld()->GetTimerManager().SetTimer(CSP_TimerHandle, [&]()
		// {
		// 	CustomSlidePressed();
		// }, Duration, false);
	}
	UE_LOG(LogTemp, Warning, TEXT("Custom Slide Pressed"));
}

void ALivreCharacter::CustomSlideReleased()
{

	UKismetSystemLibrary::PrintString(this, "Sliding Happened After Animation");
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	FTimerHandle CSP_InternalTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(CSP_InternalTimerHandle, [&]()
	{
		GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	}, 0.5f, false);
	
	UE_LOG(LogTemp, Warning, TEXT("Custom Slide Released"));
}
// Vaulting functionality

// start vaulting
void ALivreCharacter::CustomVaultingPressed()
{
	FVector StartPos = GetActorLocation() - FVector(0.0, 0.0, 44.0);
	FVector EndPos = StartPos + (GetActorForwardVector() * 70.0f); //GetFirstPersonCameraComponent()->GetForwardVector() * 70.0f;
	FHitResult HitTracking;
	// look more in here
	bool FirstLineTraceDidHit = UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		StartPos, //get camera position
		EndPos,
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		true,
		TArray<AActor*>(),
		EDrawDebugTrace::ForDuration,
		HitTracking,
		true
		);

	if (FirstLineTraceDidHit)
	{
		wallLocation = HitTracking.Location;
		wallNormal = HitTracking.Normal;
		
		FVector EndPos2 = (
			wallLocation +
			(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * -10.0f)
			);
		FVector StartPos2 = EndPos2 - FVector(0.0f, 0.0f, 200.0f);
		FHitResult HitTracking2;
		
		bool SecondLineTraceDidHit = UKismetSystemLibrary::LineTraceSingleForObjects(
			this,
			StartPos2,
			EndPos2,
			TArray<TEnumAsByte<EObjectTypeQuery>>(),
			true,
			TArray<AActor*>(),
			EDrawDebugTrace::ForDuration,
			HitTracking2,
			true
			);

		if (SecondLineTraceDidHit)
		{
			wallHeight = HitTracking2.Location;
			// sequence 0
			aboutToClimb = ((wallHeight - wallLocation).Z > 60.0f);

			// sequence 1
			FVector StartPos3 = (
				wallLocation +
				(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * -50.0f) +
				FVector(0.0f, 0.0f, 250.0f)
				);
			FVector EndPos3 = StartPos3 - FVector(0.0f, 0.0f, 300.0f);
			FHitResult HitTracking3;

			bool ThirdLineTraceDidHit = UKismetSystemLibrary::LineTraceSingleForObjects(
				this,
				StartPos3,
				EndPos3,
				TArray<TEnumAsByte<EObjectTypeQuery>>(),
				true,
				TArray<AActor*>(),
				EDrawDebugTrace::ForDuration,
				HitTracking3,
				true
				);

			if (ThirdLineTraceDidHit)
			{
				wallHeight2 = HitTracking3.Location;

				wallIsThicc = (wallHeight - wallHeight2).Z <= 30.0f;	//  negation was removed for simplicity
			}
			else
			{
				wallIsThicc = false;
			}

			if (aboutToClimb)
			{
				// sequence 0
				FVector StartPos4 = GetActorLocation() + FVector(0.0f, 0.0f, 200.0f);
				
				bool FourthLineTraceDidHit = UKismetSystemLibrary::LineTraceSingleForObjects(
					this,
					StartPos4,
					StartPos4 + GetActorForwardVector() * 70.0f,
					TArray<TEnumAsByte<EObjectTypeQuery>>(),
					true,
					TArray<AActor*>(),
					EDrawDebugTrace::ForDuration,
					HitTracking3,
					true
					);

				if (FourthLineTraceDidHit) canClimb = false;
				
				// this last trace may not be necessary, none of the outputs are used.
				bool FifthLineTraceDidHit = UKismetSystemLibrary::LineTraceSingleForObjects(
					this,
					GetActorLocation(),
					GetActorLocation() + FVector(0.0f, 0.0f, 200.0f),
					TArray<TEnumAsByte<EObjectTypeQuery>>(),
					true,
					TArray<AActor*>(),
					EDrawDebugTrace::ForDuration,
					HitTracking3,
					true
					);

				// sequence 1
				if (canClimb)
				{
					GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					GetCharacterMovement()->SetMovementMode(MOVE_Flying);
					SetActorRotation(FRotator(GetActorRotation().Roll, GetActorRotation().Pitch, UKismetMathLibrary::MakeRotFromX(wallNormal).Yaw + 180.0f));
					
					SetActorLocation(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * 50.0f + GetActorLocation());
					SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, wallHeight.Z - 44.0f));
					
					// play anim montage here
					// Duration = PlayAnimMontage();
					float Duration = 4.0f;

					UKismetSystemLibrary::PrintString(this, "landing happened after animation");

					FTimerHandle CVP_ClimbTimerHandle;
					GetWorld()->GetTimerManager().SetTimer(CVP_ClimbTimerHandle, [&]()
					{
						GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
						GetCharacterMovement()->SetMovementMode(MOVE_Walking);
					}, Duration, false);
				}
			}
			else
			{
				GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				GetCharacterMovement()->SetMovementMode(MOVE_Flying);

				FVector NewActorLocation = (UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * 50.0f) + GetActorLocation();
				SetActorLocation(NewActorLocation);

				float Duration;
				if (wallIsThicc)
				{
					// getting Up Animation goes here
					// Duration = PlayAnimMontage();
					Duration = 4.0f;
				}
				else
				{
					SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, wallHeight.Z - 20.0f));
					
					// Duration = PlayAnimMontage();
					Duration = 3.0f;
					UKismetSystemLibrary::PrintString(this, "vaulting happened after animation");
				}

				FTimerHandle CVP_TimerHandle;
				GetWorld()->GetTimerManager().SetTimer(CVP_TimerHandle, [&]()
				{
					GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					GetCharacterMovement()->SetMovementMode(MOVE_Walking);
					SetActorRotation(FRotator(GetActorRotation().Roll, GetActorRotation().Pitch, UKismetMathLibrary::MakeRotFromX(wallNormal).Yaw + 180.0f));
				}, Duration, false);
			}
		}
	}
}
//End vaulting
void ALivreCharacter::CustomVaultingReleased()
{
}
// Custom collision profile
FCollisionQueryParams ALivreCharacter::GetIgnoreCharacterParams()
{
	FCollisionQueryParams parametres;

	TArray<AActor*> characterChildren;
	GetAllChildActors(characterChildren);
	parametres.AddIgnoredActors(characterChildren);
	parametres.AddIgnoredActor(this);

	return parametres;
}
// Setting running speed
void ALivreCharacter::StartSprint(float NewSprintSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewSprintSpeed;
	printf("start sprint");
}
// Changing sprinting speed to walking speed
void ALivreCharacter::StopSprint(float NewWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
	printf("stop sprint");
}

// Setting movement velocity
 void ALivreCharacter::SetHorizontalVelocity(float velocityX, float velocityY)
{
	float z = GetCharacterMovement()->Velocity.Z;
	GetCharacterMovement()->Velocity = FVector(velocityX, velocityY, z);
	printf("set horizontal velocity");
}

// What is happening during actual wall run (math)
void ALivreCharacter::UpdateWallRun()
{
    if (AreKeysRequired())
    {
        FHitResult HitResultCapture;
        FVector TraceEnd = GetActorLocation() + (GetActorRightVector() * (250 * (WallRunSide == Left ? 1 : -1)));
        bool LineTraceDidHit = UKismetSystemLibrary::LineTraceSingle(
            this,
            GetActorLocation(),
            TraceEnd,
            UEngineTypes::ConvertToTraceType(ECC_Visibility),
            false,
            TArray<AActor*>(),
            EDrawDebugTrace::None,
            HitResultCapture,
            true
            );

        if (LineTraceDidHit)
        {
            auto [Direction, Side] = FindRunDirectionAndSide(HitResultCapture.ImpactNormal);

            if (Side == WallRunSide)
            {
                wallRunDirection = Direction;

                FVector SpeedByWallRunDirection = wallRunDirection * GetCharacterMovement()->GetMaxSpeed();
                GetCharacterMovement()->Velocity = FVector(SpeedByWallRunDirection.X, SpeedByWallRunDirection.Y, 0.0f);
            }
            else
            {
            	EndWallRun(FallOff);
            }
        }
        else
        {
        	EndWallRun(FallOff);
        }
    }
	printf("update wall run");
}

void ALivreCharacter::ClampHorizontalVelocity()
{
    if (GetCharacterMovement()->IsFalling())
    {
        float length = GetHorizontalVelocity().Length();
        float lengthBySpeed = length / GetCharacterMovement()->GetMaxSpeed();
        
        if (lengthBySpeed > 1)
        {
            FVector2d clampedHorizontalVelocity = GetHorizontalVelocity() / lengthBySpeed;
            SetHorizontalVelocity(clampedHorizontalVelocity.X, clampedHorizontalVelocity.Y);
        }
    }
	printf("clamp horizontal velocity");
}

std::tuple<FVector, int> ALivreCharacter::FindRunDirectionAndSide(FVector InputWallNormal)
{
	enum WallRunSide SideLocal;
    FVector2d test1(InputWallNormal);
    FVector2d test2(GetActorRightVector());

    float DotProd = FVector2d::DotProduct(test1, test2);

    float zFlip = 1;
    if  (DotProd > 0)
    {
        SideLocal = Right;
    }
    else
    {
        SideLocal = Left;
        zFlip *= -1;
    }

    FVector CrossProduct = FVector::CrossProduct(InputWallNormal, FVector(0, 0, zFlip));
	printf("find run direction and side");
    return std::tuple(CrossProduct, SideLocal);
}

bool ALivreCharacter::IsSurfaceWallRan(FVector surfaceVector)
{
    if (surfaceVector.Z < -0.05f)
    {
        return false;
    }
    
    FVector DotComp = FVector(surfaceVector.X, surfaceVector.Y, 0.0f);
    DotComp.Normalize();

    float DotResult = FVector::DotProduct(DotComp, surfaceVector);
    float ArcCosDotResult = UKismetMathLibrary::DegAcos(DotResult);    // KismetMathLibrary broooo
    
    float WalkableFloorAngle = GetCharacterMovement()->GetWalkableFloorAngle();
	printf("is surface wall run");
    return (ArcCosDotResult < WalkableFloorAngle);
}

FVector ALivreCharacter::LaunchVelocity()
{
    FVector LaunchDirection;
    // sequence 0
    if (isWallRunning)
    {
        FVector CrossComp = FVector(0, 0, WallRunSide == Left ? 1 : -1 );

        LaunchDirection = FVector::CrossProduct(wallRunDirection, CrossComp);
    }
    else
    {
        if (GetCharacterMovement()->IsFalling())
        {
            LaunchDirection = (GetActorRightVector() * axisRight) + (GetActorForwardVector() * axisForward);
        }
    }
	printf("launch velocity");
    // sequence 1
    return (LaunchDirection + FVector(0, 0, 1)) * GetCharacterMovement()->JumpZVelocity;
}

bool ALivreCharacter::AreKeysRequired()
{
    if (axisForward > 0.1f)
    {
        switch (WallRunSide)
        {
        case Left:
            return axisLeft > 0.1f;
        case Right:
            return axisRight < 0.1f;
        default: ;
        }
    }
	printf("are keys required");
    return false;
}

FVector2d ALivreCharacter::GetHorizontalVelocity()
{
	printf("get horizontal velocity");
    return FVector2d(GetCharacterMovement()->GetLastUpdateVelocity());
}

bool ALivreCharacter::JumpUsed()
{
	if (isWallRunning)	return true;

	if (jumpLeft > 0)
	{
		jumpLeft--;
		return true;
	}
	printf("jump used");
	return false;
}

void ALivreCharacter::EventJumpReset(int Jumps)
{
	//UE_LOG(LogTemp, Warning, TEXT("CALLING EVENTJUMPRESET(). New Jump Value = %i"), Jumps);

	jumpLeft = UKismetMathLibrary::Clamp(Jumps, 0, maxJump);
}

void ALivreCharacter::EventAnyDamage(float Damage)
{
	health -= Damage;

	if (health <= 0.0f)
	{
		isDead = true;
		UGameplayStatics::OpenLevel(this, FName("Main_Menu"));
	}
}

void ALivreCharacter::EventOnLanded()
{
	EventJumpReset(maxJump);

	GetCharacterMovement()->GravityScale = initialGravity;

	UGameplayStatics::PlayWorldCameraShake(this, TSubclassOf<UCameraShakeBase>(), GetActorLocation(), 0.0f, 100.0f, 1.0f);
}

void ALivreCharacter::CapsuleTouched(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (isWallRunning) return;

	if (IsSurfaceWallRan(SweepResult.ImpactNormal) && GetCharacterMovement()->IsFalling())
	{
		auto [OutDir, OutSide] = FindRunDirectionAndSide(SweepResult.ImpactNormal);
		wallRunDirection = OutDir;

		switch (OutSide)
		{
		case Left:
			WallRunSide = Left;
			break;
		case Right:
			WallRunSide = Right;
			break;
		default: break;
		}

		if (AreKeysRequired())
		{
			BeginWallRun();
		}
		else
		{
			EndWallRun(FallOff);
		}
	}
}

void ALivreCharacter::BeginWallRun()
{
	
	// FTimerHandle TestHandle;        // Can be declared inside the function and not worried about again
	//
	// GetWorld()->GetTimerManager().SetTimer(TestHandle, [&]()
	// {
	// 	if (!isUpdatingWallRun)
	// 	{
	// 		isUpdatingWallRun = true;
	// 		GetWorld()->GetTimerManager().SetTimer(TestHandle, [&]()
	// 		{
	// 			EndWallRun(FallOff);
	// 		}, -1.0f, false, timeDelay);
	// 	}
	// 	else
	// 	{
	// 		UpdateWallRun();
	// 	}
	// }, 0.01f, true);
	
	// sequence 0
	if (!isWallRunning)	// This check makes sure the function isn't called multiple times on activation. Once it activates once, it prevents it from starting again.
	{
		// Added check in here to see if the player is next to a wall before deciding to activate the wallrun.
		// Doesn't check for sides atm.
		FHitResult HitResultCapture;
		
		FVector rightTraceEnd = GetActorLocation() + (GetActorRightVector() * 250);
		bool rightLineTraceDidHit = UKismetSystemLibrary::LineTraceSingle(
			this,
			GetActorLocation(),
			rightTraceEnd,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			TArray<AActor*>(),
			EDrawDebugTrace::Persistent,
			HitResultCapture,
			true
			);

		FVector leftTraceEnd = GetActorLocation() + (GetActorRightVector() * -250);
		bool leftLineTraceDidHit = UKismetSystemLibrary::LineTraceSingle(
			this,
			GetActorLocation(),
			leftTraceEnd,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			TArray<AActor*>(),
			EDrawDebugTrace::Persistent,
			HitResultCapture,
			true
			);

		// If we did detect a line trace, perform the function and let's wallrun!
		if (rightLineTraceDidHit || leftLineTraceDidHit)
		{
			FTimerHandle BWR_Delayhandle;
			GetWorld()->GetTimerManager().SetTimer(BWR_Delayhandle, [&]()
			{
				// call End Wall Run
				EndWallRun(FallOff);
			}, 2, false);
	
			// sequence 1 where player runs on walls
			GetCharacterMovement()->AirControl = 1.0f;
			GetCharacterMovement()->GravityScale = 0.0f;

			isWallRunning = true;
			//  place for camera tilt begin

			// Timeline the UpdateWallRun function for 5 seconds
		}
	}
}

/**
 * @brief Function explicitly to work within the EnhancedInputComponent as binding functions to actions requires they do not have variables to enter, for now.
 */
void ALivreCharacter::CallEndWallRun()
{
	if (isWallRunning)
	{
		EndWallRun(FallOff);
	}
}

void ALivreCharacter::EndWallRun(WallRunEnd Why)
{
	if (isWallRunning) // we only want to stop the wallrunning if it's already started
	{
		switch (Why)
		{
		case FallOff:
			EventJumpReset(1);
			break;
		case JumpOff:
			EventJumpReset(maxJump);
			break;
		default: ;
		}

		GetCharacterMovement()->AirControl = 0.05f;
		GetCharacterMovement()->GravityScale = 1.0f; // why not working?
		isWallRunning = false;
		isUpdatingWallRun = false;
	}

	//  place for camera tilt end

	
}

void ALivreCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ALivreCharacter::Look(const FInputActionValue& Value)
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
